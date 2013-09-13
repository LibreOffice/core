/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

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
    if( Event.EventName == "OnFocus" || Event.EventName == "OnUnload" )
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
                    if( !xInfo->supportsService("com.sun.star.presentation.PresentationDocument") )
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

XMLFilterTestDialog::XMLFilterTestDialog(Window* pParent,
    const Reference<XComponentContext>& rxContext)
    : ModalDialog(pParent, "TestXMLFilterDialog", "filter/ui/testxmlfilter.ui")
    , mxContext(rxContext)
    , m_pFilterInfo(NULL)
{
    get(m_pExport, "export");
    get(m_pFTExportXSLTFile, "exportxsltfile");
    get(m_pPBExportBrowse, "exportbrowse");
    get(m_pPBCurrentDocument, "currentdocument");
    get(m_pFTNameOfCurrentFile, "currentfilename");

    get(m_pImport, "import");
    get(m_pFTImportXSLTFile, "importxsltfile");
    get(m_pFTImportTemplate, "templateimport");
    get(m_pFTImportTemplateFile, "importxslttemplate");
    get(m_pCBXDisplaySource, "displaysource");
    get(m_pPBImportBrowse, "importbrowse");
    get(m_pPBRecentFile, "recentfile");
    get(m_pFTNameOfRecentFile, "recentfilename");

    get(m_pPBClose, "close");

    m_pPBExportBrowse->SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_pPBCurrentDocument->SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_pPBImportBrowse->SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_pPBRecentFile->SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_pPBClose->SetClickHdl(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );

    m_sDialogTitle = GetText();

    try
    {
        mxGlobalBroadcaster = GlobalEventBroadcaster::create(mxContext);
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

    delete m_pFilterInfo;
}

void XMLFilterTestDialog::test( const filter_info_impl& rFilterInfo )
{
    delete m_pFilterInfo;
    m_pFilterInfo = new filter_info_impl( rFilterInfo );

    m_sImportRecentFile = OUString();

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
        if( checkComponent( *pRef, m_pFilterInfo->maDocumentService ) )
            mxLastFocusModel = *pRef;
    }

    bool bExport = (m_pFilterInfo->maFlags & 2) == 2;
    Reference< XComponent > xCurrentDocument;
    if( bExport )
        xCurrentDocument = getFrontMostDocument( m_pFilterInfo->maDocumentService );
    m_pPBCurrentDocument->Enable( bExport && xCurrentDocument.is() );
    m_pFTNameOfCurrentFile->Enable( bExport && xCurrentDocument.is() );

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

        m_pFTNameOfCurrentFile->SetText( aTitle );
    }
}

void XMLFilterTestDialog::initDialog()
{
    DBG_ASSERT( m_pFilterInfo, "i need a filter I can test!" );
    if( NULL == m_pFilterInfo )
        return;

    OUString aTitle( m_sDialogTitle );
    aTitle = aTitle.replaceFirst( "%s", m_pFilterInfo->maFilterName );
    SetText( aTitle );

    bool bImport = (m_pFilterInfo->maFlags & 1) == 1;
    bool bExport = (m_pFilterInfo->maFlags & 2) == 2;

    updateCurrentDocumentButtonState();

    m_pExport->Enable(bExport);
    m_pFTExportXSLTFile->SetText( getFileNameFromURL( m_pFilterInfo->maExportXSLT ) );

    // ---

    m_pImport->Enable(bImport);
    m_pFTImportTemplate->Enable(bImport && !m_pFilterInfo->maImportTemplate.isEmpty());
    m_pFTImportTemplateFile->Enable(bImport && !m_pFilterInfo->maImportTemplate.isEmpty());
    m_pPBRecentFile->Enable(bImport && !m_sImportRecentFile.isEmpty());
    m_pFTNameOfRecentFile->Enable(bImport && !m_sImportRecentFile.isEmpty());

    m_pFTImportXSLTFile->SetText( getFileNameFromURL( m_pFilterInfo->maImportXSLT ) );
    m_pFTImportTemplateFile->SetText( getFileNameFromURL( m_pFilterInfo->maImportTemplate ) );
    m_pFTNameOfRecentFile->SetText( getFileNameFromURL( m_sImportRecentFile ) );
}

