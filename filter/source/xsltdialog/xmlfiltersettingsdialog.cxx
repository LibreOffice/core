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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <tools/urlobj.hxx>
#include <vcl/headbar.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/streamwrap.hxx>
#include <osl/file.hxx>
#include <o3tl/enumrange.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/treelistentry.hxx>
#include <tools/stream.hxx>

#include <rtl/uri.hxx>

#include <algorithm>
#include <memory>

#include <strings.hrc>
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfilterjar.hxx"
#include <strings.hxx>

using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

using ::rtl::Uri;

OUString XsltResId(const char* pId)
{
    return Translate::get(pId, Translate::Create("flt"));
}

XMLFilterSettingsDialog::XMLFilterSettingsDialog(vcl::Window* pParent,
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    Dialog::InitFlag eFlag)
    : ModelessDialog(pParent, "XMLFilterSettingsDialog", "filter/ui/xmlfiltersettings.ui", eFlag)
    , mxContext( rxContext )
    , m_sTemplatePath("$(user)/template/")
    , m_sDocTypePrefix("doctype:")
{
    get(m_pCtrlFilterList, "filterlist");
    get(m_pPBNew, "new");
    get(m_pPBEdit, "edit");
    get(m_pPBTest, "test");
    get(m_pPBDelete, "delete");
    get(m_pPBSave, "save");
    get(m_pPBOpen, "open");
    get(m_pPBClose, "close");

    m_pFilterListBox = m_pCtrlFilterList->getListBox();
    m_pFilterListBox->SetSelectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    m_pFilterListBox->SetDeselectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    m_pFilterListBox->SetDoubleClickHdl( LINK( this, XMLFilterSettingsDialog, DoubleClickHdl_Impl ) );
    m_pFilterListBox->SetAccessibleName(XsltResId(STR_XML_FILTER_LISTBOX));
    m_pFilterListBox->SetHelpId(m_pCtrlFilterList->GetHelpId());

    m_pPBNew->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBEdit->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBTest->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBDelete->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBSave->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBOpen->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    m_pPBClose->SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );

    try
    {
        mxFilterContainer.set( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", rxContext ), UNO_QUERY );
        mxTypeDetection.set( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.TypeDetection", rxContext ), UNO_QUERY );
        mxExtendedTypeDetection.set( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.ExtendedTypeDetectionFactory", rxContext ), UNO_QUERY );

        SvtPathOptions aOptions;
        m_sTemplatePath = aOptions.SubstituteVariable( m_sTemplatePath );
    }
    catch(const Exception&)
    {
        OSL_FAIL( "XMLFilterSettingsDialog::XMLFilterSettingsDialog exception caught!" );
    }
}

XMLFilterSettingsDialog::~XMLFilterSettingsDialog()
{
    disposeOnce();
}

void XMLFilterSettingsDialog::dispose()
{
    m_pFilterListBox.clear();
    m_pCtrlFilterList.clear();
    m_pPBNew.clear();
    m_pPBEdit.clear();
    m_pPBTest.clear();
    m_pPBDelete.clear();
    m_pPBSave.clear();
    m_pPBOpen.clear();
    m_pPBClose.clear();
    ModelessDialog::dispose();
}

IMPL_LINK(XMLFilterSettingsDialog, ClickHdl_Impl, Button *, pButton, void )
{
    // tdf#122171 block closing libreoffice until the following dialog is dismissed
    incBusy();

    if (m_pPBNew == pButton)
    {
        onNew();
    }
    else if (m_pPBEdit == pButton)
    {
        onEdit();
    }
    else if (m_pPBTest == pButton)
    {
        onTest();
    }
    else if (m_pPBDelete == pButton)
    {
        onDelete();
    }
    else if (m_pPBSave == pButton)
    {
        onSave();
    }
    else if (m_pPBOpen == pButton)
    {
        onOpen();
    }
    else if (m_pPBClose == pButton)
    {
        Close();
    }

    decBusy();
}

IMPL_LINK_NOARG(XMLFilterSettingsDialog, SelectionChangedHdl_Impl, SvTreeListBox*, void)
{
    updateStates();
}

IMPL_LINK_NOARG(XMLFilterSettingsDialog, DoubleClickHdl_Impl, SvTreeListBox*, bool)
{
    onEdit();
    return false;
}

void XMLFilterSettingsDialog::UpdateWindow()
{
    m_pCtrlFilterList->GrabFocus();
    disposeFilterList();
    m_pFilterListBox->Clear();
    initFilterList();
    updateStates();
}

void XMLFilterSettingsDialog::updateStates()
{
    SvTreeListEntry* pSelectedEntry = m_pFilterListBox->FirstSelected();

    bool bHasSelection = pSelectedEntry != nullptr;

    bool bMultiSelection = bHasSelection && (m_pFilterListBox->NextSelected( pSelectedEntry ) != nullptr );
    bool bIsReadonly = false;
    bool bIsDefault = false;
    if(pSelectedEntry)
    {
        filter_info_impl* pInfo = static_cast<filter_info_impl*>(pSelectedEntry->GetUserData());
        bIsReadonly = pInfo->mbReadonly;

        for( auto nFact : o3tl::enumrange<SvtModuleOptions::EFactory>())
        {
            OUString sDefault = maModuleOpt.GetFactoryDefaultFilter(nFact);
            if( sDefault == pInfo->maFilterName )
            {
                bIsDefault = true;
                break;
            }
        }
    }
    m_pPBEdit->Enable( bHasSelection && !bMultiSelection && !bIsReadonly);
    m_pPBTest->Enable( bHasSelection && !bMultiSelection );
    m_pPBDelete->Enable( bHasSelection && !bMultiSelection && !bIsReadonly && !bIsDefault);
    m_pPBSave->Enable( bHasSelection );
}

/** is called when the user clicks on the "New" button */
void XMLFilterSettingsDialog::onNew()
{
    filter_info_impl aTempInfo;

    // create a unique filter name
    aTempInfo.maFilterName = createUniqueFilterName(XsltResId(STR_DEFAULT_FILTER_NAME));

    // init default extension
    aTempInfo.maExtension = STR_DEFAULT_EXTENSION;

    // set default ui name
    aTempInfo.maInterfaceName = createUniqueInterfaceName(XsltResId(STR_DEFAULT_UI_NAME));

    // set default application
    aTempInfo.maDocumentService = "com.sun.star.text.TextDocument";

    // execute XML Filter Dialog
    XMLFilterTabDialog aDlg(GetFrameWeld(), mxContext, &aTempInfo);
    if (aDlg.run() == RET_OK)
    {
        // insert the new filter
        insertOrEdit( aDlg.getNewFilterInfo() );
    }
}

/** is called when the user clicks on the "Edit" Button */
void XMLFilterSettingsDialog::onEdit()
{
    // get selected filter entry
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        // get its filter info
        filter_info_impl* pOldInfo = static_cast<filter_info_impl*>(pEntry->GetUserData());

        // execute XML Filter Dialog
        XMLFilterTabDialog aDlg(GetFrameWeld(), mxContext, pOldInfo);
        if (aDlg.run() == RET_OK)
        {
            filter_info_impl* pNewInfo = aDlg.getNewFilterInfo();

            if( !(*pOldInfo == *pNewInfo) )
            {
                // change filter
                insertOrEdit( pNewInfo, pOldInfo );
            }
        }
    }
}

/** helper to create a sequence of strings from an extensions strings
    "ext1;ext2;ext3" will become { "ext1", "ext2", "ext3" } */
static Sequence< OUString > createExtensionsSequence( const OUString& rExtensions )
{
    // first count how many extensions we have inside the string
    int nExtensions = 0;

    int nLength = rExtensions.getLength();
    if( nLength )
    {
        // a non empty string has at least one extension
        nExtensions++;

        // now count the delimiters ';'
        const sal_Unicode * pString = rExtensions.getStr();
        int i;
        for( i = 0; i < nLength; i++, pString++ )
        {
            if( *pString == ';' )
                nExtensions++;
        }
    }

    Sequence< OUString > aExtensions( nExtensions );

    // extract the extensions from the source string and fill the sequence

    int nLastIndex = 0;
    int nCurrentIndex = 0;
    int i;

    for( i = 0; i < nExtensions; i++ )
    {
        nLastIndex = rExtensions.indexOf( ';', nLastIndex );

        if( nLastIndex == -1 )
        {
            aExtensions[i] = rExtensions.copy( nCurrentIndex );
            break;
        }
        else
        {
            aExtensions[i] = rExtensions.copy( nCurrentIndex, nLastIndex - nCurrentIndex );
            nCurrentIndex = nLastIndex + 1;
            nLastIndex = nCurrentIndex;
        }
    }

    return aExtensions;
}

/** checks if the given name is unique inside the filter factory. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueFilterName( const OUString& rFilterName )
{
    OUString aFilterName( rFilterName );

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aFilterName ) )
    {
        aFilterName = rFilterName + " " + OUString::number( nId++ );
    }

    return aFilterName;
}

/** checks if the given name is unique inside the type detection. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueTypeName( const OUString& rTypeName )
{
    OUString aTypeName( rTypeName );

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aTypeName ) )
    {
        aTypeName = rTypeName + " " + OUString::number( nId++ );
    }

    return aTypeName;
}

/** checks if the given name is a unique ui name inside the filter factory. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueInterfaceName( const OUString& rInterfaceName )
{
    sal_Int32 nDefaultNumber = 0;

    try
    {
        Sequence< OUString > aFilterNames( mxFilterContainer->getElementNames() );

        Sequence< PropertyValue > aValues;
        for( OUString const & filterName : aFilterNames)
        {
            Any aAny( mxFilterContainer->getByName( filterName ) );
            if( !(aAny >>= aValues) )
                continue;

            const sal_Int32 nValueCount( aValues.getLength() );
            PropertyValue* pValues = aValues.getArray();
            sal_Int32 nValue;

            for( nValue = 0; nValue < nValueCount; nValue++, pValues++ )
            {
                if ( pValues->Name == "UIName" )
                {
                    OUString aInterfaceName;
                    pValues->Value >>= aInterfaceName;


                    // see if this filter matches our default filter name
                    if( aInterfaceName.match( rInterfaceName ) )
                    {
                        // if yes, make sure we generate a unique name with a higher number
                        // this is dump but fast
                        sal_Int32 nNumber = aInterfaceName.copy( rInterfaceName.getLength() ).toInt32();
                        if( nNumber >= nDefaultNumber )
                            nDefaultNumber = nNumber + 1;
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterSettingsDialog::createUniqueInterfaceName exception caught!" );
    }

    OUString aInterfaceName( rInterfaceName );
    if( nDefaultNumber )
    {
        aInterfaceName += " " + OUString::number( nDefaultNumber );
    }

    return aInterfaceName;
}

/** inserts a new filter into the ui and configuration if pOldInfo is NULL.
    If pOldInfo is not null, the old filter will be replaced with the new settings */
bool XMLFilterSettingsDialog::insertOrEdit( filter_info_impl* pNewInfo, const filter_info_impl* pOldInfo )
{
    bool bOk = true;

    if( pOldInfo )
    {
        // see if we need to update the type name
        if( pOldInfo->maFilterName != pNewInfo->maFilterName )
        {
            if( pOldInfo->maType == pOldInfo->maFilterName )
            {
                pNewInfo->maType.clear();
            }
        }

        // see if we need to clean up old stuff first
        try
        {
            // if filter name changed, we need to remove the old filter first
            if( pOldInfo->maFilterName != pNewInfo->maFilterName )
                mxFilterContainer->removeByName( pOldInfo->maFilterName );

            // if type name changed, we need to remove the old type first
            if( pOldInfo->maType != pNewInfo->maType )
                mxTypeDetection->removeByName( pOldInfo->maType );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
            bOk = false;
        }
    }

    filter_info_impl* pFilterEntry( nullptr );

    if( bOk )
    {
        // create or copy filter info
        if( pOldInfo )
        {
            // change existing filter entry in filter list box
            pFilterEntry = const_cast<filter_info_impl*>(pOldInfo);
            *pFilterEntry = *pNewInfo;
        }
        else
        {
            // add new entry to filter list box
            pFilterEntry = new filter_info_impl( *pNewInfo );
        }
    }

    // check if we need to copy the template
    if( bOk && !pFilterEntry->maImportTemplate.isEmpty() )
    {
        if( !pFilterEntry->maImportTemplate.matchIgnoreAsciiCase( m_sTemplatePath ) )
        {
            INetURLObject aSourceURL( pFilterEntry->maImportTemplate );
            if( !aSourceURL.GetName().isEmpty() )
            {
                OUString aDestURL( m_sTemplatePath );
                aDestURL += pFilterEntry->maFilterName + "/";
                if( createDirectory( aDestURL ) )
                {
                    aDestURL += aSourceURL.GetName();

                    SvFileStream aInputStream(pFilterEntry->maImportTemplate, StreamMode::READ );
                    Reference< XInputStream > xIS( new utl::OInputStreamWrapper( aInputStream ) );
                    SvFileStream aOutputStream(aDestURL, StreamMode::WRITE );
                    Reference< XOutputStream > xOS(  new utl::OOutputStreamWrapper( aOutputStream ) );

                    if( copyStreams( xIS, xOS ) )
                        pFilterEntry->maImportTemplate = aDestURL;
                }
            }
        }
    }

    if( bOk )
    {
        if( pFilterEntry->maType.isEmpty() )
        {
            pFilterEntry->maType = createUniqueTypeName( pNewInfo->maFilterName );
        }

        // update import/export flags
        if( !pFilterEntry->maImportXSLT.isEmpty() )
        {
            pFilterEntry->maFlags |= 1;
        }
        else
        {
            pFilterEntry->maFlags &= ~1;
        }

        if( !pFilterEntry->maExportXSLT.isEmpty() )
        {
            pFilterEntry->maFlags |= 2;
        }
        else
        {
            pFilterEntry->maFlags &= ~2;
        }
        pFilterEntry->maFlags |= 0x80040;

        // 2. create user data for filter entry
        Sequence< OUString > aUserData( pFilterEntry->getFilterUserData());

        // 3. create property values for filter entry
        Sequence< PropertyValue > aFilterData( 8 );

        aFilterData[0].Name = "Type";
        aFilterData[0].Value <<= pFilterEntry->maType;

        aFilterData[1].Name = "UIName";
        aFilterData[1].Value <<= pFilterEntry->maInterfaceName;

        aFilterData[2].Name = "DocumentService";
        aFilterData[2].Value <<= pFilterEntry->maDocumentService;

        aFilterData[3].Name = "FilterService";
        aFilterData[3].Value <<= OUString( "com.sun.star.comp.Writer.XmlFilterAdaptor" );

        aFilterData[4].Name = "Flags";
        aFilterData[4].Value <<= pFilterEntry->maFlags;

        aFilterData[5].Name = "UserData";
        aFilterData[5].Value <<= aUserData;

        aFilterData[6].Name = "FileFormatVersion";
        aFilterData[6].Value <<= pFilterEntry->maFileFormatVersion;

        aFilterData[7].Name = "TemplateName";
        aFilterData[7].Value <<= pFilterEntry->maImportTemplate;

        // 4. insert new or replace existing filter
        try
        {
            Any aAny( makeAny( aFilterData ) );
            if( mxFilterContainer->hasByName( pFilterEntry->maFilterName ) )
            {
                mxFilterContainer->replaceByName( pFilterEntry->maFilterName, aAny );
            }
            else
            {
                mxFilterContainer->insertByName( pFilterEntry->maFilterName, aAny );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
            bOk = false;
        }
    }

    // 5. prepare type information
    if( bOk )
    {
        Sequence< PropertyValue > aValues(4);

        aValues[0].Name = "UIName";
        aValues[0].Value <<= pFilterEntry->maInterfaceName;
        aValues[1].Name = "ClipboardFormat";
        OUString aDocType;
        if( !pFilterEntry->maDocType.match( m_sDocTypePrefix ) )
        {
            aDocType = m_sDocTypePrefix + pFilterEntry->maDocType;
        }
        else
        {
            aDocType = pFilterEntry->maDocType;
        }
        if (aDocType == m_sDocTypePrefix)
            aValues[1].Value <<= OUString();
        else
            aValues[1].Value <<= aDocType;

        aValues[2].Name = "DocumentIconID";
        aValues[2].Value <<= pFilterEntry->mnDocumentIconID;

        aValues[3].Name = "Extensions";
        aValues[3].Value <<= createExtensionsSequence( pFilterEntry->maExtension );

        // the detect service will only be registered, if a doctype/search token was specified
        if (aDocType.getLength() > m_sDocTypePrefix.getLength())
        {
            aValues.realloc(5);
            aValues[4].Name = "DetectService";
            aValues[4].Value <<= OUString( "com.sun.star.comp.filters.XMLFilterDetect" );
        }

        // 6. insert new or replace existing type information
        if( mxTypeDetection.is() )
        {
            try
            {
                Any aAny( makeAny( aValues ) );
                if( mxTypeDetection->hasByName( pFilterEntry->maType ) )
                {
                    mxTypeDetection->replaceByName( pFilterEntry->maType, aAny );
                }
                else
                {
                    mxTypeDetection->insertByName( pFilterEntry->maType, aAny );
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
                bOk = false;
            }
        }

        if( bOk )
        {
            try
            {
                Reference< XFlushable > xFlushable( mxTypeDetection, UNO_QUERY );
                if( xFlushable.is() )
                    xFlushable->flush();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
                bOk = false;
            }
        }

        if( !bOk )
        {
            // we failed to add the type, so lets remove the filter
            try
            {
                mxFilterContainer->removeByName( pFilterEntry->maFilterName );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
                bOk = false;
            }
        }
        else // bOk
        {
            try
            {
                Reference< XFlushable > xFlushable( mxFilterContainer, UNO_QUERY );
                if( xFlushable.is() )
                    xFlushable->flush();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
                bOk = false;
            }

            if( !bOk )
            {
                // we failed to add the filter, so lets remove the type
                try
                {
                    mxTypeDetection->removeByName( pFilterEntry->maType );
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception caught!" );
                }

            }
        }
    }

    if( bOk )
    {
        if( mxExtendedTypeDetection.is() )
        {
            OUString sFilterDetectService( "com.sun.star.comp.filters.XMLFilterDetect" );
            if( mxExtendedTypeDetection->hasByName( sFilterDetectService ) )
            {
                Sequence< PropertyValue > aSequence;
                if( mxExtendedTypeDetection->getByName( sFilterDetectService ) >>= aSequence )
                {
                    sal_Int32 nCount = aSequence.getLength();
                    sal_Int32 nIndex;
                    for( nIndex = 0; nIndex < nCount; nIndex++ )
                    {
                        if ( aSequence[nIndex].Name == "Types" )
                        {
                            Sequence< OUString > aTypes;
                            if( aSequence[nIndex].Value >>= aTypes )
                            {
                                sal_Int32 nStrCount = aTypes.getLength();
                                sal_Int32 nStr;
                                for( nStr = 0; nStr < nStrCount; nStr++ )
                                {
                                    if( aTypes[nStr] == pFilterEntry->maType )
                                        break;
                                }

                                if( nStr == nStrCount )
                                {
                                    aTypes.realloc( nStrCount + 1 );
                                    aTypes[nStrCount] = pFilterEntry->maType;

                                    aSequence[nIndex].Value <<= aTypes;

                                    mxExtendedTypeDetection->replaceByName( sFilterDetectService, makeAny( aSequence ) );

                                    Reference< XFlushable > xFlushable( mxExtendedTypeDetection, UNO_QUERY );
                                    if( xFlushable.is() )
                                        xFlushable->flush();
                                }
                            }

                            break;
                        }
                    }
                }
            }
        }
    }

    // update ui
    if( bOk )
    {
        if( pOldInfo )
        {
            m_pFilterListBox->changeEntry( pFilterEntry );
        }
        else
        {
            m_pFilterListBox->addFilterEntry( pFilterEntry );
            maFilterVector.push_back( std::unique_ptr<filter_info_impl>(pFilterEntry) );
        }
    }

    return bOk;
}

/** is called when the user clicks the "Test" button */
void XMLFilterSettingsDialog::onTest()
{
    // get the first selected filter
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = static_cast<filter_info_impl*>(pEntry->GetUserData());

        XMLFilterTestDialog aDlg(GetFrameWeld(), mxContext);
        aDlg.test( *pInfo );
    }
}

void XMLFilterSettingsDialog::onDelete()
{
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = static_cast<filter_info_impl*>(pEntry->GetUserData());

        OUString aMessage(XsltResId(STR_WARN_DELETE));
        aMessage = aMessage.replaceFirst( "%s", pInfo->maFilterName );

        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                   aMessage));
        xWarn->set_default_response(RET_YES);
        if (xWarn->run() == RET_YES)
        {
            try
            {
                if( mxFilterContainer->hasByName( pInfo->maFilterName ) )
                {
                    mxFilterContainer->removeByName( pInfo->maFilterName );

                    bool bTypeStillUsed = false;

                    // now loop over all filter and see if someone else uses the same type
                    Sequence< OUString > aFilterNames( mxFilterContainer->getElementNames() );
                    OUString* pFilterName = aFilterNames.getArray();

                    const sal_Int32 nCount = aFilterNames.getLength();
                    sal_Int32 nFilter;
                    Sequence< PropertyValue > aValues;

                    for( nFilter = 0; (nFilter < nCount) && !bTypeStillUsed; nFilter++, pFilterName++ )
                    {
                        Any aAny( mxFilterContainer->getByName( *pFilterName ) );
                        if( !(aAny >>= aValues) )
                            continue;

                        const sal_Int32 nValueCount( aValues.getLength() );
                        PropertyValue* pValues = aValues.getArray();
                        sal_Int32 nValue;

                        for (nValue = 0; nValue < nValueCount; nValue++, pValues++)
                        {
                            if ( pValues->Name == "Type" )
                            {
                                OUString aType;
                                pValues->Value >>= aType;
                                if( aType == pInfo->maType )
                                    bTypeStillUsed = true;

                                break;
                            }
                        }
                    }

                    // if the type is not used anymore, remove it also
                    if( !bTypeStillUsed )
                    {
                        if( mxTypeDetection->hasByName( pInfo->maType ) )
                        {
                            mxTypeDetection->removeByName( pInfo->maType );
                        }
                    }

                    Reference< XFlushable > xFlushable( mxFilterContainer, UNO_QUERY );
                    if( xFlushable.is() )
                        xFlushable->flush();

                    xFlushable.set( mxTypeDetection, UNO_QUERY );
                    if( xFlushable.is() )
                        xFlushable->flush();

                    // now remove entry from ui
                    m_pFilterListBox->RemoveSelection();

                    // and delete the filter entry
                    maFilterVector.erase(std::find_if( maFilterVector.begin(), maFilterVector.end(),
                                            [&] (std::unique_ptr<filter_info_impl> const & p)
                                            { return p.get() == pInfo; }));
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::onDelete exception caught!" );
            }
        }
    }

    updateStates();
}

