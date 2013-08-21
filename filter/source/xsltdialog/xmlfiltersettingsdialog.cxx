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

#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <tools/urlobj.hxx>
#include <svtools/headbar.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/pathoptions.hxx>
#include <osl/file.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include "svtools/treelistentry.hxx"

#include <rtl/uri.hxx>

#include <algorithm>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfilterjar.hxx"

using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

using ::rtl::Uri;

XMLFilterSettingsDialog::XMLFilterSettingsDialog(Window* pParent,
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext)
    : ModelessDialog(pParent, "XMLFilterSettingsDialog", "filter/ui/xmlfiltersettings.ui")
    , mxContext( rxContext )
    , m_bIsClosable(true)
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
    m_pFilterListBox->SetAccessibleName(RESIDSTR(STR_XML_FILTER_LISTBOX));
    m_pCtrlFilterList->SetAccessibleName(RESIDSTR(STR_XML_FILTER_LISTBOX));
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
        mxFilterContainer = Reference< XNameContainer >::query( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", rxContext ) );
        mxTypeDetection = Reference< XNameContainer >::query( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.TypeDetection", rxContext ) );
        mxExtendedTypeDetection = Reference< XNameContainer >::query( rxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.ExtendedTypeDetectionFactory", rxContext ) );

        SvtPathOptions aOptions;
        m_sTemplatePath = aOptions.SubstituteVariable( m_sTemplatePath );
    }
    catch(const Exception&)
    {
        OSL_FAIL( "XMLFilterSettingsDialog::XMLFilterSettingsDialog exception catched!" );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK(XMLFilterSettingsDialog, ClickHdl_Impl, PushButton *, pButton )
{
    m_bIsClosable = false;

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
        onClose();
    }

    m_bIsClosable = true;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(XMLFilterSettingsDialog, SelectionChangedHdl_Impl)
{
    updateStates();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(XMLFilterSettingsDialog, DoubleClickHdl_Impl)
{
    onEdit();
    return 0;
}

bool XMLFilterSettingsDialog::isClosable()
{
    return m_bIsClosable;
}

// -----------------------------------------------------------------------

short XMLFilterSettingsDialog::Execute()
{
    m_pCtrlFilterList->GrabFocus();
    disposeFilterList();
    m_pFilterListBox->Clear();
    initFilterList();
    updateStates();

    return ModelessDialog::Execute();
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::updateStates()
{
    SvTreeListEntry* pSelectedEntry = m_pFilterListBox->FirstSelected();

    bool bHasSelection = pSelectedEntry != NULL;

    bool bMultiSelection = bHasSelection && (m_pFilterListBox->NextSelected( pSelectedEntry ) != NULL );
    bool bIsReadonly = false;
    bool bIsDefault = false;
    if(pSelectedEntry)
    {
        filter_info_impl* pInfo = (filter_info_impl*)pSelectedEntry->GetUserData();
        bIsReadonly = 0 != pInfo->mbReadonly;

        sal_Int32 nFact = SvtModuleOptions::E_WRITER;
        while(nFact <= SvtModuleOptions::E_BASIC)
        {
            OUString sDefault = maModuleOpt.GetFactoryDefaultFilter((SvtModuleOptions::EFactory)nFact);
            if( sDefault == pInfo->maFilterName )
            {
                bIsDefault = true;
                break;
            }
            ++nFact;
        }
    }
    m_pPBEdit->Enable( bHasSelection && !bMultiSelection && !bIsReadonly);
    m_pPBTest->Enable( bHasSelection && !bMultiSelection );
    m_pPBDelete->Enable( bHasSelection && !bMultiSelection && !bIsReadonly && !bIsDefault);
    m_pPBSave->Enable( bHasSelection );
}

// -----------------------------------------------------------------------

/** is called when the user clicks on the "New" button */
void XMLFilterSettingsDialog::onNew()
{
    filter_info_impl aTempInfo;

    // create a unique filter name
    aTempInfo.maFilterName = createUniqueFilterName(RESIDSTR(STR_DEFAULT_FILTER_NAME));

    // init default extension
    OUString aDefaultExtension(RESIDSTR(STR_DEFAULT_EXTENSION));
    aTempInfo.maExtension = aDefaultExtension;

    // set default ui name
    aTempInfo.maInterfaceName = createUniqueInterfaceName(RESIDSTR(STR_DEFAULT_UI_NAME));

    // set default application
    aTempInfo.maDocumentService = "com.sun.star.text.TextDocument";

    // execute XML Filter Dialog
    XMLFilterTabDialog aDlg( this, *getXSLTDialogResMgr(), mxContext, &aTempInfo );
    if ( aDlg.Execute() == RET_OK )
    {
        // insert the new filter
        insertOrEdit( aDlg.getNewFilterInfo() );
    }
}

// -----------------------------------------------------------------------

/** is called when the user clicks on the "Edit" Button */
void XMLFilterSettingsDialog::onEdit()
{
    // get selected filter entry
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        // get its filter info
        filter_info_impl* pOldInfo = (filter_info_impl*)pEntry->GetUserData();

        // execute XML Filter Dialog
        XMLFilterTabDialog aDlg( this, *getXSLTDialogResMgr(), mxContext, pOldInfo );
        if ( aDlg.Execute() == RET_OK )
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

// -----------------------------------------------------------------------

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

        // now count the delimeters ';'
        const sal_Unicode * pString = rExtensions.getStr();
        int i;
        for( i = 0; i < nLength; i++, pString++ )
        {
            if( *pString == sal_Unicode( ';' ) )
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

// -----------------------------------------------------------------------

/** checks if the given name is unique inside the filter factory. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueFilterName( const OUString& rFilterName )
{
    OUString aFilterName( rFilterName );
    OUString aSpace(" ");

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aFilterName ) )
    {
        aFilterName = rFilterName;
        aFilterName += aSpace;
        aFilterName += OUString::number( nId++ );
    }

    return aFilterName;
}

// -----------------------------------------------------------------------

/** checks if the given name is unique inside the type detection. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueTypeName( const OUString& rTypeName )
{
    OUString aTypeName( rTypeName );
    OUString aSpace(" ");

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aTypeName ) )
    {
        aTypeName = rTypeName;
        aTypeName += aSpace;
        aTypeName += OUString::number( nId++ );
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
        OUString* pFilterName = aFilterNames.getArray();

        const sal_Int32 nCount = aFilterNames.getLength();
        sal_Int32 nFilter;

        Sequence< PropertyValue > aValues;
        for( nFilter = 0; (nFilter < nCount); nFilter++, pFilterName++ )
        {
            Any aAny( mxFilterContainer->getByName( *pFilterName ) );
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
        OSL_FAIL( "XMLFilterSettingsDialog::createUniqueInterfaceName exception catched!" );
    }

    OUString aInterfaceName( rInterfaceName );
    if( nDefaultNumber )
    {
        aInterfaceName += " " + OUString::number( nDefaultNumber );
    }

    return aInterfaceName;
}

// -----------------------------------------------------------------------

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
                pNewInfo->maType = OUString();
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
            OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
            bOk = false;
        }
    }

    filter_info_impl* pFilterEntry( NULL );

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
    if( !pFilterEntry->maImportTemplate.isEmpty() )
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

                    SvFileStream aInputStream(pFilterEntry->maImportTemplate, STREAM_READ );
                    Reference< XInputStream > xIS( new utl::OInputStreamWrapper( aInputStream ) );
                    SvFileStream aOutputStream(aDestURL, STREAM_WRITE );
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
            OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
            aDocType = m_sDocTypePrefix;
            aDocType += pFilterEntry->maDocType;
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
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
                bOk = false;
            }
        }

        if( bOk )
        {
            try
            {
                Reference< XFlushable > xFlushable = Reference< XFlushable >::query( mxTypeDetection );
                if( xFlushable.is() )
                    xFlushable->flush();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
                OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
                bOk = false;
            }
        }
        else
        {
            if( bOk )
            {
                try
                {
                    Reference< XFlushable > xFlushable( mxFilterContainer, UNO_QUERY );
                    if( xFlushable.is() )
                        xFlushable->flush();
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
                        OSL_FAIL( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
                    }
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
            maFilterVector.push_back( pFilterEntry );
        }
    }

    return bOk;
}

// -----------------------------------------------------------------------

/** is called when the user clicks the "Test" button */
void XMLFilterSettingsDialog::onTest()
{
    // get the first selected filter
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();

        XMLFilterTestDialog aDlg(this, mxContext);
        aDlg.test( *pInfo );
    }
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::onDelete()
{
    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();

        OUString aPlaceHolder( "%s" );
        OUString aMessage(RESIDSTR(STR_WARN_DELETE));
        aMessage = aMessage.replaceFirst( aPlaceHolder, pInfo->maFilterName );

        WarningBox aWarnBox(this, (WinBits)(WB_YES_NO | WB_DEF_YES),    aMessage );
        if( aWarnBox.Execute() == RET_YES )
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

                        for( nValue = 0; (nValue < nValueCount) && !bTypeStillUsed; nValue++, pValues++ )
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

                    xFlushable = Reference< XFlushable >::query( mxTypeDetection );
                    if( xFlushable.is() )
                        xFlushable->flush();

                    // now remove entry from ui
                    m_pFilterListBox->RemoveSelection();

                    // and delete the filter entry
                    maFilterVector.erase(std::find( maFilterVector.begin(), maFilterVector.end(), pInfo ));

                    delete pInfo;
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::onDelete exception catched!" );
            }
        }
    }

    updateStates();
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::onSave()
{
    XMLFilterVector aFilters;

    int nFilters = 0;

    SvTreeListEntry* pEntry = m_pFilterListBox->FirstSelected();
    while( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();
        aFilters.push_back( pInfo );
        pEntry = m_pFilterListBox->NextSelected( pEntry );
        nFilters++;
    }

    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
        0 );

    OUString aExtensions( "*.jar" );
    OUString aFilterName(RESIDSTR(STR_FILTER_PACKAGE));
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
            aMsg = RESIDSTR(STR_FILTERS_HAVE_BEEN_SAVED);
            aMsg = aMsg.replaceFirst( sPlaceholder, OUString::number( nFilters ) );
            aMsg = aMsg.replaceFirst( sPlaceholder, aURL.GetName() );
        }
        else
        {
            aMsg = RESIDSTR(STR_FILTER_HAS_BEEN_SAVED);
            aMsg = aMsg.replaceFirst( sPlaceholder, (*aFilters.begin())->maFilterName );
            aMsg = aMsg.replaceFirst( sPlaceholder, aURL.GetName() );
        }

        InfoBox aBox(this, aMsg );
        aBox.Execute();
    }
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::onOpen()
{
    XMLFilterVector aFilters;

    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

    OUString aExtensions( "*.jar" );
    OUString aFilterName(RESIDSTR(STR_FILTER_PACKAGE));
    aFilterName += " (" + aExtensions + ")";

    aDlg.AddFilter( aFilterName, aExtensions );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString aURL( aDlg.GetPath() );

        XMLFilterJarHelper aJarHelper( mxContext );
        aJarHelper.openPackage( aURL, aFilters );

        int nFilters = 0;
        XMLFilterVector::iterator aIter( aFilters.begin() );
        while( aIter != aFilters.end() )
        {
            filter_info_impl* pInfo = (*aIter++);

            if( insertOrEdit( pInfo ) )
            {
                aFilterName = pInfo->maFilterName;
                nFilters++;
            }

            delete pInfo;
        }

        disposeFilterList();
        initFilterList();

        OUString sPlaceholder( "%s" );
        OUString aMsg;
        if( nFilters == 0 )
        {
            INetURLObject aURLObj( aURL );
            aMsg = RESIDSTR(STR_NO_FILTERS_FOUND);
            aMsg = aMsg.replaceFirst( sPlaceholder, aURLObj.GetName() );
        }
        else if( nFilters == 1 )
        {
            aMsg = RESIDSTR(STR_FILTER_INSTALLED);
            aMsg = aMsg.replaceFirst( sPlaceholder, aFilterName );

        }
        else
        {
            aMsg = RESIDSTR(STR_FILTERS_INSTALLED);
            aMsg = aMsg.replaceFirst( sPlaceholder, OUString::number( nFilters ) );
        }

        InfoBox aBox(this, aMsg );
        aBox.Execute();
    }
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::onClose()
{
    Close();
}

long XMLFilterSettingsDialog::Notify( NotifyEvent& rNEvt )
{
    // Because of of tab control first call the base class.
    long nRet = ModelessDialog::Notify( rNEvt );
    if ( !nRet )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            KeyCode         aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16          nKeyCode = aKeyCode.GetCode();
            sal_Bool        bMod1 =   pKEvt->GetKeyCode().IsMod1();

            if( nKeyCode == KEY_ESCAPE || (bMod1 && (nKeyCode == KEY_W)))
            {
                Close();
                return sal_True;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::disposeFilterList()
{
    std::vector< filter_info_impl* >::iterator aIter( maFilterVector.begin() );
    while( aIter != maFilterVector.end() )
    {
        delete (*aIter++);
    }
    maFilterVector.clear();

    m_pFilterListBox->Clear();
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::initFilterList()
{
    if( mxFilterContainer.is() )
    {
        Sequence< OUString > aFilterNames( mxFilterContainer->getElementNames() );
        OUString* pFilterName = aFilterNames.getArray();

        const sal_Int32 nCount = aFilterNames.getLength();
        sal_Int32 nFilter;

        Sequence< PropertyValue > aValues;

        filter_info_impl* pTempFilter = new filter_info_impl;
        Sequence< OUString > aUserData;

        for( nFilter = 0; nFilter < nCount; nFilter++, pFilterName++ )
        {
            aUserData.realloc(0);

            try
            {
                Any aAny( mxFilterContainer->getByName( *pFilterName ) );
                if( !(aAny >>= aValues) )
                    continue;

                OUString aFilterService;
                pTempFilter->maFilterName = *pFilterName;

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
                if( !aFilterService.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) ) )
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
                                        pTempFilter->maExtension = OUString();

                                        sal_Int32 nCount3( aExtensions.getLength() );
                                        OUString* pExtensions = aExtensions.getArray();
                                        sal_Int32 n;
                                        for( n = 0; n < nCount3; n++ )
                                        {
                                            if( n > 0 )
                                                pTempFilter->maExtension += ";";
                                            pTempFilter->maExtension += (*pExtensions++);
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
                                    sal_Bool bTemp = sal_False;
                                    pValues2->Value >>= bTemp;
                                    pTempFilter->mbReadonly |= bTemp;
                                }
                            }
                        }
                    }
                    catch( const ::com::sun::star::container::NoSuchElementException& )
                    {
                        OSL_FAIL( "Type not found, user error?" ); // TODO: error?
                    }
                }

                // add entry to internal container and to ui filter list box
                maFilterVector.push_back( pTempFilter );
                m_pFilterListBox->addFilterEntry( pTempFilter );


                pTempFilter = new filter_info_impl;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::initFilterList exception catched!" );
            }

        }

        delete pTempFilter;
    }

    SvTreeListEntry* pEntry = m_pFilterListBox->GetEntry( 0 );
    if( pEntry )
        m_pFilterListBox->Select( pEntry );
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

application_info_impl::application_info_impl( const sal_Char * pDocumentService, ResId& rUINameRes, const sal_Char * mpXMLImporter, const sal_Char * mpXMLExporter )
:   maDocumentService( pDocumentService, strlen( pDocumentService ), RTL_TEXTENCODING_ASCII_US ),
    maDocumentUIName( String( rUINameRes ) ),
    maXMLImporter( mpXMLImporter, strlen( mpXMLImporter ), RTL_TEXTENCODING_ASCII_US ),
    maXMLExporter( mpXMLExporter, strlen( mpXMLExporter ), RTL_TEXTENCODING_ASCII_US )
{
}

// -----------------------------------------------------------------------

std::vector< application_info_impl* >& getApplicationInfos()
{
    static std::vector< application_info_impl* > aInfos;

    if( aInfos.empty() )
    {
        ResId aResId1( STR_APPL_NAME_WRITER, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.text.TextDocument",
            aResId1,
            "com.sun.star.comp.Writer.XMLImporter",
            "com.sun.star.comp.Writer.XMLExporter" ) );

        ResId aResId2( STR_APPL_NAME_CALC, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.sheet.SpreadsheetDocument",
            aResId2,
            "com.sun.star.comp.Calc.XMLImporter",
            "com.sun.star.comp.Calc.XMLExporter" ) );

        ResId aResId3( STR_APPL_NAME_IMPRESS, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.presentation.PresentationDocument",
            aResId3,
            "com.sun.star.comp.Impress.XMLImporter",
            "com.sun.star.comp.Impress.XMLExporter" ) );

        ResId aResId4( STR_APPL_NAME_DRAW, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.drawing.DrawingDocument",
            aResId4,
            "com.sun.star.comp.Draw.XMLImporter",
            "com.sun.star.comp.Draw.XMLExporter" ) );

        // --- oasis file formats...
        ResId aResId5( STR_APPL_NAME_OASIS_WRITER, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.text.TextDocument",
            aResId5,
            "com.sun.star.comp.Writer.XMLOasisImporter",
            "com.sun.star.comp.Writer.XMLOasisExporter" ) );

        ResId aResId6( STR_APPL_NAME_OASIS_CALC, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.sheet.SpreadsheetDocument",
            aResId6,
            "com.sun.star.comp.Calc.XMLOasisImporter",
            "com.sun.star.comp.Calc.XMLOasisExporter" ) );

        ResId aResId7( STR_APPL_NAME_OASIS_IMPRESS, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.presentation.PresentationDocument",
            aResId7,
            "com.sun.star.comp.Impress.XMLOasisImporter",
            "com.sun.star.comp.Impress.XMLOasisExporter" ) );

        ResId aResId8( STR_APPL_NAME_OASIS_DRAW, *getXSLTDialogResMgr() );
        aInfos.push_back( new application_info_impl(
            "com.sun.star.drawing.DrawingDocument",
            aResId8,
            "com.sun.star.comp.Draw.XMLOasisImporter",
            "com.sun.star.comp.Draw.XMLOasisExporter" ) );
}

    return aInfos;
}

