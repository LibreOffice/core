/*************************************************************************
 *
 *  $RCSfile: xmlfiltertestdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:36:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XGRAPHICOBJECTRESOLVER_HXX_
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTRESOLVER_HXX_
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_XIMPORTFILTER_HPP_
#include <com/sun/star/xml/XImportFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_XEXPORTFILTER_HPP_
#include <com/sun/star/xml/XExportFilter.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif

#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#include <comphelper/oslfile2streamwrap.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfiltertestdialog.hrc"
#include "xmlfileview.hxx"


using namespace rtl;
using namespace utl;
using namespace osl;
using namespace vos;
using namespace comphelper;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::frame;
using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;

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
    OGuard aGuard( Application::GetSolarMutex() );
    if( (Event.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnFocus") ) == 0) ||
        (Event.EventName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("OnUnload") ) == 0) )
    {
        Reference< XComponent > xComp( Event.Source, UNO_QUERY );
        mpDialog->updateCurrentDocumentButtonState( &xComp );
    }
}

void SAL_CALL GlobalEventListenerImpl::disposing( const com::sun::star::lang::EventObject& Source ) throw (RuntimeException)
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
                if( rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) )
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
    catch( Exception& )
    {
        DBG_ERROR( "checkComponent exception catched!" );
    }

    return false;
}

XMLFilterTestDialog::XMLFilterTestDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF ) :
    ModalDialog( pParent, ResId( DLG_XML_FILTER_TEST_DIALOG, &rResMgr ) ),
    mxMSF( rxMSF ),
    mrResMgr( rResMgr ),

    maFLExport( this, ResId( FL_EXPORT, &rResMgr ) ),
    maFTExportXSLT( this, ResId( FT_EXPORT_XSLT, &rResMgr ) ),
    maFTExportXSLTFile( this, ResId( FT_EXPORT_XSLT_FILE, &rResMgr ) ),
    maFTTransformDocument( this, ResId( FT_TRANSFORM_DOCUMENT, &rResMgr ) ),
    maPBExportBrowse( this, ResId( PB_EXPORT_BROWSE, &rResMgr ) ),
    maPBCurrentDocument( this, ResId( PB_CURRENT_DOCUMENT, &rResMgr ) ),
    maFTNameOfCurentFile( this, ResId( FT_NAME_OF_CURRENT_FILE, &rResMgr ) ),
    maFLImport( this, ResId( FL_IMPORT, &rResMgr ) ),
    maFTImportXSLT( this, ResId( FT_IMPORT_XSLT, &rResMgr ) ),
    maFTImportXSLTFile( this, ResId( FT_IMPORT_XSLT_FILE, &rResMgr ) ),
    maFTImportTemplate( this, ResId( FT_IMPORT_TEMPLATE, &rResMgr ) ),
    maFTImportTemplateFile( this, ResId( FT_IMPORT_TEMPLATE_FILE, &rResMgr ) ),
    maFTTransformFile( this, ResId( FT_TRANSFORM_FILE, &rResMgr ) ),
    maCBXDisplaySource( this, ResId( CBX_DISPLAY_SOURCE, &rResMgr ) ),
    maPBImportBrowse( this, ResId( PB_IMPORT_BROWSE, &rResMgr ) ),
    maPBRecentDocument( this, ResId( PB_RECENT_DOCUMENT, &rResMgr ) ),
    maFTNameOfRecentFile( this, ResId( FT_NAME_OF_RECENT_FILE, &rResMgr ) ),
    maPBHelp( this, ResId( PB_HELP, &rResMgr ) ),
    maPBClose( this, ResId( PB_CLOSE, &rResMgr ) ),
    mpSourceDLG( NULL ),
    mpFilterInfo( NULL ),
    sDTDPath( RTL_CONSTASCII_USTRINGPARAM( "$(inst)/share/dtd/officedocument/1_0/office.dtd" ) )
{
    maPBExportBrowse.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBCurrentDocument.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBImportBrowse.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBRecentDocument.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    maPBClose.SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );

    maDialogTitle = GetText();

    try
    {
        Reference< XConfigManager > xCfgMgr( mxMSF->createInstance( OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")), UNO_QUERY );
        if( xCfgMgr.is() )
            sDTDPath = xCfgMgr->substituteVariables( sDTDPath );

        mxGlobalBroadcaster = Reference < XEventBroadcaster >::query( mxMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster") ) );
        if ( mxGlobalBroadcaster.is() )
        {
            mxGlobalEventListener = new GlobalEventListenerImpl( this );
            mxGlobalBroadcaster->addEventListener( mxGlobalEventListener );
        }
    }
    catch( Exception& e )
    {
        DBG_ERROR( "XMLFilterTestDialog::XMLFilterTestDialog exception catched!" );
    }
}

XMLFilterTestDialog::~XMLFilterTestDialog()
{
    FreeResource();

    try
    {
        if( mxGlobalBroadcaster.is() )
            mxGlobalBroadcaster->removeEventListener( mxGlobalEventListener );
    }
    catch( Exception& e )
    {
        DBG_ERROR( "XMLFilterTestDialog::~XMLFilterTestDialog exception catched!" );
    }

    delete mpSourceDLG;
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

String getSystemPathFromFileURL( const OUString& rURL )
{
    OUString aPath;

    osl::FileBase::getSystemPathFromFileURL( rURL, aPath );

    return aPath;
}

static OUString getFileNameFromURL( OUString& rURL )
{
    INetURLObject aURL( rURL );
    OUString aName( aURL.getName() );
    if( 0 == aName.getLength() )
    {
        sal_Int32 n = aName.lastIndexOf( sal_Unicode('/') );
        if( n != -1 )
            aName = aName.copy( n+1 );
    }

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
        Reference< XDocumentInfoSupplier > xInfoSupplier( xCurrentDocument, UNO_QUERY );
        if( xInfoSupplier.is() )
        {
            Reference< XPropertySet > xInfo( xInfoSupplier->getDocumentInfo(), UNO_QUERY );
            if( xInfo.is() )
            {
                xInfo->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) ) >>= aTitle;
            }
        }

        if( 0 == aTitle.getLength() )
        {
            Reference< XStorable > xStorable( xCurrentDocument, UNO_QUERY );
            if( xStorable.is() )
            {
                if( xStorable->hasLocation() )
                {
                    aTitle = getFileNameFromURL( xStorable->getLocation() );
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
    maFTImportTemplate.Enable( bImport && mpFilterInfo->maImportTemplate.getLength() );
    maFTImportTemplateFile.Enable( bImport && mpFilterInfo->maImportTemplate.getLength() );
    maFTTransformFile.Enable( bImport );
    maCBXDisplaySource.Enable( bImport );
    maPBImportBrowse.Enable( bImport );
    maPBRecentDocument.Enable( bImport && maImportRecentFile.getLength() );
    maFTNameOfRecentFile.Enable( bImport && maImportRecentFile.getLength() );

    maFTImportXSLTFile.SetText( getFileNameFromURL( mpFilterInfo->maImportXSLT ) );
    maFTImportTemplateFile.SetText( getFileNameFromURL( mpFilterInfo->maImportTemplate ) );
    maFTNameOfRecentFile.SetText( getFileNameFromURL( maImportRecentFile ) );
}

void XMLFilterTestDialog::onExportBrowse()
{
    try
    {
        // Open Fileopen-Dialog
           ::sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );

        Reference< XNameAccess > xFilterContainer( mxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.FilterFactory" ) ), UNO_QUERY );
        Reference< XNameAccess > xTypeDetection( mxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.TypeDetection" ) ), UNO_QUERY );
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
                sal_Int32 nFlags;

                int nFound = 0;

                for( sal_Int32  nValue = 0; (nValue < aValues.getLength()) && (nFound != 15); nValue++, pValues++ )
                {
                    if( pValues->Name.equalsAscii( "Type" ) )
                    {
                        pValues->Value >>= aType;
                        nFound |= 1;
                    }
                    else if( pValues->Name.equalsAscii( "DocumentService" ) )
                    {
                        pValues->Value >>= aService;
                        nFound |= 2;
                    }
                    else if( pValues->Name.equalsAscii( "Flags" ) )
                    {
                        pValues->Value >>= nFlags;
                        nFound |= 4;
                    }
                    if( pValues->Name.equalsAscii( "UIName" ) )
                    {
                        pValues->Value >>= aInterfaceName;
                        nFound |= 8;
                    }

                }

                if( (nFound == 15) && (aType.getLength() && aService == mpFilterInfo->maDocumentService) )
                {
                    // see if this filter is not supressed in dialog
                    if( (nFlags & 0x1000) == 0 )
                    {
                        Any aAny( xTypeDetection->getByName( aType ) );
                        Sequence< PropertyValue > aValues;

                        if( aAny >>= aValues )
                        {
                            PropertyValue* pValues = aValues.getArray();
                            sal_Int32 nValue;

                            OUString aExtension;
                            for( nValue = 0; nValue < aValues.getLength(); nValue++, pValues++ )
                            {
                                if( pValues->Name.equalsAscii( "Extensions" ) )
                                {
                                    Sequence< OUString > aExtensions;
                                    if( pValues->Value >>= aExtensions )
                                    {
                                        const sal_Int32 nCount( aExtensions.getLength() );
                                        OUString* pExtensions = aExtensions.getArray();
                                        sal_Int32 n;
                                        for( n = 0; n < nCount; n++ )
                                        {
                                            if( n > 0 )
                                                aExtension += OUString( sal_Unicode(';') );
                                            aExtension += OUString::createFromAscii("*.");
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

            Reference< XComponentLoader > xLoader( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
            Reference< XInteractionHandler > xInter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.task.InteractionHandler" ) ), UNO_QUERY );
            if( xLoader.is() && xInter.is() )
            {
                OUString aFrame( RTL_CONSTASCII_USTRINGPARAM( "_default" ) );
                Sequence< PropertyValue > aArguments(1);
                aArguments[0].Name == OUString::createFromAscii( "InteractionHandler" );
                aArguments[0].Value <<= xInter;
                Reference< XComponent > xComp( xLoader->loadComponentFromURL( maExportRecentFile, aFrame, 0, aArguments ) );
                if( xComp.is() )
                {
                    doExport( xComp );
                }
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("XMLFilterTestDialog::onExportBrowse exception caught!");
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
            utl::TempFile aTempFile;
            OUString aTempFileURL( aTempFile.GetURL() );

            const application_info_impl* pAppInfo = getApplicationInfo( mpFilterInfo->maDocumentService );
            if( pAppInfo )
            {
                File aOutputFile( aTempFileURL );
                File::RC rc = aOutputFile.open( OpenFlag_Write );

                // create xslt exporter
                Reference< XOutputStream > xIS( new comphelper::OSLOutputStreamWrapper( aOutputFile ) );

                int bUseDTD = (mpFilterInfo->maDTD.getLength() != 0) ? 1 : 0 ;
                int bUseDocType = (mpFilterInfo->maDocType.getLength() != 0 ) ? 1 : 0;
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

                Reference< XExportFilter > xExporter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.documentconversion.XSLTFilter" ) ), UNO_QUERY );
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
                            xGrfResolver = Reference< XGraphicObjectResolver >::query( xDocFac->createInstance( OUString::createFromAscii("com.sun.star.document.ExportGraphicObjectResolver") ) );
                            xObjectResolver = Reference< XEmbeddedObjectResolver >::query( xDocFac->createInstance( OUString::createFromAscii("com.sun.star.document.ExportEmbeddedObjectResolver") ) );
                        }
                        catch( Exception& )
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
                        Reference< XExporter > xExporter( xFilter, UNO_QUERY );
                        if( xExporter.is() )
                        {
                            xExporter->setSourceDocument( xComp );

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
    catch( Exception& )
    {
        DBG_ERROR( "XMLFilterTestDialog::doExport exception catched!" );
    }
}

void XMLFilterTestDialog::displayXMLFile( const OUString& rURL )
{
    if( NULL == mpSourceDLG )
        mpSourceDLG = new XMLSourceFileDialog( NULL, mrResMgr, mxMSF );

    mpSourceDLG->Show( rURL, mpFilterInfo);
}

void XMLFilterTestDialog::onImportBrowse()
{
    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );
    String aFilterName( mpFilterInfo->maInterfaceName );
    String aExtensions;

    int nLastIndex = 0;
    int nCurrentIndex = 0;
    for( int i = 0; nLastIndex != -1; i++ )
    {
        nLastIndex = mpFilterInfo->maExtension.indexOf( sal_Unicode( ';' ), nLastIndex );

        if( i > 0 )
            aExtensions += ';';

        aExtensions += String( RTL_CONSTASCII_STRINGPARAM("*.") );

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
        Reference< XComponentLoader > xLoader( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
        Reference< XInteractionHandler > xInter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.task.InteractionHandler" ) ), UNO_QUERY );
        if( xLoader.is() && xInter.is() )
        {

            OUString aFrame( RTL_CONSTASCII_USTRINGPARAM( "_default" ) );
            Sequence< PropertyValue > aArguments(2);
            aArguments[0].Name == OUString::createFromAscii( "FilterName" );
            aArguments[0].Value <<= mpFilterInfo->maFilterName;
            aArguments[1].Name == OUString::createFromAscii( "InteractionHandler" );
            aArguments[1].Value <<= xInter;

            xLoader->loadComponentFromURL( rURL, aFrame, 0, aArguments );
        }

        if( maCBXDisplaySource.IsChecked() )
        {
            TempFile aTempFile;
            OUString aTempFileURL( aTempFile.GetURL() );

            Reference< XImportFilter > xImporter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.documentconversion.XSLTFilter" ) ), UNO_QUERY );
            if( xImporter.is() )
            {
                osl::File aInputFile( rURL );
                osl::File::RC rc = aInputFile.open( OpenFlag_Read );

                Reference< XInputStream > xIS( new comphelper::OSLInputStreamWrapper( aInputFile ) );

                Sequence< PropertyValue > aSourceData( 5 );
                int i = 0;

                aSourceData[i  ].Name = OUString::createFromAscii( "InputStream" );
                aSourceData[i++].Value <<= xIS;

                aSourceData[i  ].Name = OUString::createFromAscii( "FileName" );
                aSourceData[i++].Value <<= rURL;

                aSourceData[i  ].Name = OUString::createFromAscii( "Indent" );
                aSourceData[i++].Value <<= (sal_Bool)sal_True;

                aSourceData[i  ].Name = OUString::createFromAscii("DocType_Public");
                aSourceData[i++].Value <<= OUString::createFromAscii("-//OpenOffice.org//DTD OfficeDocument 1.0//EN");

                aSourceData[i  ].Name = OUString::createFromAscii("DocType_System");
                aSourceData[i++].Value <<= sDTDPath;

                Reference< XDocumentHandler > xWriter( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ) ), UNO_QUERY );

                File aOutputFile( aTempFileURL );
                rc = aOutputFile.open( OpenFlag_Write );

                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );
                Reference< XActiveDataSource > xDocSrc( xWriter, UNO_QUERY );
                xDocSrc->setOutputStream( xOS );

                Sequence< OUString > aFilterUserData( mpFilterInfo->getFilterUserData() );
                xImporter->importer( aSourceData, xWriter, aFilterUserData );
            }

            displayXMLFile( aTempFileURL );
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("XMLFilterTestDialog::import catched an exception" );
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
        Reference< XDesktop > xDesktop( mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );
        if( xDesktop.is() )
        {
            Reference< XComponent > xTest( mxLastFocusModel );
            if( checkComponent( xTest, rServiceName ) )
            {
                xRet = xTest;
            }
            else
            {
                Reference< XComponent > xTest( xDesktop->getCurrentComponent() );

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
    catch( Exception& )
    {
        DBG_ERROR( "XMLFilterTestDialog::getFrontMostDocument exception catched!" );
    }

    return xRet;
}


const filter_info_impl* XMLFilterTestDialog::getFilterInfo() const
{
    return mpFilterInfo;
}