void XMLFilterSettingsDialog::onSave()
{
    std::vector<filter_info_impl*> aFilters;

    int nFilters = 0;

    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    while( pEntry )
    {
        filter_info_impl* pInfo = static_cast<filter_info_impl*>(pEntry->GetUserData());
        aFilters.push_back( pInfo );
        pEntry = m_pFilterListBox->NextSelected( pEntry );
        nFilters++;
    }

    // Open Fileopen-Dialog
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
        FileDialogFlags::NONE, GetFrameWeld());

    OUString aExtensions( "*.jar" );
    OUString aFilterName(XsltResId(STR_FILTER_PACKAGE));
    aFilterName += " (" + aExtensions + ")";

    aDlg.AddFilter( aFilterName, aExtensions );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        XMLFilterJarHelper aJarHelper( mxContext );
        aJarHelper.savePackage( aDlg.GetPath(), aFilters );

        INetURLObject aURL( aDlg.GetPath() );

        OUString sPlaceholder( "%s" );

        OUString aMsg;
        if( nFilters > 0 )
        {
            aMsg = XsltResId(STR_FILTERS_HAVE_BEEN_SAVED);
            aMsg = aMsg.replaceFirst( sPlaceholder, OUString::number( nFilters ) );
            aMsg = aMsg.replaceFirst( sPlaceholder, aURL.GetName() );
        }
        else
        {
            aMsg = XsltResId(STR_FILTER_HAS_BEEN_SAVED);
            aMsg = aMsg.replaceFirst( sPlaceholder, (*aFilters.begin())->maFilterName );
            aMsg = aMsg.replaceFirst( sPlaceholder, aURL.GetName() );
        }

        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      aMsg));
        xInfoBox->run();
    }
}