void XMLFilterTestDialog::onExportBrowse()
{
    try
    {
        // Open Fileopen-Dialog
           ::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );

        Reference< XNameAccess > xFilterContainer( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", mxContext ), UNO_QUERY );
        Reference< XNameAccess > xTypeDetection( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.TypeDetection", mxContext ), UNO_QUERY );
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

                if( (nFound == 15) && (!aType.isEmpty() && aService == m_pFilterInfo->maDocumentService) )
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
                                                aExtension += ";";
                                            aExtension += "*.";
                                            aExtension += (*pExtensions++);
                                        }
                                    }
                                }
                            }

                            OUString aFilterName( aInterfaceName + " (" + aExtension + ")" );

                            aDlg.AddFilter( aFilterName, aExtension );

                            if( (nFlags & 0x100) == 0x100 )
                                aDlg.SetCurrentFilter( aFilterName );
                        }
                    }
                }

            }
        }

        aDlg.SetDisplayDirectory( m_sExportRecentFile );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            m_sExportRecentFile = aDlg.GetPath();

            Reference< XDesktop2 > xLoader = Desktop::create( mxContext );
            Reference< XInteractionHandler2 > xInter = InteractionHandler::createWithParent(mxContext, 0);
            OUString aFrame( "_default" );
            Sequence< PropertyValue > aArguments(1);
            aArguments[0].Name = OUString( "InteractionHandler" );
            aArguments[0].Value <<= xInter;
            Reference< XComponent > xComp( xLoader->loadComponentFromURL( m_sExportRecentFile, aFrame, 0, aArguments ) );
            if( xComp.is() )
            {
                doExport( xComp );
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
    doExport( getFrontMostDocument( m_pFilterInfo->maDocumentService ) );
}