// -----------------------------------------------------------------------

const application_info_impl* getApplicationInfo( const OUString& rServiceName )
{
    std::vector< application_info_impl* >& rInfos = getApplicationInfos();
    for (std::vector< application_info_impl* >::const_iterator aIter( rInfos.begin() ), aEnd( rInfos.end() );
        aIter != aEnd ; ++aIter)
    {
        if( rServiceName == (*aIter)->maXMLExporter ||
            rServiceName == (*aIter)->maXMLImporter)
        {
            return (*aIter);
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------

OUString getApplicationUIName( const OUString& rServiceName )
{
    const application_info_impl* pInfo = getApplicationInfo( rServiceName );
    if( pInfo )
    {
        return pInfo->maDocumentUIName;
    }
    else
    {
        OUString aRet = RESIDSTR(STR_UNKNOWN_APPLICATION);
        if( !rServiceName.isEmpty() )
        {
            aRet += " (" + rServiceName + ")";
        }
        return aRet;
    }
}

SvxPathControl::SvxPathControl(Window* pParent)
    : VclVBox(pParent)
    , bHasBeenShown(false)
{
    m_pHeaderBar = new HeaderBar(this, WB_BOTTOMBORDER);
    m_pHeaderBar->set_height_request(GetTextHeight() + 6);

    m_pFocusCtrl = new XMLFilterListBox(this);
    m_pFocusCtrl->set_fill(true);
    m_pFocusCtrl->set_expand(true);
}

#define ITEMID_NAME     1
#define ITEMID_TYPE     2

void SvxPathControl::setAllocation(const Size &rAllocation)
{
    VclVBox::setAllocation(rAllocation);

    if (!bHasBeenShown)
        bHasBeenShown = IsReallyShown();

    if (!bHasBeenShown)
    {
        std::vector<long> aWidths;
        m_pFocusCtrl->getPreferredDimensions(aWidths);
        long nFirstColumnWidth = aWidths[1];
        m_pHeaderBar->SetItemSize(ITEMID_NAME, nFirstColumnWidth);
        m_pHeaderBar->SetItemSize(ITEMID_TYPE, 0xFFFF);
        long nTabs[] = {2, 0, nFirstColumnWidth};
        m_pFocusCtrl->SetTabs(&nTabs[0], MAP_PIXEL);
    }
}

SvxPathControl::~SvxPathControl()
{
    delete m_pFocusCtrl;
    delete m_pHeaderBar;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxPathControl(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxPathControl(pParent);
}

long SvxPathControl::Notify(NotifyEvent& rNEvt)
{
    long nRet = VclVBox::Notify(rNEvt);

    if ( m_pFocusCtrl && rNEvt.GetWindow() != m_pFocusCtrl && rNEvt.GetType() == EVENT_GETFOCUS )
        m_pFocusCtrl->GrabFocus();

    return nRet;
}

XMLFilterListBox::XMLFilterListBox(SvxPathControl* pParent)
    : SvTabListBox(pParent, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP)
    , mbFirstPaint(true)
    , m_pHeaderBar(pParent->getHeaderBar())
{
    Size aBoxSize( pParent->GetOutputSizePixel() );

    m_pHeaderBar->SetEndDragHdl( LINK( this, XMLFilterListBox, HeaderEndDrag_Impl ) );

    OUString aStr1(RESIDSTR(STR_COLUMN_HEADER_NAME));
    OUString aStr2(RESIDSTR(STR_COLUMN_HEADER_TYPE));

    long nTabSize = aBoxSize.Width() / 2;

    m_pHeaderBar->InsertItem( ITEMID_NAME, aStr1, nTabSize,
                            HIB_LEFT | HIB_VCENTER );
    m_pHeaderBar->InsertItem( ITEMID_TYPE, aStr2, nTabSize,
                            HIB_LEFT | HIB_VCENTER );

    static long nTabs[] = {2, 0, nTabSize };

    SetSelectionMode( MULTIPLE_SELECTION );
    SetTabs( &nTabs[0], MAP_PIXEL );
    SetScrolledHdl( LINK( this, XMLFilterListBox, TabBoxScrollHdl_Impl ) );
    SetHighlightRange();
    Show();
    m_pHeaderBar->Show();
}

// -----------------------------------------------------------------------

void XMLFilterListBox::Paint( const Rectangle& rRect )
{
    if( mbFirstPaint )
    {
        mbFirstPaint = false;
        RepaintScrollBars();
    }

    SvTabListBox::Paint( rRect );
}

IMPL_LINK( XMLFilterListBox, TabBoxScrollHdl_Impl, SvTabListBox*, /* pList */ )
{
    m_pHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterListBox, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( pBar && !pBar->GetCurItemId() )
        return 0;

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
            aSz.Width() =  nW + nTmpSz;
            nTmpSz += nW;
            SetTab( i, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
        }
    }
    return 1;
}

// -----------------------------------------------------------------------

/** adds a new filter info entry to the ui filter list */
void XMLFilterListBox::addFilterEntry( const filter_info_impl* pInfo )
{
    const OUString aEntryStr( getEntryString( pInfo ) );
    InsertEntryToColumn( aEntryStr, LIST_APPEND, 0xffff, (void*)pInfo );
}

// -----------------------------------------------------------------------

void XMLFilterListBox::changeEntry( const filter_info_impl* pInfo )
{
    const sal_uLong nCount = GetEntryCount();
    sal_uLong nPos;
    for( nPos = 0; nPos < nCount; nPos++ )
    {
        SvTreeListEntry* pEntry = GetEntry( nPos );
        if( (filter_info_impl*)pEntry->GetUserData() == pInfo )
        {
            OUString aEntryText( getEntryString( pInfo ) );
            SetEntryText( aEntryText, pEntry );
            break;
        }
    }
}

// -----------------------------------------------------------------------

OUString XMLFilterListBox::getEntryString( const filter_info_impl* pInfo ) const
{
    OUString aEntryStr( pInfo->maFilterName + "\t");
    if ( !pInfo->maExportService.isEmpty() )
        aEntryStr += OUString( getApplicationUIName( pInfo->maExportService ) );
    else
        aEntryStr += OUString( getApplicationUIName( pInfo->maImportService ) );
    aEntryStr += " - ";

    if( pInfo->maFlags & 1 )
    {
        if( pInfo->maFlags & 2 )
        {
            aEntryStr += RESIDSTR(STR_IMPORT_EXPORT);
        }
        else
        {
            aEntryStr += RESIDSTR(STR_IMPORT_ONLY);
        }
    }
    else if( pInfo->maFlags & 2 )
    {
        aEntryStr += RESIDSTR(STR_EXPORT_ONLY);
    }
    else
    {
        aEntryStr += RESIDSTR(STR_UNDEFINED_FILTER);
    }

    return aEntryStr;
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

filter_info_impl::filter_info_impl()
:   maFlags(0x00080040),
    maFileFormatVersion(0),
    mnDocumentIconID(0),
    mbReadonly(sal_False),
    mbNeedsXSLT2(sal_False)
{
}

// -----------------------------------------------------------------------

filter_info_impl::filter_info_impl( const filter_info_impl& rInfo ) :
    maFilterName( rInfo.maFilterName ),
    maType( rInfo.maType ),
    maDocumentService( rInfo.maDocumentService ),
    maFilterService( rInfo.maFilterService ),
    maInterfaceName( rInfo.maInterfaceName ),
    maComment( rInfo.maComment ),
    maExtension( rInfo.maExtension ),
    maExportXSLT( rInfo.maExportXSLT ),
    maImportXSLT( rInfo.maImportXSLT ),
    maImportTemplate( rInfo.maImportTemplate ),
    maDocType( rInfo.maDocType ),
    maImportService( rInfo.maImportService ),
    maExportService( rInfo.maExportService ),
    maFlags( rInfo.maFlags ),
    maFileFormatVersion( rInfo.maFileFormatVersion ),
    mnDocumentIconID( rInfo.mnDocumentIconID ),
    mbReadonly( rInfo.mbReadonly ),
    mbNeedsXSLT2( rInfo.mbNeedsXSLT2 )
{
}

// -----------------------------------------------------------------------

int filter_info_impl::operator==( const filter_info_impl& r ) const
{
    if( maFilterName != r.maFilterName ||
        maType != r.maType ||
        maDocumentService != r.maDocumentService ||
        maFilterService != r.maFilterService ||
        maInterfaceName != r.maInterfaceName ||
        maComment != r.maComment ||
        maExtension != r.maExtension ||
        maDocType != r.maDocType ||
        maExportXSLT != r.maExportXSLT ||
        maImportXSLT != r.maImportXSLT ||
        maExportService != r.maExportService ||
        maImportService != r.maImportService ||
        maImportTemplate != r.maImportTemplate ||
        maFlags != r.maFlags ||
        maFileFormatVersion != r.maFileFormatVersion ||
        mbNeedsXSLT2 != r.mbNeedsXSLT2
        )
        return false;

    return true;
}

// -----------------------------------------------------------------------

Sequence< OUString > filter_info_impl::getFilterUserData() const
{
    Sequence< OUString > aUserData(8);

    aUserData[0] = OUString( "com.sun.star.documentconversion.XSLTFilter" );
    aUserData[1] = OUString::boolean( mbNeedsXSLT2 );
    aUserData[2] = maImportService;
    aUserData[3] = maExportService;
    aUserData[4] = maImportXSLT;
    aUserData[5] = maExportXSLT;
    aUserData[7] = maComment;

    return aUserData;
}


// -----------------------------------------------------------------------

OUString string_encode( const OUString & rText )
{

    static sal_Bool const aCharClass[]
    =  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* UricNoSlash */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       };


    return Uri::encode( rText, aCharClass, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

// -----------------------------------------------------------------------

OUString string_decode( const OUString & rText )
{
    return Uri::decode( rText, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
}

// -----------------------------------------------------------------------

bool isFileURL( const OUString & rURL )
{
    return rURL.startsWith("file:");
}

// -----------------------------------------------------------------------

bool copyStreams( Reference< XInputStream > xIS, Reference< XOutputStream > xOS )
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
        OSL_FAIL( "copyStreams() exception catched!" );
    }

    return false;
}

// -----------------------------------------------------------------------

bool createDirectory( OUString& rURL )
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