void XMLFilterSettingsDialog::onOpen()
{
    std::vector< std::unique_ptr<filter_info_impl> > aFilters;

    // Open Fileopen-Dialog
    ::sfx2::FileDialogHelper aDlg(
    css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
    FileDialogFlags::NONE, GetFrameWeld());

    OUString aExtensions( "*.jar" );
    OUString aFilterName(XsltResId(STR_FILTER_PACKAGE));
    aFilterName += " (" + aExtensions + ")";

    aDlg.AddFilter( aFilterName, aExtensions );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString aURL( aDlg.GetPath() );

        XMLFilterJarHelper aJarHelper( mxContext );
        aJarHelper.openPackage( aURL, aFilters );

        int nFilters = 0;
        for (auto& filter : aFilters)
        {
            if( insertOrEdit(filter.get()) )
            {
                aFilterName = filter->maFilterName;
                nFilters++;
            }

            filter.reset();
        }

        disposeFilterList();
        initFilterList();

        OUString sPlaceholder( "%s" );
        OUString aMsg;
        if( nFilters == 0 )
        {
            INetURLObject aURLObj( aURL );
            aMsg = XsltResId(STR_NO_FILTERS_FOUND);
            aMsg = aMsg.replaceFirst( sPlaceholder, aURLObj.GetName() );
        }
        else if( nFilters == 1 )
        {
            aMsg = XsltResId(STR_FILTER_INSTALLED);
            aMsg = aMsg.replaceFirst( sPlaceholder, aFilterName );

        }
        else
        {
            aMsg = XsltResId(STR_FILTERS_INSTALLED);
            aMsg = aMsg.replaceFirst( sPlaceholder, OUString::number( nFilters ) );
        }

        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      aMsg));
        xInfoBox->run();
    }
}

