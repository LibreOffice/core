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
#include <osl/file.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>

#include <rtl/uri.hxx>

#include <algorithm>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltersettingsdialog.hrc"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfilterjar.hxx"
#include "xmlfilterhelpids.hrc"

using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

using ::rtl::OUString;
using ::rtl::Uri;

ResMgr* XMLFilterSettingsDialog::mpResMgr = NULL;

XMLFilterSettingsDialog::XMLFilterSettingsDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF ) :
    WorkWindow( pParent, ResId( DLG_XML_FILTER_SETTINGS_DIALOG, rResMgr ) ),
    mxMSF( rxMSF ),
    maCtrlFilterList( this, ResId( CTRL_XML_FILTER_LIST, rResMgr ) ),
    maPBNew( this, ResId( PB_XML_FILTER_NEW, rResMgr ) ),
    maPBEdit( this, ResId( PB_XML_FILTER_EDIT, rResMgr ) ),
    maPBTest( this, ResId( PB_XML_FILTER_TEST, rResMgr ) ),
    maPBDelete( this, ResId( PB_XML_FILTER_DELETE, rResMgr ) ),
    maPBSave( this, ResId( PB_XML_FILTER_SAVE, rResMgr ) ),
    maPBOpen( this, ResId( PB_XML_FILTER_OPEN, rResMgr ) ),
    maPBHelp( this, ResId( BTN_XML_FILTER_HELP, rResMgr ) ),
    maPBClose( this, ResId( PB_XML_FILTER_CLOSE, rResMgr ) ),
    mbIsClosable(true),
    sTemplatePath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/template/") ),
    sDocTypePrefix( RTL_CONSTASCII_USTRINGPARAM( "doctype:") )
{
    FreeResource();

    mpResMgr = &rResMgr;

    mpFilterListBox = new XMLFilterListBox( &maCtrlFilterList );
    mpFilterListBox->SetSelectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    mpFilterListBox->SetDeselectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    mpFilterListBox->SetDoubleClickHdl( LINK( this, XMLFilterSettingsDialog, DoubleClickHdl_Impl ) );
    mpFilterListBox->SetAccessibleName(String( RESID( STR_XML_FILTER_LISTBOX )));
    maCtrlFilterList.SetAccessibleName(String( RESID( STR_XML_FILTER_LISTBOX )));
    mpFilterListBox->SetHelpId( HID_XML_FILTER_LIST );

    maPBNew.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBEdit.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBTest.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBDelete.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBSave.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBOpen.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );
    maPBClose.SetClickHdl(LINK( this, XMLFilterSettingsDialog, ClickHdl_Impl ) );

    try
    {
        mxFilterContainer = Reference< XNameContainer >::query( rxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" )) ) );
        mxTypeDetection = Reference< XNameContainer >::query( rxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" )) ));
        mxExtendedTypeDetection = Reference< XNameContainer >::query( rxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExtendedTypeDetectionFactory" )) ) );

        Reference< XConfigManager > xCfgMgr( mxMSF->createInstance(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" )) ), UNO_QUERY );
        if( xCfgMgr.is() )
        {
            sTemplatePath = xCfgMgr->substituteVariables( sTemplatePath );
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL( "XMLFilterSettingsDialog::XMLFilterSettingsDialog exception catched!" );
    }
}

// -----------------------------------------------------------------------

XMLFilterSettingsDialog::~XMLFilterSettingsDialog()
{
    delete mpFilterListBox;
}

// -----------------------------------------------------------------------

IMPL_LINK(XMLFilterSettingsDialog, ClickHdl_Impl, PushButton *, pButton )
{
    mbIsClosable = false;

    if( &maPBNew == pButton )
    {
        onNew();
    }
    else if( &maPBEdit == pButton )
    {
        onEdit();
    }
    else if( &maPBTest == pButton )
    {
        onTest();
    }
    else if( &maPBDelete == pButton )
    {
        onDelete();
    }
    else if( &maPBSave == pButton )
    {
        onSave();
    }
    else if( &maPBOpen == pButton )
    {
        onOpen();
    }
    else if( &maPBClose == pButton )
    {
        onClose();
    }

    mbIsClosable = true;
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
    return mbIsClosable;
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::ShowWindow()
{
    maCtrlFilterList.GrabFocus();
    disposeFilterList();
    mpFilterListBox->Clear();
    initFilterList();
    updateStates();
    mpFilterListBox->Reset();

    WorkWindow::Show( sal_True );
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::updateStates()
{
    SvLBoxEntry* pSelectedEntry = mpFilterListBox->FirstSelected();

    bool bHasSelection = pSelectedEntry != NULL;

    bool bMultiSelection = bHasSelection && (mpFilterListBox->NextSelected( pSelectedEntry ) != NULL );
    bool bIsReadonly = false;
    bool bIsDefault = false;
    if(pSelectedEntry)
    {
        filter_info_impl* pInfo = (filter_info_impl*)pSelectedEntry->GetUserData();
        bIsReadonly = 0 != pInfo->mbReadonly;

        sal_Int32 nFact = SvtModuleOptions::E_WRITER;
        while(nFact <= SvtModuleOptions::E_BASIC)
        {
            ::rtl::OUString sDefault = maModuleOpt.GetFactoryDefaultFilter((SvtModuleOptions::EFactory)nFact);
            if( sDefault == pInfo->maFilterName )
            {
                bIsDefault = true;
                break;
            }
            ++nFact;
        }
    }
    maPBEdit.Enable( bHasSelection && !bMultiSelection && !bIsReadonly);
    maPBTest.Enable( bHasSelection && !bMultiSelection );
    maPBDelete.Enable( bHasSelection && !bMultiSelection && !bIsReadonly && !bIsDefault);
    maPBSave.Enable( bHasSelection );
}

// -----------------------------------------------------------------------

/** is called when the user clicks on the "New" button */
void XMLFilterSettingsDialog::onNew()
{
    filter_info_impl aTempInfo;

    // create a unique filter name
    aTempInfo.maFilterName = createUniqueFilterName( String( RESID( STR_DEFAULT_FILTER_NAME ) ) );

    // init default extension
    String aDefaultExtension( RESID( STR_DEFAULT_EXTENSION ) );
    aTempInfo.maExtension = aDefaultExtension;

    // set default ui name
    aTempInfo.maInterfaceName = createUniqueInterfaceName( String( RESID( STR_DEFAULT_UI_NAME ) ) );

    // set default application
    aTempInfo.maDocumentService = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ));

    // execute XML Filter Dialog
    XMLFilterTabDialog aDlg( this, *mpResMgr, mxMSF, &aTempInfo );
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
    SvLBoxEntry* pEntry = mpFilterListBox->FirstSelected();
    if( pEntry )
    {
        // get its filter info
        filter_info_impl* pOldInfo = (filter_info_impl*)pEntry->GetUserData();

        // execute XML Filter Dialog
        XMLFilterTabDialog aDlg( this, *mpResMgr, mxMSF, pOldInfo );
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
static Sequence< OUString > createExtensionsSequence( const rtl::OUString& rExtensions )
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
        nLastIndex = rExtensions.indexOf( sal_Unicode( ';' ), nLastIndex );

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
    OUString aSpace( sal_Unicode( ' ' ) );

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aFilterName ) )
    {
        aFilterName = rFilterName;
        aFilterName += aSpace;
        aFilterName += OUString::valueOf( nId++ );
    }

    return aFilterName;
}

// -----------------------------------------------------------------------

/** checks if the given name is unique inside the type detection. If not,
    numbers are added until the returned name is unique */
OUString XMLFilterSettingsDialog::createUniqueTypeName( const OUString& rTypeName )
{
    OUString aTypeName( rTypeName );
    OUString aSpace( sal_Unicode( ' ' ) );

    sal_Int32 nId = 2;

    while( mxFilterContainer->hasByName( aTypeName ) )
    {
        aTypeName = rTypeName;
        aTypeName += aSpace;
        aTypeName += OUString::valueOf( nId++ );
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
        aInterfaceName += OUString( sal_Unicode( ' ' ) );
        aInterfaceName += String::CreateFromInt32( nDefaultNumber );
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
        if( !pFilterEntry->maImportTemplate.matchIgnoreAsciiCase( sTemplatePath ) )
        {
            INetURLObject aSourceURL( pFilterEntry->maImportTemplate );
            if( !aSourceURL.GetName().isEmpty() )
            {
                OUString aDestURL( sTemplatePath );
                aDestURL += pFilterEntry->maFilterName;
                aDestURL += OUString( sal_Unicode('/') );
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

        aFilterData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" ) );
        aFilterData[0].Value <<= pFilterEntry->maType;

        aFilterData[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) );
        aFilterData[1].Value <<= pFilterEntry->maInterfaceName;

        aFilterData[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentService" ) );
        aFilterData[2].Value <<= pFilterEntry->maDocumentService;

        aFilterData[3].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterService" ) );
        aFilterData[3].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) );

        aFilterData[4].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Flags" ) );
        aFilterData[4].Value <<= pFilterEntry->maFlags;

        aFilterData[5].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UserData" ) );
        aFilterData[5].Value <<= aUserData;

        aFilterData[6].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileFormatVersion" ) );
        aFilterData[6].Value <<= pFilterEntry->maFileFormatVersion;

        aFilterData[7].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "TemplateName" ) );
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

        aValues[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) );
        aValues[0].Value <<= pFilterEntry->maInterfaceName;
        aValues[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ClipboardFormat" ) );
        OUString aDocType;
        if( !pFilterEntry->maDocType.match( sDocTypePrefix ) )
        {
            aDocType = sDocTypePrefix;
            aDocType += pFilterEntry->maDocType;
        }
        else
        {
            aDocType = pFilterEntry->maDocType;
        }
        if (aDocType == sDocTypePrefix)
            aValues[1].Value <<= OUString();
        else
            aValues[1].Value <<= aDocType;

        aValues[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentIconID" ) );
        aValues[2].Value <<= pFilterEntry->mnDocumentIconID;

        aValues[3].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Extensions" ) );
        aValues[3].Value <<= createExtensionsSequence( pFilterEntry->maExtension );

        // the detect service will only be registered, if a doctype/search token was specified
        if (aDocType.getLength() > sDocTypePrefix.getLength())
        {
            aValues.realloc(5);
            aValues[4].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DetectService" ) );
            aValues[4].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.filters.XMLFilterDetect" ) );
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
            OUString sFilterDetectService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.filters.XMLFilterDetect") );
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
                                    OUString aType( aTypes[nStr] );
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
            mpFilterListBox->changeEntry( pFilterEntry );
        }
        else
        {
            mpFilterListBox->addFilterEntry( pFilterEntry );
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
    SvLBoxEntry* pEntry = mpFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();

        XMLFilterTestDialog aDlg( this, *mpResMgr, mxMSF );
        aDlg.test( *pInfo );
    }
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::onDelete()
{
    SvLBoxEntry* pEntry = mpFilterListBox->FirstSelected();
    if( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();

        String aPlaceHolder( RTL_CONSTASCII_USTRINGPARAM("%s") );
        String aMessage(RESID(STR_WARN_DELETE));
        aMessage.SearchAndReplace( aPlaceHolder, pInfo->maFilterName );

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
                    mpFilterListBox->RemoveSelection();

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

    SvLBoxEntry* pEntry = mpFilterListBox->FirstSelected();
    while( pEntry )
    {
        filter_info_impl* pInfo = (filter_info_impl*)pEntry->GetUserData();
        aFilters.push_back( pInfo );
        pEntry = mpFilterListBox->NextSelected( pEntry );
        nFilters++;
    }

    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
        0 );

    String aExtensions( RTL_CONSTASCII_USTRINGPARAM("*.jar") );
    String aFilterName( RESID( STR_FILTER_PACKAGE ) );
    aFilterName += String( RTL_CONSTASCII_USTRINGPARAM(" (") );
    aFilterName += aExtensions;
    aFilterName += sal_Unicode(')');

    aDlg.AddFilter( aFilterName, aExtensions );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        XMLFilterJarHelper aJarHelper( mxMSF );
        aJarHelper.savePackage( aDlg.GetPath(), aFilters );

        INetURLObject aURL( aDlg.GetPath() );

        String sPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%s" ) );

        String aMsg;
        if( nFilters > 0 )
        {
            aMsg = String( RESID( STR_FILTERS_HAVE_BEEN_SAVED ) );
            aMsg.SearchAndReplace( sPlaceholder, String::CreateFromInt32(nFilters) );
            aMsg.SearchAndReplace( sPlaceholder, aURL.GetName() );
        }
        else
        {
            aMsg = String( RESID( STR_FILTER_HAS_BEEN_SAVED ) );
            aMsg.SearchAndReplace( sPlaceholder, (*aFilters.begin())->maFilterName );
            aMsg.SearchAndReplace( sPlaceholder, aURL.GetName() );
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

    String aExtensions( RTL_CONSTASCII_USTRINGPARAM("*.jar") );
    String aFilterName( RESID( STR_FILTER_PACKAGE ) );
    aFilterName += String( RTL_CONSTASCII_USTRINGPARAM(" (") );
    aFilterName += aExtensions;
    aFilterName += sal_Unicode(')');

    aDlg.AddFilter( aFilterName, aExtensions );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString aURL( aDlg.GetPath() );

        XMLFilterJarHelper aJarHelper( mxMSF );
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

        String sPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%s" ) );
        String aMsg;
        if( nFilters == 0 )
        {
            INetURLObject aURLObj( aURL );
            aMsg = String( RESID( STR_NO_FILTERS_FOUND ) );
            aMsg.SearchAndReplace( sPlaceholder, aURLObj.GetName() );
        }
        else if( nFilters == 1 )
        {
            aMsg = String( RESID( STR_FILTER_INSTALLED ) );
            aMsg.SearchAndReplace( sPlaceholder, aFilterName );

        }
        else
        {
            aMsg = String( RESID( STR_FILTERS_INSTALLED ) );
            aMsg.SearchAndReplace( sPlaceholder, String::CreateFromInt32(nFilters) );
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
    // Zuerst Basisklasse rufen wegen TabSteuerung
    long nRet = WorkWindow::Notify( rNEvt );
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

    mpFilterListBox->Clear();
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
                pTempFilter->maXSLTTransformerImpl = aUserData[1];
                pTempFilter->maImportService = aUserData[2];
                pTempFilter->maExportService = aUserData[3];
                pTempFilter->maImportXSLT = aUserData[4];
                pTempFilter->maExportXSLT = aUserData[5];
                if( aUserData.getLength() >= 7 )
                    pTempFilter->maDTD = aUserData[6];
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

                                    if( aDocType.match( sDocTypePrefix ) )
                                        aDocType = aDocType.copy( sDocTypePrefix.getLength() );

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
                                                pTempFilter->maExtension += OUString( sal_Unicode(';') );
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
                mpFilterListBox->addFilterEntry( pTempFilter );


                pTempFilter = new filter_info_impl;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterSettingsDialog::initFilterList exception catched!" );
            }

        }

        delete pTempFilter;
    }

    SvLBoxEntry* pEntry = mpFilterListBox->GetEntry( 0 );
    if( pEntry )
        mpFilterListBox->Select( pEntry );
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
    std::vector< application_info_impl* >::iterator aIter( rInfos.begin() );
    while( aIter != rInfos.end() )
    {
        // if( rServiceName == (*aIter)->maDocumentService )
        if( rServiceName == (*aIter)->maXMLExporter ||
            rServiceName == (*aIter)->maXMLImporter)
        {
            return (*aIter);
        }
        ++aIter;
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
        OUString aRet = String( RESID( STR_UNKNOWN_APPLICATION ) );
        if( !rServiceName.isEmpty() )
        {
            aRet += OUString( RTL_CONSTASCII_USTRINGPARAM( " (" ));
            aRet += rServiceName;
            aRet += OUString( RTL_CONSTASCII_USTRINGPARAM( ")" ));
        }
        return aRet;
    }
}

// -----------------------------------------------------------------------

ResMgr* getXSLTDialogResMgr()
{
    return XMLFilterSettingsDialog::mpResMgr;
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

long SvxPathControl_Impl::Notify( NotifyEvent& rNEvt )
{
    long nRet = Control::Notify( rNEvt );

    if ( m_pFocusCtrl && rNEvt.GetWindow() != m_pFocusCtrl && rNEvt.GetType() == EVENT_GETFOCUS )
        m_pFocusCtrl->GrabFocus();
    return nRet;
}

#define ITEMID_NAME     1
#define ITEMID_TYPE     2

XMLFilterListBox::XMLFilterListBox( SvxPathControl_Impl * pParent )
:   SvTabListBox( pParent, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP ),
    mbFirstPaint( true )
{
    Size aBoxSize( pParent->GetOutputSizePixel() );

    mpHeaderBar = new HeaderBar( pParent, /*WB_BUTTONSTYLE | */ WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), Size( aBoxSize.Width(), 16 ) );
    mpHeaderBar->SetEndDragHdl( LINK( this, XMLFilterListBox, HeaderEndDrag_Impl ) );

    String aStr1( RESID( STR_COLUMN_HEADER_NAME ) );
    String aStr2( RESID( STR_COLUMN_HEADER_TYPE ) );

    long nTabSize = aBoxSize.Width() / 2;

    mpHeaderBar->InsertItem( ITEMID_NAME, aStr1, nTabSize,
                            HIB_LEFT | HIB_VCENTER );
    mpHeaderBar->InsertItem( ITEMID_TYPE, aStr2, nTabSize,
                            HIB_LEFT | HIB_VCENTER );

    static long nTabs[] = {3, 0, nTabSize, 2*nTabSize };
    Size aHeadSize( mpHeaderBar->GetSizePixel() );

    pParent->SetFocusControl( this );
//  SetDoubleClickHdl( aLink );
//  SetSelectHdl( LINK( this, SvxPathTabPage, PathSelect_Impl ) );
    SetSelectionMode( MULTIPLE_SELECTION );
    SetPosSizePixel( Point( 0, aHeadSize.Height() ), Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    SetTabs( &nTabs[0], MAP_PIXEL );
    SetScrolledHdl( LINK( this, XMLFilterListBox, TabBoxScrollHdl_Impl ) );
    SetHighlightRange();
//  SetHelpId( HID_OPTPATH_CTL_PATH );
//  mpHeaderBar->SetHelpId( HID_OPTPATH_HEADERBAR );
    Show();
    mpHeaderBar->Show();
}

// -----------------------------------------------------------------------

XMLFilterListBox::~XMLFilterListBox()
{
    delete mpHeaderBar;
}

// -----------------------------------------------------------------------

void XMLFilterListBox::Reset()
{
    Size aBoxSize( Window::GetParent()->GetOutputSizePixel() );
    long nTabSize = aBoxSize.Width() / 2;
    static long nTabs[] = {3, 0, nTabSize, 2*nTabSize };
    SetTabs( &nTabs[0], MAP_PIXEL );
    mpHeaderBar->SetItemSize( ITEMID_NAME, nTabSize );
    mpHeaderBar->SetItemSize( ITEMID_TYPE, nTabSize );
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
    mpHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterListBox, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( pBar && !pBar->GetCurItemId() )
        return 0;

    if ( !mpHeaderBar->IsItemMode() )
    {
        Size aSz;
        sal_uInt16 nTabs = mpHeaderBar->GetItemCount();
        long nTmpSz = 0;
        long nWidth = mpHeaderBar->GetItemSize(ITEMID_NAME);
        long nBarWidth = mpHeaderBar->GetSizePixel().Width();

        if(nWidth < 30)
            mpHeaderBar->SetItemSize( ITEMID_TYPE, 30);
        else if ( ( nBarWidth - nWidth ) < 30 )
            mpHeaderBar->SetItemSize( ITEMID_TYPE, nBarWidth - 30 );

        for ( sal_uInt16 i = 1; i <= nTabs; ++i )
        {
            long nW = mpHeaderBar->GetItemSize(i);
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
    const XubString aEntryStr( getEntryString( pInfo ) );
    InsertEntryToColumn( aEntryStr, LIST_APPEND, 0xffff, (void*)pInfo );
}

// -----------------------------------------------------------------------

void XMLFilterListBox::changeEntry( const filter_info_impl* pInfo )
{
    const sal_uLong nCount = GetEntryCount();
    sal_uLong nPos;
    for( nPos = 0; nPos < nCount; nPos++ )
    {
        SvLBoxEntry* pEntry = GetEntry( nPos );
        if( (filter_info_impl*)pEntry->GetUserData() == pInfo )
        {
            XubString aEntryText( getEntryString( pInfo ) );
            SetEntryText( aEntryText, pEntry );
            break;
        }
    }
}

// -----------------------------------------------------------------------

String XMLFilterListBox::getEntryString( const filter_info_impl* pInfo ) const
{
    String aEntryStr( pInfo->maFilterName );
    aEntryStr += '\t';
    // aEntryStr += String( getApplicationUIName( pInfo->maDocumentService ) );
    if ( !pInfo->maExportService.isEmpty() )
        aEntryStr += String( getApplicationUIName( pInfo->maExportService ) );
    else
        aEntryStr += String( getApplicationUIName( pInfo->maImportService ) );
    aEntryStr += ' ';
    aEntryStr += '-';
    aEntryStr += ' ';

    if( pInfo->maFlags & 1 )
    {
        if( pInfo->maFlags & 2 )
        {
            aEntryStr += String( RESID( STR_IMPORT_EXPORT ) );
        }
        else
        {
            aEntryStr += String( RESID( STR_IMPORT_ONLY ) );
        }
    }
    else if( pInfo->maFlags & 2 )
    {
        aEntryStr += String( RESID( STR_EXPORT_ONLY ) );
    }
    else
    {
        aEntryStr += String( RESID( STR_UNDEFINED_FILTER ) );
    }

    return aEntryStr;
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

filter_info_impl::filter_info_impl()
:   maFlags(0x00080040),
    maFileFormatVersion(0),
    mnDocumentIconID(0),
    mbReadonly(sal_False)
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
    maDTD( rInfo.maDTD ),
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
    maXSLTTransformerImpl( rInfo.maXSLTTransformerImpl )
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
        maDTD != r.maDTD ||
        maExportXSLT != r.maExportXSLT ||
        maImportXSLT != r.maImportXSLT ||
        maExportService != r.maExportService ||
        maImportService != r.maImportService ||
        maImportTemplate != r.maImportTemplate ||
        maFlags != r.maFlags ||
        maFileFormatVersion != r.maFileFormatVersion ||
        maXSLTTransformerImpl != r.maXSLTTransformerImpl
        )
        return false;

    return true;
}

// -----------------------------------------------------------------------

Sequence< OUString > filter_info_impl::getFilterUserData() const
{
    Sequence< OUString > aUserData(8);

    aUserData[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.documentconversion.XSLTFilter" ) );
    aUserData[1] = maXSLTTransformerImpl;
    aUserData[2] = maImportService;
    aUserData[3] = maExportService;
    aUserData[4] = maImportXSLT;
    aUserData[5] = maExportXSLT;
    aUserData[6] = maDTD;
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

bool isFileURL( const ::rtl::OUString & rURL )
{
    return rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("file:") ) == 0;
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
        nLastIndex = rURL.indexOf( sal_Unicode('/'), nLastIndex + 1);
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
