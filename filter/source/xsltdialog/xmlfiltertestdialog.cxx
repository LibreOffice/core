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
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/Desktop.hpp>
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
#include <sfx2/filedlghelper.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>

#include <strings.hrc>
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


class GlobalEventListenerImpl : public ::cppu::WeakImplHelper< css::document::XDocumentEventListener >
{
public:
    explicit GlobalEventListenerImpl( XMLFilterTestDialog* pDialog );

    // XDocumentEventListener
    virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
private:
    XMLFilterTestDialog* mpDialog;
};

GlobalEventListenerImpl::GlobalEventListenerImpl( XMLFilterTestDialog* pDialog )
: mpDialog( pDialog )
{
}

void SAL_CALL GlobalEventListenerImpl::documentEventOccured( const css::document::DocumentEvent& Event )
{
    ::SolarMutexGuard aGuard;
    if( Event.EventName == "OnFocus" || Event.EventName == "OnUnload" )
    {
        Reference< XComponent > xComp( Event.Source, UNO_QUERY );
        mpDialog->updateCurrentDocumentButtonState( &xComp );
    }
}

void SAL_CALL GlobalEventListenerImpl::disposing( const css::lang::EventObject& /* Source */ )
{
}

/** returns true if the given component supports the given service */
static bool checkComponent( Reference< XComponent > const & rxComponent, const OUString& rServiceName )
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
                    // so if we want a draw we need to check if it's not an impress
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
        OSL_FAIL( "checkComponent exception caught!" );
    }

    return false;
}