bool XMLFilterSettingsDialog::EventNotify( NotifyEvent& rNEvt )
{
    // Because of tab control first call the base class.
    bool bRet = ModelessDialog::EventNotify(rNEvt);
    if ( !bRet )
    {
        if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16 nKeyCode = aKeyCode.GetCode();
            bool bMod1 = pKEvt->GetKeyCode().IsMod1();

            if( nKeyCode == KEY_ESCAPE || (bMod1 && (nKeyCode == KEY_W)))
            {
                Close();
                return true;
            }
        }
    }

    return bRet;
}

void XMLFilterSettingsDialog::disposeFilterList()
{
    maFilterVector.clear();
    m_pFilterListBox->Clear();
}

void XMLFilterSettingsDialog::initFilterList()
{
    if( mxFilterContainer.is() )
    {
        Sequence< OUString > aFilterNames( mxFilterContainer->getElementNames() );

        Sequence< PropertyValue > aValues;

        std::unique_ptr<filter_info_impl> pTempFilter( new filter_info_impl );
        Sequence< OUString > aUserData;

        for( OUString const & filterName : aFilterNames )
        {
            aUserData.realloc(0);

            try
            {
                Any aAny( mxFilterContainer->getByName( filterName ) );
                if( !(aAny >>= aValues) )
                    continue;

                OUString aFilterService;
                pTempFilter->maFilterName = filterName;

                const sal_Int32 nValueCount( aValues.getLength() );
                PropertyValue* pValues = aValues.getArray();
                sal_Int32 nValue;

                for( nValue = 0; nValue < nValueCount; nValue++, pValues++ )
                {
                    if ( pValues->Name == "Type" )
                    {
                        pValues->Value >>= pTempFilter->maType;
                    }
                    else if ( pValues->Name == "UIName" )
                    {
                        pValues->Value >>= pTempFilter->maInterfaceName;
                    }
                    else if ( pValues->Name == "DocumentService" )
                    {
                        pValues->Value >>= pTempFilter->maDocumentService;
                    }
                    else if ( pValues->Name == "FilterService" )
                    {
                        pValues->Value >>= aFilterService;
                    }
                    else if ( pValues->Name == "Flags" )
                    {
                        pValues->Value >>= pTempFilter->maFlags;
                    }
                    else if ( pValues->Name == "UserData" )
                    {
                        pValues->Value >>= aUserData;
                    }
                    else if ( pValues->Name == "FileFormatVersion" )
                    {
                        pValues->Value >>= pTempFilter->maFileFormatVersion;
                    }
                    else if ( pValues->Name == "TemplateName" )
                    {
                        pValues->Value >>= pTempFilter->maImportTemplate;
                    }
                    else if ( pValues->Name == "Finalized" )
                    {
                        pValues->Value >>= pTempFilter->mbReadonly;
                    }
                }

                // if this is not a XmlFilterAdaptor entry, skip it
                if( aFilterService != "com.sun.star.comp.Writer.XmlFilterAdaptor" )
                    continue;


                // if we don't have the needed user data, skip it
                if( aUserData.getLength() < 6 )
                    continue;

                // if this is not an XSLTFilter entry, skip it
                if( aUserData[0] != "com.sun.star.documentconversion.XSLTFilter" )
                    continue;

                // get filter information from userdata
                pTempFilter->mbNeedsXSLT2 = aUserData[1].toBoolean();
                pTempFilter->maImportService = aUserData[2];
                pTempFilter->maExportService = aUserData[3];
                pTempFilter->maImportXSLT = aUserData[4];
                pTempFilter->maExportXSLT = aUserData[5];
                if( aUserData.getLength() >= 8 )
                    pTempFilter->maComment = aUserData[7];

                // get type information
                if( mxTypeDetection.is() )
                {
                    try
                    {
                        aAny = mxTypeDetection->getByName( pTempFilter->maType );
                        Sequence< PropertyValue > aValues2;

                        if( aAny >>= aValues2 )
                        {
                            const sal_Int32 nValueCount2( aValues2.getLength() );
                            PropertyValue* pValues2 = aValues2.getArray();
                            sal_Int32 nValue2;

                            for( nValue2 = 0; nValue2 < nValueCount2; nValue2++, pValues2++ )
                            {
                                if ( pValues2->Name == "ClipboardFormat" )
                                {
                                    OUString aDocType;
                                    pValues2->Value >>= aDocType;

                                    if( aDocType.match( m_sDocTypePrefix ) )
                                        aDocType = aDocType.copy( m_sDocTypePrefix.getLength() );

                                    pTempFilter->maDocType = aDocType;
                                }
                                else if ( pValues2->Name == "Extensions" )
                                {
                                    Sequence< OUString > aExtensions;
                                    if( pValues2->Value >>= aExtensions )
                                    {
                                        pTempFilter->maExtension.clear();

                                        sal_Int32 nCount3( aExtensions.getLength() );
                                        OUString* pExtensions = aExtensions.getArray();
                                        sal_Int32 n;
                                        for( n = 0; n < nCount3; n++ )
                                        {
                                            if( n > 0 )
                                                pTempFilter->maExtension += ";";
                                            pTempFilter->maExtension += *pExtensions++;
                                        }
                                    }
                                }
                                else if ( pValues2->Name == "DocumentIconID" )
                                {
                                    pValues2->Value >>= pTempFilter->mnDocumentIconID;
                                }
                                else if ( pValues2->Name == "Finalized" )
                                {
                                    // both the filter and the type may be finalized
                                    bool bTemp = false;
                                    pValues2->Value >>= bTemp;
                                    pTempFilter->mbReadonly |= bTemp;
                                }
                            }
                        }
                    }
                    catch( const css::container::NoSuchElementException& )
                    {
                        OSL_FAIL( "Type not found, user error?" ); // TODO: error?
                    }
                }

                // add entry to internal container and to ui filter list box
                maFilterVector.push_back( std::unique_ptr<filter_info_impl>(pTempFilter.get()) );
                m_pFilterListBox->addFilterEntry( pTempFilter.release() );


                pTempFilter.reset( new filter_info_impl );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::initFilterList exception caught!" );
            }

        }
    }

    SvTreeListEntry* pEntry = m_pFilterListBox->GetEntry( 0 );
    if( pEntry )
        m_pFilterListBox->Select( pEntry );
}

