/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/oslfile2streamwrap.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <sfx2/filedlghelper.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfiltertestdialog.hrc"


using namespace utl;
using namespace osl;
using namespace comphelper;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::frame;
using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::system;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;

using ::rtl::OUString;

class GlobalEventListenerImpl : public ::cppu::WeakImplHelper1< com::sun::star::document::XEventListener >
{
public:
    GlobalEventListenerImpl( XMLFilterTestDialog* pDialog );

    // XEventListener
    virtual void SAL_CALL notifyEvent( const com::sun::star::document::EventObject& Event ) throw (RuntimeException);

    // lang::XEventListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw (RuntimeException);
private:
    XMLFilterTestDialog* mpDialog;
};

GlobalEventListenerImpl::GlobalEventListenerImpl( XMLFilterTestDialog* pDialog )
: mpDialog( pDialog )
{
}

void SAL_CALL GlobalEventListenerImpl::notifyEvent( const com::sun::star::document::EventObject& Event ) throw (RuntimeException)
{
    ::SolarMutexGuard aGuard;
    if( (Event.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnFocus") ) == 0) ||
        (Event.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnUnload") ) == 0) )
    {
        Reference< XComponent > xComp( Event.Source, UNO_QUERY );
        mpDialog->updateCurrentDocumentButtonState( &xComp );
    }
}

void SAL_CALL GlobalEventListenerImpl::disposing( const com::sun::star::lang::EventObject& /* Source */ ) throw (RuntimeException)
{
}

/** returns true if the given component supports the given service */
static bool checkComponent( Reference< XComponent >& rxComponent, const OUString& rServiceName )
{
    try
    {
        Reference< XServiceInfo > xInfo( rxComponent, UNO_QUERY );
        if( xInfo.is() )
        {
            if( xInfo->supportsService( rServiceName ) )
            {
                // special case for impress documents which supports same service as draw documents
                if ( rServiceName == "com.sun.star.drawing.DrawingDocument" )
                {
                    // so if we want a draw we need to check if its not an impress
                    if( !xInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument") ) ) )
                        return true;
                }
                else
                {
                    return true;
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "checkComponent exception catched!" );
    }

    return false;
}

XMLFilterTestDialog::XMLFilterTestDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF ) :
    ModalDialog( pParent, ResId( DLG_XML_FILTER_TEST_DIALOG, rResMgr ) ),
    mxMSF( rxMSF ),

    maFLExport( this, ResId( FL_EXPORT, rResMgr ) ),
    maFTExportXSLT( this, ResId( FT_EXPORT_XSLT, rResMgr ) ),
    maFTExportXSLTFile( this, ResId( FT_EXPORT_XSLT_FILE, rResMgr ) ),
    maFTTransformDocument( this, ResId( FT_TRANSFORM_DOCUMENT, rResMgr ) ),
    maPBExportBrowse( this, ResId( PB_EXPORT_BROWSE, rResMgr ) ),
    maPBCurrentDocument( this, ResId( PB_CURRENT_DOCUMENT, rResMgr ) ),
    maFTNameOfCurentFile( this, ResId( FT_NAME_OF_CURRENT_FILE, rResMgr ) ),
    maFLImport( this, ResId( FL_IMPORT, rResMgr ) ),
    maFTImportXSLT( this, ResId( FT_IMPORT_XSLT, rResMgr ) ),
    maFTImportXSLTFile( this, ResId( FT_IMPORT_XSLT_FILE, rResMgr ) ),
    maFTImportTemplate( this, ResId( FT_IMPORT_TEMPLATE, rResMgr ) ),
    maFTImportTemplateFile( this, ResId( FT_IMPORT_TEMPLATE_FILE, rResMgr ) ),
    maFTTransformFile( this, ResId( FT_TRANSFORM_FILE, rResMgr ) ),
    maCBXDisplaySource( this, ResId( CBX_DISPLAY_SOURCE, rResMgr ) ),
    maPBImportBrowse( this, ResId( PB_IMPORT_BROWSE, rResMgr ) ),
    maPBRecentDocument( this, ResId( PB_RECENT_DOCUMENT, rResMgr ) ),
    maFTNameOfRecentFile( this, ResId( FT_NAME_OF_RECENT_FILE, rResMgr ) ),
    maPBClose( this, ResId( PB_CLOSE, rResMgr ) ),
    maPBHelp( this, ResId( PB_HELP, rResMgr ) ),
    mpFilterInfo( NULL ),
    sDTDPath( RTL_CONSTASCII_USTRINGPARAM( "$(inst)/share/dtd/officedocument/1_0/office.dtd" ) )
{
    FreeResource();

    maPBExportBrowse.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBCurrentDocument.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBImportBrowse.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBRecentDocument.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBClose.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );

    maDialogTitle = GetText();

    try
    {
        Reference< XConfigManager > xCfgMgr( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" ))), UNO_QUERY );
        if( xCfgMgr.is() )
            sDTDPath = xCfgMgr->substituteVariables( sDTDPath );

        mxGlobalBroadcaster = Reference < XEventBroadcaster >( GlobalEventBroadcaster::create(comphelper::getComponentContext(mxMSF)), UNO_QUERY_THROW );
        mxGlobalEventListener = new GlobalEventListenerImpl( this );
        mxGlobalBroadcaster->addEventListener( mxGlobalEventListener );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::XMLFilterTestDialog exception catched!" );
    }
}

XMLFilterTestDialog::~XMLFilterTestDialog()
{
    try
    {
        if( mxGlobalBroadcaster.is() )
            mxGlobalBroadcaster->removeEventListener( mxGlobalEventListener );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::~XMLFilterTestDialog exception catched!" );
    }

    delete mpFilterInfo;
}

void XMLFilterTestDialog::test( const filter_info_impl& rFilterInfo )
{
    delete mpFilterInfo;
    mpFilterInfo = new filter_info_impl( rFilterInfo );

    maImportRecentFile = OUString();

    initDialog();

    Execute();
}

static OUString getFileNameFromURL( OUString& rURL )
{
    INetURLObject aURL( rURL );
    OUString aName( aURL.getName(INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::DECODE_WITH_CHARSET) );
    return aName;
}

void XMLFilterTestDialog::updateCurrentDocumentButtonState( Reference< XComponent > * pRef /* = NULL */ )
{
    if( pRef && pRef->is() )
    {
        if( checkComponent( *pRef, mpFilterInfo->maDocumentService ) )
            mxLastFocusModel = *pRef;
    }

    bool bExport = (mpFilterInfo->maFlags & 2) == 2;
    Reference< XComponent > xCurrentDocument;
    if( bExport )
        xCurrentDocument = getFrontMostDocument( mpFilterInfo->maDocumentService );
    maPBCurrentDocument.Enable( bExport && xCurrentDocument.is() );
    maFTNameOfCurentFile.Enable( bExport && xCurrentDocument.is() );

    if( xCurrentDocument.is() )
    {
        OUString aTitle;
        Reference< XDocumentPropertiesSupplier > xDPS( xCurrentDocument, UNO_QUERY );
        if( xDPS.is() )
        {
            Reference< XDocumentProperties > xProps( xDPS->getDocumentProperties() );
            if( xProps.is() )
            {
                aTitle = xProps->getTitle();
            }
        }

        if( aTitle.isEmpty() )
        {
            Reference< XStorable > xStorable( xCurrentDocument, UNO_QUERY );
            if( xStorable.is() )
            {
                if( xStorable->hasLocation() )
                {
                    OUString aURL( xStorable->getLocation() );
                    aTitle = getFileNameFromURL( aURL );
                }
            }
        }

        maFTNameOfCurentFile.SetText( aTitle );
    }
}

void XMLFilterTestDialog::initDialog()
{
    DBG_ASSERT( mpFilterInfo, "i need a filter I can test!" );
    if( NULL == mpFilterInfo )
        return;

    String aTitle( maDialogTitle );
    aTitle.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("%s") ), mpFilterInfo->maFilterName );
    SetText( aTitle );

    String aEmpty;
    bool bImport = (mpFilterInfo->maFlags & 1) == 1;
    bool bExport = (mpFilterInfo->maFlags & 2) == 2;

    updateCurrentDocumentButtonState();

    maFLExport.Enable( bExport );
    maFTExportXSLT.Enable( bExport );
    maFTExportXSLTFile.Enable( bExport );
    maFTTransformDocument.Enable( bExport );
    maPBExportBrowse.Enable( bExport );

    maFTExportXSLTFile.SetText( getFileNameFromURL( mpFilterInfo->maExportXSLT ) );

    // ---

    maFLImport.Enable( bImport );
    maFTImportXSLT.Enable( bImport );
    maFTImportXSLTFile.Enable( bImport );
    maFTImportTemplate.Enable( bImport && !mpFilterInfo->maImportTemplate.isEmpty() );
    maFTImportTemplateFile.Enable( bImport && !mpFilterInfo->maImportTemplate.isEmpty() );
    maFTTransformFile.Enable( bImport );
    maCBXDisplaySource.Enable( bImport );
    maPBImportBrowse.Enable( bImport );
    maPBRecentDocument.Enable( bImport && !maImportRecentFile.isEmpty() );
    maFTNameOfRecentFile.Enable( bImport && !maImportRecentFile.isEmpty() );

    maFTImportXSLTFile.SetText( getFileNameFromURL( mpFilterInfo->maImportXSLT ) );
    maFTImportTemplateFile.SetText( getFileNameFromURL( mpFilterInfo->maImportTemplate ) );
    maFTNameOfRecentFile.SetText( getFileNameFromURL( maImportRecentFile ) );
}

void XMLFilterTestDialog::onExportBrowse()
{
    try
    {
        // Open Fileopen-Dialog
           ::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );

        Reference< XNameAccess > xFilterContainer( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" )) ), UNO_QUERY );
        Reference< XNameAccess > xTypeDetection( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" )) ), UNO_QUERY );
        if( xFilterContainer.is() && xTypeDetection.is() )
        {
            Sequence< OUString > aFilterNames( xFilterContainer->getElementNames() );
            OUString* pFilterName = aFilterNames.getArray();

            for( sal_Int32 nFilter = 0; nFilter < aFilterNames.getLength(); nFilter++, pFilterName++ )
            {
                Sequence< PropertyValue > aValues;

                Any aAny( xFilterContainer->getByName( *pFilterName ) );
                if( !(aAny >>= aValues) )
                    continue;

                OUString aInterfaceName;
                PropertyValue* pValues = aValues.getArray();
                OUString aType, aService;
                sal_Int32 nFlags( 0 );

                int nFound = 0;

                for( sal_Int32  nValue = 0; (nValue < aValues.getLength()) && (nFound != 15); nValue++, pValues++ )
                {
                    if ( pValues->Name == "Type" )
                    {
                        pValues->Value >>= aType;
                        nFound |= 1;
                    }
                    else if ( pValues->Name == "DocumentService" )
                    {
                        pValues->Value >>= aService;
                        nFound |= 2;
                    }
                    else if ( pValues->Name == "Flags" )
                    {
                        pValues->Value >>= nFlags;
                        nFound |= 4;
                    }
                    if ( pValues->Name == "UIName" )
                    {
                        pValues->Value >>= aInterfaceName;
                        nFound |= 8;
                    }

                }

                if( (nFound == 15) && (!aType.isEmpty() && aService == mpFilterInfo->maDocumentService) )
                {
                    // see if this filter is not supressed in dialog
                    if( (nFlags & 0x1000) == 0 )
                    {
                        aAny = xTypeDetection->getByName( aType );
                        Sequence< PropertyValue > aValues2;

                        if( aAny >>= aValues2 )
                        {
                            PropertyValue* pValues2 = aValues2.getArray();
                            sal_Int32 nValue;

                            OUString aExtension;
                            for( nValue = 0; nValue < aValues2.getLength(); nValue++, pValues2++ )
                            {
                                if ( pValues2->Name == "Extensions" )
                                {
                                    Sequence< OUString > aExtensions;
                                    if( pValues2->Value >>= aExtensions )
                                    {
                                        const sal_Int32 nCount( aExtensions.getLength() );
                                        OUString* pExtensions = aExtensions.getArray();
                                        sal_Int32 n;
                                        for( n = 0; n < nCount; n++ )
                                        {
                                            if( n > 0 )
                                                aExtension += OUString( sal_Unicode(';') );
                                            aExtension += OUString( RTL_CONSTASCII_USTRINGPARAM( "*." ));
                                            aExtension += (*pExtensions++);
                                        }
                                    }
                                }
                            }

                            String aExtensions( aExtension );
                            String aFilterName( aInterfaceName );
                            aFilterName += String( RTL_CONSTASCII_USTRINGPARAM(" (") );
                            aFilterName += aExtensions;
                            aFilterName += sal_Unicode(')');

                            aDlg.AddFilter( aFilterName, aExtensions );

                            if( (nFlags & 0x100) == 0x100 )
                                aDlg.SetCurrentFilter( aFilterName );
                        }
                    }
                }

            }
        }

        aDlg.SetDisplayDirectory( maExportRecentFile );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            maExportRecentFile = aDlg.GetPath();

            Reference< XComponentLoader > xLoader( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ), UNO_QUERY );
            Reference< XInteractionHandler > xInter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.InteractionHandler" )) ), UNO_QUERY );
            if( xLoader.is() && xInter.is() )
            {
                OUString aFrame( RTL_CONSTASCII_USTRINGPARAM( "_default" ) );
                Sequence< PropertyValue > aArguments(1);
                aArguments[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
                aArguments[0].Value <<= xInter;
                Reference< XComponent > xComp( xLoader->loadComponentFromURL( maExportRecentFile, aFrame, 0, aArguments ) );
                if( xComp.is() )
                {
                    doExport( xComp );
                }
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("XMLFilterTestDialog::onExportBrowse exception caught!");
    }

    initDialog();
}

void XMLFilterTestDialog::onExportCurrentDocument()
{
    doExport( getFrontMostDocument( mpFilterInfo->maDocumentService ) );
}

void XMLFilterTestDialog::doExport( Reference< XComponent > xComp )
{
    try
    {
        Reference< XStorable > xStorable( xComp, UNO_QUERY );
        if( xStorable.is() )
        {
            String leadingChars;
            String ext(RTL_CONSTASCII_USTRINGPARAM(".xml"));
            utl::TempFile aTempFile(leadingChars, &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            const application_info_impl* pAppInfo = getApplicationInfo( mpFilterInfo->maExportService );
            if( pAppInfo )
            {
                File aOutputFile( aTempFileURL );
                /* File::RC rc = */ aOutputFile.open( osl_File_OpenFlag_Write );

                // create xslt exporter
                Reference< XOutputStream > xIS( new comphelper::OSLOutputStreamWrapper( aOutputFile ) );

                int bUseDTD = mpFilterInfo->maDTD.isEmpty() ? 0 : 1 ;
                int bUseDocType = mpFilterInfo->maDocType.isEmpty()  ? 0 : 1;
                Sequence< PropertyValue > aSourceData( 2 + bUseDTD + bUseDocType );
                int i = 0;

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "OutputStream" ) );
                aSourceData[i++].Value <<= xIS;

                aSourceData[i].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Indent" ) );
                aSourceData[i++].Value <<= (sal_Bool)sal_True;

                if( bUseDTD )
                {
                    aSourceData[i  ].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("DocType_System"));
                    aSourceData[i++].Value <<= mpFilterInfo->maDTD;
                }

                if( bUseDocType )
                {
                    aSourceData[i  ].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("DocType_Public"));
                    aSourceData[i++].Value <<= mpFilterInfo->maDocType;
                }

                Reference< XExportFilter > xExporter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.documentconversion.XSLTFilter" )) ), UNO_QUERY );
                Reference< XDocumentHandler > xHandler( xExporter, UNO_QUERY );
                if( xHandler.is() )
                {
                    Sequence< OUString > aFilterUserData( mpFilterInfo->getFilterUserData() );
                    xExporter->exporter( aSourceData, aFilterUserData );

                    Reference< XMultiServiceFactory > xDocFac( xComp, UNO_QUERY );

                    Reference< XEmbeddedObjectResolver > xObjectResolver;
                    Reference< XGraphicObjectResolver > xGrfResolver;

                    if( xDocFac.is() )
                    {
                        try
                        {
                            xGrfResolver = Reference< XGraphicObjectResolver >::query( xDocFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExportGraphicObjectResolver" )) ) );
                            xObjectResolver = Reference< XEmbeddedObjectResolver >::query( xDocFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExportEmbeddedObjectResolver" )) ) );
                        }
                        catch( const Exception& )
                        {
                        }
                    }

                    Sequence< Any > aArgs( 1 + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                    Any* pArgs = aArgs.getArray();
                    if( xGrfResolver.is() )         *pArgs++ <<= xGrfResolver;
                    if( xObjectResolver.is() )      *pArgs++ <<= xObjectResolver;

    //              *pArgs++ <<= xInfoSet;
                    *pArgs   <<= xHandler;

                    Reference< XFilter > xFilter( mxMSF->createInstanceWithArguments( pAppInfo->maXMLExporter, aArgs ), UNO_QUERY );
                    if( xFilter.is() )
                    {
                        Reference< XExporter > xExporter2( xFilter, UNO_QUERY );
                        if( xExporter2.is() )
                        {
                            xExporter2->setSourceDocument( xComp );

                            Sequence< PropertyValue > aDescriptor( 1 );
                            aDescriptor[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
                            aDescriptor[0].Value <<= aTempFileURL;

                            if( xFilter->filter( aDescriptor ) )
                                displayXMLFile( aTempFileURL );
                        }
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::doExport exception catched!" );
    }
}

void XMLFilterTestDialog::displayXMLFile( const OUString& rURL )
{
    Reference< XSystemShellExecute > xSystemShellExecute(
          SystemShellExecute::create(comphelper::getProcessComponentContext()) );
    xSystemShellExecute->execute( rURL, rtl::OUString(), SystemShellExecuteFlags::URIS_ONLY );
}

void XMLFilterTestDialog::onImportBrowse()
{
    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    String aFilterName( mpFilterInfo->maInterfaceName );
    String aExtensions;

    int nLastIndex = 0;
    int nCurrentIndex = 0;
    for( int i = 0; nLastIndex != -1; i++ )
    {
        nLastIndex = mpFilterInfo->maExtension.indexOf( sal_Unicode( ';' ), nLastIndex );

        if( i > 0 )
            aExtensions += ';';

        aExtensions += String( RTL_CONSTASCII_USTRINGPARAM("*.") );

        if( nLastIndex == -1 )
        {

            aExtensions += String( mpFilterInfo->maExtension.copy( nCurrentIndex ) );
        }
        else
        {
            aExtensions += String( mpFilterInfo->maExtension.copy( nCurrentIndex, nLastIndex - nCurrentIndex ) );
            nCurrentIndex = nLastIndex + 1;
            nLastIndex = nCurrentIndex;
        }
    }

    aFilterName += String( RTL_CONSTASCII_USTRINGPARAM( " (" ) );
    aFilterName += aExtensions;
    aFilterName += sal_Unicode(')');

    aDlg.AddFilter( aFilterName, aExtensions );
    aDlg.SetDisplayDirectory( maImportRecentFile );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        maImportRecentFile = aDlg.GetPath();
        import( maImportRecentFile );
    }

    initDialog();
}

void XMLFilterTestDialog::onImportRecentDocument()
{
    import( maImportRecentFile );
}

void XMLFilterTestDialog::import( const OUString& rURL )
{
    try
    {
        Reference< XComponentLoader > xLoader( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ), UNO_QUERY );
        Reference< XInteractionHandler > xInter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.InteractionHandler" )) ), UNO_QUERY );
        if( xLoader.is() && xInter.is() )
        {

            OUString aFrame( RTL_CONSTASCII_USTRINGPARAM( "_default" ) );
            Sequence< PropertyValue > aArguments(2);
            aArguments[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
            aArguments[0].Value <<= mpFilterInfo->maFilterName;
            aArguments[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
            aArguments[1].Value <<= xInter;

            xLoader->loadComponentFromURL( rURL, aFrame, 0, aArguments );
        }

        if( maCBXDisplaySource.IsChecked() )
        {
            String lead;
            String ext(RTL_CONSTASCII_USTRINGPARAM(".xml"));
            TempFile aTempFile(lead, &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            Reference< XImportFilter > xImporter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.documentconversion.XSLTFilter" )) ), UNO_QUERY );
            if( xImporter.is() )
            {
                osl::File aInputFile( rURL );
                aInputFile.open( osl_File_OpenFlag_Read );

                Reference< XInputStream > xIS( new comphelper::OSLInputStreamWrapper( aInputFile ) );

                Sequence< PropertyValue > aSourceData( 5 );
                int i = 0;

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ));
                aSourceData[i++].Value <<= xIS;

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ));
                aSourceData[i++].Value <<= rURL;

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Indent" ));
                aSourceData[i++].Value <<= (sal_Bool)sal_True;

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocType_Public" ));
                aSourceData[i++].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "-//OpenOffice.org//DTD OfficeDocument 1.0//EN" ));

                aSourceData[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocType_System" ));
                aSourceData[i++].Value <<= sDTDPath;

                Reference< XDocumentHandler > xWriter( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" )) ), UNO_QUERY );

                File aOutputFile( aTempFileURL );
                aOutputFile.open( osl_File_OpenFlag_Write );

                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );
                Reference< XActiveDataSource > xDocSrc( xWriter, UNO_QUERY );
                xDocSrc->setOutputStream( xOS );

                Sequence< OUString > aFilterUserData( mpFilterInfo->getFilterUserData() );
                xImporter->importer( aSourceData, xWriter, aFilterUserData );
            }

            displayXMLFile( aTempFileURL );
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("XMLFilterTestDialog::import catched an exception" );
    }
}

IMPL_LINK(XMLFilterTestDialog, ClickHdl_Impl, PushButton *, pButton )
{
    if( &maPBExportBrowse == pButton )
    {
        onExportBrowse();
    }
    else if( &maPBCurrentDocument == pButton )
    {
        onExportCurrentDocument();
    }
    else if( &maPBImportBrowse == pButton )
    {
        onImportBrowse();
    }
    else if( &maPBRecentDocument == pButton )
    {
        onImportRecentDocument();
    }
    else if( &maPBClose == pButton )
    {
        Close();
    }

    return 0;
}

/** returns the front most open component that supports the given service */
Reference< XComponent > XMLFilterTestDialog::getFrontMostDocument( const OUString& rServiceName )
{
    Reference< XComponent > xRet;

    try
    {
        Reference< XDesktop > xDesktop( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ), UNO_QUERY );
        if( xDesktop.is() )
        {
            Reference< XComponent > xTest( mxLastFocusModel );
            if( checkComponent( xTest, rServiceName ) )
            {
                xRet = xTest;
            }
            else
            {
                xTest = (Reference< XComponent >)xDesktop->getCurrentComponent();

                if( checkComponent( xTest, rServiceName ) )
                {
                    xRet = xTest;
                }
                else
                {
                    Reference< XEnumerationAccess > xAccess( xDesktop->getComponents() );
                    if( xAccess.is() )
                    {
                        Reference< XEnumeration > xEnum( xAccess->createEnumeration() );
                        if( xEnum.is() )
                        {
                            while( xEnum->hasMoreElements() )
                            {
                                if( (xEnum->nextElement() >>= xTest) && xTest.is() )
                                {
                                    if( checkComponent( xTest, rServiceName ) )
                                    {
                                        xRet = xTest;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::getFrontMostDocument exception catched!" );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