XMLFilterTestDialog::XMLFilterTestDialog(weld::Window* pParent,
    const Reference<XComponentContext>& rxContext)
    : GenericDialogController(pParent, "filter/ui/testxmlfilter.ui", "TestXMLFilterDialog")
    , mxContext(rxContext)
    , m_xExport(m_xBuilder->weld_widget("export"))
    , m_xFTExportXSLTFile(m_xBuilder->weld_label("exportxsltfile"))
    , m_xPBExportBrowse(m_xBuilder->weld_button("exportbrowse"))
    , m_xPBCurrentDocument(m_xBuilder->weld_button("currentdocument"))
    , m_xFTNameOfCurrentFile(m_xBuilder->weld_label("currentfilename"))
    , m_xImport(m_xBuilder->weld_widget("import"))
    , m_xFTImportXSLTFile(m_xBuilder->weld_label("importxsltfile"))
    , m_xFTImportTemplate(m_xBuilder->weld_label("templateimport"))
    , m_xFTImportTemplateFile(m_xBuilder->weld_label("importxslttemplate"))
    , m_xCBXDisplaySource(m_xBuilder->weld_check_button("displaysource"))
    , m_xPBImportBrowse(m_xBuilder->weld_button("importbrowse"))
    , m_xPBRecentFile(m_xBuilder->weld_button("recentfile"))
    , m_xFTNameOfRecentFile(m_xBuilder->weld_label("recentfilename"))
    , m_xPBClose(m_xBuilder->weld_button("close"))
{
    m_xPBExportBrowse->connect_clicked(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_xPBCurrentDocument->connect_clicked(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_xPBImportBrowse->connect_clicked(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_xPBRecentFile->connect_clicked(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );
    m_xPBClose->connect_clicked(LINK( this, XMLFilterTestDialog, ClickHdl_Impl ) );

    m_sDialogTitle = m_xDialog->get_title();

    try
    {
        mxGlobalBroadcaster = theGlobalEventBroadcaster::get(mxContext);
        mxGlobalEventListener = new GlobalEventListenerImpl( this );
        mxGlobalBroadcaster->addDocumentEventListener( mxGlobalEventListener );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::XMLFilterTestDialog exception caught!" );
    }
}

XMLFilterTestDialog::~XMLFilterTestDialog()
{
    try
    {
        if( mxGlobalBroadcaster.is() )
            mxGlobalBroadcaster->removeDocumentEventListener( mxGlobalEventListener );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterTestDialog::~XMLFilterTestDialog exception caught!" );
    }
}

void XMLFilterTestDialog::test( const filter_info_impl& rFilterInfo )
{
    m_xFilterInfo.reset(new filter_info_impl( rFilterInfo ));

    m_sImportRecentFile.clear();

    initDialog();

    m_xDialog->run();
}

static OUString getFileNameFromURL( OUString const & rURL )
{
    INetURLObject aURL( rURL );
    OUString aName( aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset) );
    return aName;
}

void XMLFilterTestDialog::updateCurrentDocumentButtonState( Reference< XComponent > const * pRef /* = NULL */ )
{
    if( pRef && pRef->is() )
    {
        if( checkComponent( *pRef, m_xFilterInfo->maDocumentService ) )
            mxLastFocusModel = *pRef;
    }

    bool bExport = (m_xFilterInfo->maFlags & 2) == 2;
    Reference< XComponent > xCurrentDocument;
    if( bExport )
        xCurrentDocument = getFrontMostDocument( m_xFilterInfo->maDocumentService );
    m_xPBCurrentDocument->set_sensitive( bExport && xCurrentDocument.is() );
    m_xFTNameOfCurrentFile->set_sensitive( bExport && xCurrentDocument.is() );

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

        m_xFTNameOfCurrentFile->set_label( aTitle );
    }
}

void XMLFilterTestDialog::initDialog()
{
    DBG_ASSERT( m_xFilterInfo, "i need a filter I can test!" );
    if( nullptr == m_xFilterInfo )
        return;

    OUString aTitle( m_sDialogTitle );
    aTitle = aTitle.replaceFirst( "%s", m_xFilterInfo->maFilterName );
    m_xDialog->set_title( aTitle );

    bool bImport = (m_xFilterInfo->maFlags & 1) == 1;
    bool bExport = (m_xFilterInfo->maFlags & 2) == 2;

    updateCurrentDocumentButtonState();

    m_xExport->set_sensitive(bExport);
    m_xFTExportXSLTFile->set_label( getFileNameFromURL( m_xFilterInfo->maExportXSLT ) );


    m_xImport->set_sensitive(bImport);
    m_xFTImportTemplate->set_sensitive(bImport && !m_xFilterInfo->maImportTemplate.isEmpty());
    m_xFTImportTemplateFile->set_sensitive(bImport && !m_xFilterInfo->maImportTemplate.isEmpty());
    m_xPBRecentFile->set_sensitive(bImport && !m_sImportRecentFile.isEmpty());
    m_xFTNameOfRecentFile->set_sensitive(bImport && !m_sImportRecentFile.isEmpty());

    m_xFTImportXSLTFile->set_label( getFileNameFromURL( m_xFilterInfo->maImportXSLT ) );
    m_xFTImportTemplateFile->set_label( getFileNameFromURL( m_xFilterInfo->maImportTemplate ) );
    m_xFTNameOfRecentFile->set_label( getFileNameFromURL( m_sImportRecentFile ) );
}

void XMLFilterTestDialog::onExportBrowse()
{
    try
    {
        // Open Fileopen-Dialog
        ::sfx2::FileDialogHelper aDlg(
            css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::NONE, m_xDialog.get());

        Reference< XNameAccess > xFilterContainer( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", mxContext ), UNO_QUERY );
        Reference< XNameAccess > xTypeDetection( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.TypeDetection", mxContext ), UNO_QUERY );
        if( xFilterContainer.is() && xTypeDetection.is() )
        {
            Sequence< OUString > aFilterNames( xFilterContainer->getElementNames() );

            for( OUString const & filterName : aFilterNames )
            {
                Sequence< PropertyValue > aValues;

                Any aAny( xFilterContainer->getByName( filterName ) );
                if( !(aAny >>= aValues) )
                    continue;

                OUString aInterfaceName;
                OUString aType, aService;
                sal_Int32 nFlags( 0 );

                int nFound = 0;

                for( const PropertyValue& rValue : aValues )
                {
                    if ( rValue.Name == "Type" )
                    {
                        rValue.Value >>= aType;
                        nFound |= 1;
                    }
                    else if ( rValue.Name == "DocumentService" )
                    {
                        rValue.Value >>= aService;
                        nFound |= 2;
                    }
                    else if ( rValue.Name == "Flags" )
                    {
                        rValue.Value >>= nFlags;
                        nFound |= 4;
                    }
                    else if ( rValue.Name == "UIName" )
                    {
                        rValue.Value >>= aInterfaceName;
                        nFound |= 8;
                    }

                    if (nFound == 15)
                        break;
                }

                if( (nFound == 15) && (!aType.isEmpty() && aService == m_xFilterInfo->maDocumentService) )
                {
                    // see if this filter is not suppressed in dialog
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
                                            aExtension += *pExtensions++;
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
            Reference< XInteractionHandler2 > xInter = InteractionHandler::createWithParent(mxContext, nullptr);
            Sequence< PropertyValue > aArguments(1);
            aArguments[0].Name = "InteractionHandler";
            aArguments[0].Value <<= xInter;
            Reference< XComponent > xComp( xLoader->loadComponentFromURL( m_sExportRecentFile, "_default", 0, aArguments ) );
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
    doExport( getFrontMostDocument( m_xFilterInfo->maDocumentService ) );
}

void XMLFilterTestDialog::doExport( const Reference< XComponent >& xComp )
{
    try
    {
        Reference< XStorable > xStorable( xComp, UNO_QUERY );
        if( xStorable.is() )
        {
            OUString const ext(".xml");
            utl::TempFile aTempFile(OUString(), true, &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            const application_info_impl* pAppInfo = getApplicationInfo( m_xFilterInfo->maExportService );
            if( pAppInfo )
            {
                File aOutputFile( aTempFileURL );
                /* File::RC rc = */ aOutputFile.open( osl_File_OpenFlag_Write );

                // create xslt exporter
                Reference< XOutputStream > xIS( new comphelper::OSLOutputStreamWrapper( aOutputFile ) );
                int bUseDocType = m_xFilterInfo->maDocType.isEmpty()  ? 0 : 1;
                Sequence< PropertyValue > aSourceData( 2 + bUseDocType );
                int i = 0;


                aSourceData[i  ].Name = "OutputStream";
                aSourceData[i++].Value <<= xIS;

                aSourceData[i].Name = "Indent";
                aSourceData[i++].Value <<= true;

                if( bUseDocType )
                    {
                        aSourceData[i  ].Name = "DocType_Public";
                        aSourceData[i++].Value <<= m_xFilterInfo->maDocType;
                    }

                Reference< XExportFilter > xExporter( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.documentconversion.XSLTFilter", mxContext ), UNO_QUERY );
                Reference< XDocumentHandler > xHandler( xExporter, UNO_QUERY );
                if( xHandler.is() )
                {
                    Sequence< OUString > aFilterUserData( m_xFilterInfo->getFilterUserData() );
                    xExporter->exporter( aSourceData, aFilterUserData );

                    Reference< XMultiServiceFactory > xDocFac( xComp, UNO_QUERY );

                    Reference< XEmbeddedObjectResolver > xObjectResolver;
                    Reference<XGraphicStorageHandler> xGraphicStorageHandler;

                    if( xDocFac.is() )
                    {
                        try
                        {
                            xGraphicStorageHandler.set(xDocFac->createInstance("com.sun.star.document.ExportGraphicStorageHandler"), UNO_QUERY);
                            xObjectResolver.set( xDocFac->createInstance("com.sun.star.document.ExportEmbeddedObjectResolver"), UNO_QUERY );
                        }
                        catch( const Exception& )
                        {
                        }
                    }

                    Sequence< Any > aArgs( 1 + ( xGraphicStorageHandler.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                    Any* pArgs = aArgs.getArray();
                    if (xGraphicStorageHandler.is())
                        *pArgs++ <<= xGraphicStorageHandler;

                    if  (xObjectResolver.is())
                        *pArgs++ <<= xObjectResolver;

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
                            aDescriptor[0].Name = "FileName";
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
        OSL_FAIL( "XMLFilterTestDialog::doExport exception caught!" );
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
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
        FileDialogFlags::NONE, m_xDialog.get());
    OUString aFilterName( m_xFilterInfo->maInterfaceName );
    OUString aExtensions;

    int nLastIndex = 0;
    int nCurrentIndex = 0;
    for( int i = 0; nLastIndex != -1; i++ )
    {
        nLastIndex = m_xFilterInfo->maExtension.indexOf( ';', nLastIndex );

        if( i > 0 )
            aExtensions += ";";

        aExtensions += "*.";

        if( nLastIndex == -1 )
        {

            aExtensions += m_xFilterInfo->maExtension.copy( nCurrentIndex );
        }
        else
        {
            aExtensions += m_xFilterInfo->maExtension.copy( nCurrentIndex, nLastIndex - nCurrentIndex );
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

void XMLFilterTestDialog::import( const OUString& rURL )
{
    try
    {
        Reference< XDesktop2 > xLoader = Desktop::create( mxContext );
        Reference< XInteractionHandler2 > xInter = InteractionHandler::createWithParent(mxContext, nullptr);

        Sequence< PropertyValue > aArguments(2);
        aArguments[0].Name = "FilterName";
        aArguments[0].Value <<= m_xFilterInfo->maFilterName;
        aArguments[1].Name = "InteractionHandler";
        aArguments[1].Value <<= xInter;

        xLoader->loadComponentFromURL( rURL, "_default", 0, aArguments );

        if( m_xCBXDisplaySource->get_active() )
        {
            OUString const ext(".xml");
            TempFile aTempFile(OUString(), true, &ext);
            OUString aTempFileURL( aTempFile.GetURL() );

            Reference< XImportFilter > xImporter( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.documentconversion.XSLTFilter", mxContext ), UNO_QUERY );
            if( xImporter.is() )
            {
                osl::File aInputFile( rURL );
                aInputFile.open( osl_File_OpenFlag_Read );

                Reference< XInputStream > xIS( new comphelper::OSLInputStreamWrapper( aInputFile ) );

                Sequence< PropertyValue > aSourceData( 3 );
                int i = 0;

                aSourceData[i  ].Name = "InputStream";
                aSourceData[i++].Value <<= xIS;

                aSourceData[i  ].Name = "FileName";
                aSourceData[i++].Value <<= rURL;

                aSourceData[i  ].Name = "Indent";
                aSourceData[i++].Value <<= true;

                Reference< XWriter > xWriter = Writer::create( mxContext );

                File aOutputFile( aTempFileURL );
                aOutputFile.open( osl_File_OpenFlag_Write );

                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );
                xWriter->setOutputStream( xOS );

                Sequence< OUString > aFilterUserData( m_xFilterInfo->getFilterUserData() );
                xImporter->importer( aSourceData, xWriter, aFilterUserData );
            }

            displayXMLFile( aTempFileURL );
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("XMLFilterTestDialog::import caught an exception" );
    }
}

IMPL_LINK(XMLFilterTestDialog, ClickHdl_Impl, weld::Button&, rButton, void )
{
    if (m_xPBExportBrowse.get() == &rButton)
    {
        onExportBrowse();
    }
    else if (m_xPBCurrentDocument.get() == &rButton)
    {
        onExportCurrentDocument();
    }
    else if (m_xPBImportBrowse.get() == &rButton)
    {
        onImportBrowse();
    }
    else if (m_xPBRecentFile.get() == &rButton)
    {
        import( m_sImportRecentFile );
    }
    else if (m_xPBClose.get() == &rButton)
    {
        m_xDialog->response(RET_CLOSE);
    }
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
            xTest = xDesktop->getCurrentComponent();

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
        OSL_FAIL( "XMLFilterTestDialog::getFrontMostDocument exception caught!" );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