application_info_impl::application_info_impl( const sal_Char * pDocumentService, const OUString& rUINameRes, const sal_Char * mpXMLImporter, const sal_Char * mpXMLExporter )
:   maDocumentService( pDocumentService, strlen( pDocumentService ), RTL_TEXTENCODING_ASCII_US ),
    maDocumentUIName(Translate::ExpandVariables(rUINameRes)),
    maXMLImporter( mpXMLImporter, strlen( mpXMLImporter ), RTL_TEXTENCODING_ASCII_US ),
    maXMLExporter( mpXMLExporter, strlen( mpXMLExporter ), RTL_TEXTENCODING_ASCII_US )
{
}

std::vector< application_info_impl > const & getApplicationInfos()
{
    static std::vector< application_info_impl > const aInfos
    {
        {   "com.sun.star.text.TextDocument",
            STR_APPL_NAME_WRITER,
            "com.sun.star.comp.Writer.XMLImporter",
            "com.sun.star.comp.Writer.XMLExporter" },

        {   "com.sun.star.sheet.SpreadsheetDocument",
            STR_APPL_NAME_CALC,
            "com.sun.star.comp.Calc.XMLImporter",
            "com.sun.star.comp.Calc.XMLExporter" },

        {  "com.sun.star.presentation.PresentationDocument",
            STR_APPL_NAME_IMPRESS,
            "com.sun.star.comp.Impress.XMLImporter",
            "com.sun.star.comp.Impress.XMLExporter" },

        {   "com.sun.star.drawing.DrawingDocument",
            STR_APPL_NAME_DRAW,
            "com.sun.star.comp.Draw.XMLImporter",
            "com.sun.star.comp.Draw.XMLExporter" },

        // --- oasis file formats...
        {   "com.sun.star.text.TextDocument",
            STR_APPL_NAME_OASIS_WRITER,
            "com.sun.star.comp.Writer.XMLOasisImporter",
            "com.sun.star.comp.Writer.XMLOasisExporter" },

        {   "com.sun.star.sheet.SpreadsheetDocument",
            STR_APPL_NAME_OASIS_CALC,
            "com.sun.star.comp.Calc.XMLOasisImporter",
            "com.sun.star.comp.Calc.XMLOasisExporter" },

        {   "com.sun.star.presentation.PresentationDocument",
            STR_APPL_NAME_OASIS_IMPRESS,
            "com.sun.star.comp.Impress.XMLOasisImporter",
            "com.sun.star.comp.Impress.XMLOasisExporter" },

        {  "com.sun.star.drawing.DrawingDocument",
            STR_APPL_NAME_OASIS_DRAW,
            "com.sun.star.comp.Draw.XMLOasisImporter",
            "com.sun.star.comp.Draw.XMLOasisExporter" },
    };

    return aInfos;
}