void XMLFilterTestDialog::doExport( Reference< XComponent > xComp )
{
    try
    {
        Reference< XStorable > xStorable( xComp, UNO_QUERY );
        if( xStorable.is() )
        {
            OUString const ext(RTL_CONSTASCII_USTRINGPARAM(".xml"));
            utl::TempFile aTempFile(OUString(), &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            const application_info_impl* pAppInfo = getApplicationInfo( m_pFilterInfo->maExportService );
            if( pAppInfo )
            {
                File aOutputFile( aTempFileURL );
                /* File::RC rc = */ aOutputFile.open( osl_File_OpenFlag_Write );

                // create xslt exporter
                Reference< XOutputStream > xIS( new comphelper::OSLOutputStreamWrapper( aOutputFile ) );
                int bUseDocType = m_pFilterInfo->maDocType.isEmpty()  ? 0 : 1;
                Sequence< PropertyValue > aSourceData( 2 + bUseDocType );
                int i = 0;


                aSourceData[i  ].Name = OUString( "OutputStream" );
                aSourceData[i++].Value <<= xIS;

                aSourceData[i].Name = OUString( "Indent" );
                aSourceData[i++].Value <<= (sal_Bool)sal_True;

                if( bUseDocType )
                    {
                        aSourceData[i  ].Name = OUString("DocType_Public");
                        aSourceData[i++].Value <<= m_pFilterInfo->maDocType;
                    }

                Reference< XExportFilter > xExporter( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.documentconversion.XSLTFilter", mxContext ), UNO_QUERY );
                Reference< XDocumentHandler > xHandler( xExporter, UNO_QUERY );
                if( xHandler.is() )
                {
                    Sequence< OUString > aFilterUserData( m_pFilterInfo->getFilterUserData() );
                    xExporter->exporter( aSourceData, aFilterUserData );

                    Reference< XMultiServiceFactory > xDocFac( xComp, UNO_QUERY );

                    Reference< XEmbeddedObjectResolver > xObjectResolver;
                    Reference< XGraphicObjectResolver > xGrfResolver;

                    if( xDocFac.is() )
                    {
                        try
                        {
                            xGrfResolver = Reference< XGraphicObjectResolver >::query( xDocFac->createInstance("com.sun.star.document.ExportGraphicObjectResolver") );
                            xObjectResolver = Reference< XEmbeddedObjectResolver >::query( xDocFac->createInstance("com.sun.star.document.ExportEmbeddedObjectResolver") );
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

                    Reference< XFilter > xFilter( mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( pAppInfo->maXMLExporter, aArgs, mxContext ), UNO_QUERY );
                    if( xFilter.is() )
                    {
                        Reference< XExporter > xExporter2( xFilter, UNO_QUERY );
                        if( xExporter2.is() )
                        {
                            xExporter2->setSourceDocument( xComp );

                            Sequence< PropertyValue > aDescriptor( 1 );
                            aDescriptor[0].Name = OUString( "FileName" );
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
    xSystemShellExecute->execute( rURL, OUString(), SystemShellExecuteFlags::URIS_ONLY );
}

void XMLFilterTestDialog::onImportBrowse()
{
    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    OUString aFilterName( m_pFilterInfo->maInterfaceName );
    OUString aExtensions;

    int nLastIndex = 0;
    int nCurrentIndex = 0;
    for( int i = 0; nLastIndex != -1; i++ )
    {
        nLastIndex = m_pFilterInfo->maExtension.indexOf( ';', nLastIndex );

        if( i > 0 )
            aExtensions += ";";

        aExtensions += "*.";

        if( nLastIndex == -1 )
        {

            aExtensions += m_pFilterInfo->maExtension.copy( nCurrentIndex );
        }
        else
        {
            aExtensions += m_pFilterInfo->maExtension.copy( nCurrentIndex, nLastIndex - nCurrentIndex );
            nCurrentIndex = nLastIndex + 1;
            nLastIndex = nCurrentIndex;
        }
    }

    aFilterName += " (" + aExtensions + ")";

    aDlg.AddFilter( aFilterName, aExtensions );
    aDlg.SetDisplayDirectory( m_sImportRecentFile );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        m_sImportRecentFile = aDlg.GetPath();
        import( m_sImportRecentFile );
    }

    initDialog();
}

void XMLFilterTestDialog::onImportRecentDocument()
{
    import( m_sImportRecentFile );
}

void XMLFilterTestDialog::import( const OUString& rURL )
{
    try
    {
        Reference< XDesktop2 > xLoader = Desktop::create( mxContext );
        Reference< XInteractionHandler2 > xInter = InteractionHandler::createWithParent(mxContext, 0);

        OUString aFrame( "_default" );
        Sequence< PropertyValue > aArguments(2);
        aArguments[0].Name = OUString( "FilterName" );
        aArguments[0].Value <<= m_pFilterInfo->maFilterName;
        aArguments[1].Name = OUString( "InteractionHandler" );
        aArguments[1].Value <<= xInter;

        xLoader->loadComponentFromURL( rURL, aFrame, 0, aArguments );

        if( m_pCBXDisplaySource->IsChecked() )
        {
            OUString const ext(RTL_CONSTASCII_USTRINGPARAM(".xml"));
            TempFile aTempFile(OUString(), &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            Reference< XImportFilter > xImporter( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.documentconversion.XSLTFilter", mxContext ), UNO_QUERY );
            if( xImporter.is() )
            {
                osl::File aInputFile( rURL );
                aInputFile.open( osl_File_OpenFlag_Read );

                Reference< XInputStream > xIS( new comphelper::OSLInputStreamWrapper( aInputFile ) );

                Sequence< PropertyValue > aSourceData( 3 );
                int i = 0;

                aSourceData[i  ].Name = OUString( "InputStream" );
                aSourceData[i++].Value <<= xIS;

                aSourceData[i  ].Name = OUString( "FileName" );
                aSourceData[i++].Value <<= rURL;

                aSourceData[i  ].Name = OUString( "Indent" );
                aSourceData[i++].Value <<= (sal_Bool)sal_True;

                Reference< XWriter > xWriter = Writer::create( mxContext );

                File aOutputFile( aTempFileURL );
                aOutputFile.open( osl_File_OpenFlag_Write );

                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );
                xWriter->setOutputStream( xOS );

                Sequence< OUString > aFilterUserData( m_pFilterInfo->getFilterUserData() );
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
    if (m_pPBExportBrowse == pButton)
    {
        onExportBrowse();
    }
    else if (m_pPBCurrentDocument == pButton)
    {
        onExportCurrentDocument();
    }
    else if (m_pPBImportBrowse == pButton)
    {
        onImportBrowse();
    }
    else if (m_pPBRecentFile == pButton)
    {
        onImportRecentDocument();
    }
    else if (m_pPBClose == pButton)
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
        Reference< XDesktop2 > xDesktop = Desktop::create( mxContext );
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
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::getFrontMostDocument exception catched!" );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