const application_info_impl* getApplicationInfo( const OUString& rServiceName )
{
    std::vector< application_info_impl > const & rInfos = getApplicationInfos();
    for (auto const& info : rInfos)
    {
        if( rServiceName == info.maXMLExporter ||
            rServiceName == info.maXMLImporter)
        {
            return &info;
        }
    }
    return nullptr;
}

OUString getApplicationUIName( const OUString& rServiceName )
{
    const application_info_impl* pInfo = getApplicationInfo( rServiceName );
    if( pInfo )
    {
        return pInfo->maDocumentUIName;
    }
    else
    {
        OUString aRet = XsltResId(STR_UNKNOWN_APPLICATION);
        if( !rServiceName.isEmpty() )
        {
            aRet += " (" + rServiceName + ")";
        }
        return aRet;
    }
}

SvxPathControl::SvxPathControl(vcl::Window* pParent)
    : Window(pParent, WB_HIDE | WB_CLIPCHILDREN | WB_TABSTOP | WB_DIALOGCONTROL | WB_BORDER)
    , bHasBeenShown(false)
{
    m_pVBox = VclPtr<VclVBox>::Create(this);

    m_pHeaderBar = VclPtr<HeaderBar>::Create(m_pVBox, WB_BOTTOMBORDER);
    m_pHeaderBar->set_height_request(GetTextHeight() + 6);

    m_pFocusCtrl = VclPtr<XMLFilterListBox>::Create(m_pVBox, this);
    m_pFocusCtrl->set_fill(true);
    m_pFocusCtrl->set_expand(true);

    m_pVBox->set_hexpand(true);
    m_pVBox->set_vexpand(true);
    m_pVBox->set_expand(true);
    m_pVBox->set_fill(true);
    m_pVBox->Show();
}

#define ITEMID_NAME     1
#define ITEMID_TYPE     2

void SvxPathControl::Resize()
{
    Window::Resize();

    if (!m_pVBox)
        return;

    m_pVBox->SetSizePixel(GetSizePixel());

    if (!bHasBeenShown)
        bHasBeenShown = IsReallyShown();

    if (!bHasBeenShown)
    {
        std::vector<long> aWidths;
        m_pFocusCtrl->getPreferredDimensions(aWidths);
        if (aWidths.empty())
        {
            bHasBeenShown = false;
            return;
        }
        long nFirstColumnWidth = aWidths[1];
        m_pHeaderBar->SetItemSize(ITEMID_NAME, nFirstColumnWidth);
        m_pHeaderBar->SetItemSize(ITEMID_TYPE, 0xFFFF);
        long nTabs[] = {0, nFirstColumnWidth};
        m_pFocusCtrl->SetTabs(SAL_N_ELEMENTS(nTabs), nTabs, MapUnit::MapPixel);
    }
}

Size SvxPathControl::GetOptimalSize() const
{
    Size aDefSize(LogicToPixel(Size(150, 0), MapMode(MapUnit::MapAppFont)));
    Size aOptSize(m_pVBox->GetOptimalSize());
    long nRowHeight(GetTextHeight());
    aOptSize.setHeight( nRowHeight * 10 );
    aOptSize.setWidth( std::max(aDefSize.Width(), aOptSize.Width()) );
    return aOptSize;
}

SvxPathControl::~SvxPathControl()
{
    disposeOnce();
}

void SvxPathControl::dispose()
{
    m_pFocusCtrl.disposeAndClear();
    m_pHeaderBar.disposeAndClear();
    m_pVBox.disposeAndClear();
    vcl::Window::dispose();
}

VCL_BUILDER_FACTORY(SvxPathControl)

bool SvxPathControl::EventNotify(NotifyEvent& rNEvt)
{
    bool bRet = Window::EventNotify(rNEvt);

    if ( m_pFocusCtrl && rNEvt.GetWindow() != m_pFocusCtrl && rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        m_pFocusCtrl->GrabFocus();

    return bRet;
}

XMLFilterListBox::XMLFilterListBox(Window* pParent, SvxPathControl* pPathControl)
    : SvTabListBox(pParent, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP)
    , m_pHeaderBar(pPathControl->getHeaderBar())
{
    Size aBoxSize( pParent->GetOutputSizePixel() );

    m_pHeaderBar->SetEndDragHdl( LINK( this, XMLFilterListBox, HeaderEndDrag_Impl ) );

    OUString aStr1(XsltResId(STR_COLUMN_HEADER_NAME));
    OUString aStr2(XsltResId(STR_COLUMN_HEADER_TYPE));

    long nTabSize = aBoxSize.Width() / 2;

    m_pHeaderBar->InsertItem( ITEMID_NAME, aStr1, nTabSize, HeaderBarItemBits::LEFT );
    m_pHeaderBar->InsertItem( ITEMID_TYPE, aStr2, nTabSize, HeaderBarItemBits::LEFT );

    static long nTabs[] = {0, nTabSize };

    SetSelectionMode( SelectionMode::Multiple );
    SetTabs( SAL_N_ELEMENTS(nTabs), nTabs, MapUnit::MapPixel );
    SetScrolledHdl( LINK( this, XMLFilterListBox, TabBoxScrollHdl_Impl ) );
    SetHighlightRange();
    Show();
    m_pHeaderBar->Show();
}

XMLFilterListBox::~XMLFilterListBox()
{
    disposeOnce();
}

void XMLFilterListBox::dispose()
{
    m_pHeaderBar.clear();
    SvTabListBox::dispose();
}

IMPL_LINK_NOARG( XMLFilterListBox, TabBoxScrollHdl_Impl, SvTreeListBox*, void )
{
    m_pHeaderBar->SetOffset( -GetXOffset() );
}

IMPL_LINK( XMLFilterListBox, HeaderEndDrag_Impl, HeaderBar*, pBar, void )
{
    if ( pBar && !pBar->GetCurItemId() )
        return;

    if ( !m_pHeaderBar->IsItemMode() )
    {
        Size aSz;
        sal_uInt16 nTabs = m_pHeaderBar->GetItemCount();
        long nTmpSz = 0;
        long nWidth = m_pHeaderBar->GetItemSize(ITEMID_NAME);
        long nBarWidth = m_pHeaderBar->GetSizePixel().Width();

        if(nWidth < 30)
            m_pHeaderBar->SetItemSize( ITEMID_TYPE, 30);
        else if ( ( nBarWidth - nWidth ) < 30 )
            m_pHeaderBar->SetItemSize( ITEMID_TYPE, nBarWidth - 30 );

        for ( sal_uInt16 i = 1; i <= nTabs; ++i )
        {
            long nW = m_pHeaderBar->GetItemSize(i);
            aSz.setWidth(  nW + nTmpSz );
            nTmpSz += nW;
            SetTab( i, PixelToLogic( aSz, MapMode(MapUnit::MapAppFont) ).Width() );
        }
    }
}

/** adds a new filter info entry to the ui filter list */
void XMLFilterListBox::addFilterEntry( const filter_info_impl* pInfo )
{
    const OUString aEntryStr( getEntryString( pInfo ) );
    InsertEntryToColumn( aEntryStr, TREELIST_APPEND, 0xffff, const_cast<filter_info_impl *>(pInfo) );
}

void XMLFilterListBox::changeEntry( const filter_info_impl* pInfo )
{
    const sal_uLong nCount = GetEntryCount();
    sal_uLong nPos;
    for( nPos = 0; nPos < nCount; nPos++ )
    {
        SvTreeListEntry* pEntry = GetEntry( nPos );
        if( static_cast<filter_info_impl*>(pEntry->GetUserData()) == pInfo )
        {
            OUString aEntryText( getEntryString( pInfo ) );
            SetEntryText( aEntryText, pEntry );
            break;
        }
    }
}

OUString XMLFilterListBox::getEntryString( const filter_info_impl* pInfo )
{
    OUString aEntryStr( pInfo->maFilterName + "\t");
    if ( !pInfo->maExportService.isEmpty() )
        aEntryStr += getApplicationUIName( pInfo->maExportService );
    else
        aEntryStr += getApplicationUIName( pInfo->maImportService );
    aEntryStr += " - ";

    if( pInfo->maFlags & 1 )
    {
        if( pInfo->maFlags & 2 )
        {
            aEntryStr += XsltResId(STR_IMPORT_EXPORT);
        }
        else
        {
            aEntryStr += XsltResId(STR_IMPORT_ONLY);
        }
    }
    else if( pInfo->maFlags & 2 )
    {
        aEntryStr += XsltResId(STR_EXPORT_ONLY);
    }
    else
    {
        aEntryStr += XsltResId(STR_UNDEFINED_FILTER);
    }

    return aEntryStr;
}

filter_info_impl::filter_info_impl()
    : maFlags(0x00080040)
    , maFileFormatVersion(0)
    , mnDocumentIconID(0)
    , mbReadonly(false)
    , mbNeedsXSLT2(false)
{
}

bool filter_info_impl::operator==( const filter_info_impl& r ) const
{
    return maFilterName == r.maFilterName &&
        maType == r.maType &&
        maDocumentService == r.maDocumentService &&
        maInterfaceName == r.maInterfaceName &&
        maComment == r.maComment &&
        maExtension == r.maExtension &&
        maDocType == r.maDocType &&
        maExportXSLT == r.maExportXSLT &&
        maImportXSLT == r.maImportXSLT &&
        maExportService == r.maExportService &&
        maImportService == r.maImportService &&
        maImportTemplate == r.maImportTemplate &&
        maFlags == r.maFlags &&
        maFileFormatVersion == r.maFileFormatVersion &&
        mbNeedsXSLT2 == r.mbNeedsXSLT2;
}


Sequence< OUString > filter_info_impl::getFilterUserData() const
{
    Sequence< OUString > aUserData(8);

    aUserData[0] = "com.sun.star.documentconversion.XSLTFilter";
    aUserData[1] = OUString::boolean( mbNeedsXSLT2 );
    aUserData[2] = maImportService;
    aUserData[3] = maExportService;
    aUserData[4] = maImportXSLT;
    aUserData[5] = maExportXSLT;
    aUserData[7] = maComment;

    return aUserData;
}

OUString string_encode( const OUString & rText )
{
    static sal_Bool const uricNoSlash[] = {
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false,  true, false, false,  true, false,  true,  true,  //  !"#$%&'
         true,  true,  true,  true, false,  true,  true, false,  // ()*+,-./
         true,  true,  true,  true,  true,  true,  true,  true,  // 01234567
         true,  true,  true, false, false,  true, false,  true,  // 89:;<=>?
         true,  true,  true,  true,  true,  true,  true,  true,  // @ABCDEFG
         true,  true,  true,  true,  true,  true,  true,  true,  // HIJKLMNO
         true,  true,  true,  true,  true,  true,  true,  true,  // PQRSTUVW
         true,  true,  true, false, false, false, false,  true,  // XYZ[\]^_
        false,  true,  true,  true,  true,  true,  true,  true,  // `abcdefg
         true,  true,  true,  true,  true,  true,  true,  true,  // hijklmno
         true,  true,  true,  true,  true,  true,  true,  true,  // pqrstuvw
         true,  true,  true, false, false, false,  true, false}; // xyz{|}~


    return Uri::encode( rText, uricNoSlash, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

OUString string_decode( const OUString & rText )
{
    return Uri::decode( rText, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
}

bool copyStreams( const Reference< XInputStream >& xIS, const Reference< XOutputStream >& xOS )
{
    try
    {
        sal_Int32 nBufferSize = 512;
        Sequence< sal_Int8 > aDataBuffer(nBufferSize);

        sal_Int32 nRead;
        do
        {
            nRead = xIS->readBytes( aDataBuffer, nBufferSize );

            if( nRead )
            {
                if( nRead < nBufferSize )
                {
                    nBufferSize = nRead;
                    aDataBuffer.realloc(nRead);
                }

                xOS->writeBytes( aDataBuffer );
            }
        }
        while( nRead );

        xOS->flush();

        return true;
    }
    catch(const Exception&)
    {
        OSL_FAIL( "copyStreams() exception caught!" );
    }

    return false;
}

bool createDirectory( OUString const & rURL )
{
    sal_Int32 nLastIndex = sizeof( "file:///" ) - 2;
    while( nLastIndex != -1 )
    {
        nLastIndex = rURL.indexOf( '/', nLastIndex + 1);
        if( nLastIndex != -1 )
        {
            OUString aDirURL( rURL.copy( 0, nLastIndex ) );
            Directory aDir( aDirURL );
            Directory::RC rc = aDir.open();
            if( rc == Directory::E_NOENT )
                rc = osl::Directory::create( aDirURL );

            if( rc != Directory::E_None )
            {
                return false;
            }
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
