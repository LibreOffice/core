/*************************************************************************
 *
 *  $RCSfile: xmlfiltersettingsdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:34:02 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#include <rtl/uri.hxx>

#include <algorithm>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltersettingsdialog.hrc"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertestdialog.hxx"
#include "xmlfilterjar.hxx"
#include "xmlfilterhelpids.hrc"

using namespace rtl;
using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

ResMgr* XMLFilterSettingsDialog::mpResMgr = NULL;

XMLFilterSettingsDialog::XMLFilterSettingsDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF ) :
    WorkWindow( pParent, ResId( DLG_XML_FILTER_SETTINGS_DIALOG, &rResMgr ) ),
    mxMSF( rxMSF ),
    maCtrlFilterList( this, ResId( CTRL_XML_FILTER_LIST, &rResMgr ) ),
    maPBNew( this, ResId( PB_XML_FILTER_NEW, &rResMgr ) ),
    maPBEdit( this, ResId( PB_XML_FILTER_EDIT, &rResMgr ) ),
    maPBTest( this, ResId( PB_XML_FILTER_TEST, &rResMgr ) ),
    maPBDelete( this, ResId( PB_XML_FILTER_DELETE, &rResMgr ) ),
    maPBSave( this, ResId( PB_XML_FILTER_SAVE, &rResMgr ) ),
    maPBOpen( this, ResId( PB_XML_FILTER_OPEN, &rResMgr ) ),
    maPBHelp( this, ResId( BTN_XML_FILTER_HELP, &rResMgr ) ),
    maPBClose( this, ResId( PB_XML_FILTER_CLOSE, &rResMgr ) ),
    mbIsClosable(true),
    sTemplatePath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/template/") ),
    sDocTypePrefix( RTL_CONSTASCII_USTRINGPARAM( "doctype:") )
{
    mpResMgr = &rResMgr;

    mpFilterListBox = new XMLFilterListBox( &maCtrlFilterList );
    mpFilterListBox->SetSelectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    mpFilterListBox->SetDeselectHdl( LINK( this, XMLFilterSettingsDialog, SelectionChangedHdl_Impl ) );
    mpFilterListBox->SetDoubleClickHdl( LINK( this, XMLFilterSettingsDialog, DoubleClickHdl_Impl ) );
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
        mxFilterContainer = Reference< XNameContainer >::query( rxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.FilterFactory" ) ) );
        mxTypeDetection = Reference< XNameContainer >::query( rxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.TypeDetection" ) ));
        mxExtendedTypeDetection = Reference< XNameContainer >::query( rxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.ExtendedTypeDetectionFactory" ) ) );

        Reference< XConfigManager > xCfgMgr( mxMSF->createInstance(OUString::createFromAscii("com.sun.star.config.SpecialConfigManager") ), UNO_QUERY );
        if( xCfgMgr.is() )
        {
            sTemplatePath = xCfgMgr->substituteVariables( sTemplatePath );
        }
    }
    catch(Exception&)
    {
        DBG_ERROR( "XMLFilterSettingsDialog::XMLFilterSettingsDialog exception catched!" );
    }
}

// -----------------------------------------------------------------------

XMLFilterSettingsDialog::~XMLFilterSettingsDialog()
{
    delete mpFilterListBox;

    FreeResource();
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

IMPL_LINK(XMLFilterSettingsDialog, SelectionChangedHdl_Impl, void *, EMPTYARG )
{
    updateStates();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK(XMLFilterSettingsDialog, DoubleClickHdl_Impl, void *, EMPTYARG )
{
    return 0;
}

bool XMLFilterSettingsDialog::isClosable()
{
    return mbIsClosable;
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::Show()
{
    maCtrlFilterList.GrabFocus();
    disposeFilterList();
    mpFilterListBox->Clear();
    initFilterList();
    updateStates();
    mpFilterListBox->Reset();

    WorkWindow::Show( TRUE );
}

// -----------------------------------------------------------------------

void XMLFilterSettingsDialog::updateStates()
{
    SvLBoxEntry* pSelectedEntry = mpFilterListBox->FirstSelected();

    bool bHasSelection = pSelectedEntry != NULL;

    bool bMultiSelection = bHasSelection && (mpFilterListBox->NextSelected( pSelectedEntry ) != NULL );
    maPBEdit.Enable( bHasSelection && !bMultiSelection );
    maPBTest.Enable( bHasSelection && !bMultiSelection );
    maPBDelete.Enable( bHasSelection && !bMultiSelection );
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
    aTempInfo.maDocumentService = OUString::createFromAscii("com.sun.star.text.TextDocument");

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
                if( pValues->Name.equalsAscii( "UIName" ) )
                {
                    OUString aInterfaceName;
                    pValues->Value >>= aInterfaceName;


                    // see if this filter matches our default filter name
                    if( aInterfaceName.match( rInterfaceName ) )
                    {
                        // if yes, make sure we generate a unique name with a higher number
                        // this is dump but fast
                        sal_Int32 nNumber = aInterfaceName.copy( rInterfaceName.getLength() ).toInt32();
                        if( nNumber > nDefaultNumber )
                            nDefaultNumber = nNumber + 1;
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "XMLFilterSettingsDialog::createUniqueInterfaceName exception catched!" );
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
        catch( Exception& )
        {
            DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
            bOk = false;
        }
    }

    filter_info_impl* pFilterEntry;

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
    if( pFilterEntry->maImportTemplate.getLength() )
    {
        if( !pFilterEntry->maImportTemplate.matchIgnoreAsciiCase( sTemplatePath ) )
        {
            INetURLObject aSourceURL( pFilterEntry->maImportTemplate );
            if( aSourceURL.GetName().Len() != 0 )
            {
                OUString aDestURL( sTemplatePath );
                aDestURL += pFilterEntry->maFilterName;
                aDestURL += OUString( sal_Unicode('/') );
                if( createDirectory( aDestURL ) )
                {
                    aDestURL += OUString( aSourceURL.GetName() );

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
        if( pFilterEntry->maType.getLength() == 0 )
        {
            pFilterEntry->maType = createUniqueTypeName( pNewInfo->maFilterName );
        }

        // update import/export flags
        if( pFilterEntry->maImportXSLT.getLength() )
        {
            pFilterEntry->maFlags |= 1;
        }
        else
        {
            pFilterEntry->maFlags &= ~1;
        }

        if( pFilterEntry->maExportXSLT.getLength() )
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
        catch( Exception& )
        {
            DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
            bOk = false;
        }
    }

    // 5. prepare type information
    if( bOk )
    {
        Sequence< PropertyValue > aValues(4);
        int i=0;

        aValues[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) );
        aValues[i++].Value <<= pFilterEntry->maInterfaceName;
/*
        aValues[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
        aValues[i++].Value <<= pFilterEntry->maDocType;
*/
        aValues[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ClipboardFormat" ) );
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

        aValues[i++].Value <<= aDocType;

        aValues[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentIconID" ) );
        aValues[i++].Value <<= pFilterEntry->mnDocumentIconID;

        aValues[i  ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Extensions" ) );
        aValues[i++].Value <<= createExtensionsSequence( pFilterEntry->maExtension );

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
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
                catch( Exception& )
                {
                    DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
                    bOk = false;
                }

                if( !bOk )
                {
                    // we failed to add the filter, so lets remove the type
                    try
                    {
                        mxTypeDetection->removeByName( pFilterEntry->maType );
                    }
                    catch( Exception& )
                    {
                        DBG_ERROR( "XMLFilterSettingsDialog::insertOrEdit exception catched!" );
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
                        OUString aName( aSequence[nIndex].Name );
                        if( aSequence[nIndex].Name.equalsAscii( "Types" ) )
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
                            if( pValues->Name.equalsAscii( "Type" ) )
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
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterSettingsDialog::onDelete exception catched!" );
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
       ::sfx2::FileDialogHelper aDlg( ::sfx2::FILESAVE_AUTOEXTENSION, 0 );

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
       ::sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );

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

        String aFilterName;
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
            USHORT          nKeyCode = aKeyCode.GetCode();

            if( nKeyCode == KEY_ESCAPE )
            {
                Close();
                return TRUE;
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
                    if( pValues->Name.equalsAscii( "Type" ) )
                    {
                        pValues->Value >>= pTempFilter->maType;
                    }
                    else if( pValues->Name.equalsAscii( "UIName" ) )
                    {
                        pValues->Value >>= pTempFilter->maInterfaceName;
                    }
                    else if( pValues->Name.equalsAscii( "DocumentService" ) )
                    {
                        pValues->Value >>= pTempFilter->maDocumentService;
                    }
                    else if( pValues->Name.equalsAscii( "FilterService" ) )
                    {
                        pValues->Value >>= aFilterService;
                    }
                    else if( pValues->Name.equalsAscii( "Flags" ) )
                    {
                        pValues->Value >>= pTempFilter->maFlags;
                    }
                    else if( pValues->Name.equalsAscii( "UserData" ) )
                    {
                        pValues->Value >>= aUserData;
                    }
                    else if( pValues->Name.equalsAscii( "FileFormatVersion" ) )
                    {
                        pValues->Value >>= pTempFilter->maFileFormatVersion;
                    }
                    else if( pValues->Name.equalsAscii( "TemplateName" ) )
                    {
                        pValues->Value >>= pTempFilter->maImportTemplate;
                    }
                }

                // if this is not a XmlFilterAdaptor entry, skip it
                if( !aFilterService.equalsAscii( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) )
                    continue;


                // if we don't have the needed user data, skip it
                if( aUserData.getLength() < 6 )
                    continue;

                // if this is not an XSLTFilter entry, skip it
                if( !aUserData[0].equalsAscii( "com.sun.star.documentconversion.XSLTFilter" ) )
                    continue;

                // get filter information from userdata
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
                        Any aAny( mxTypeDetection->getByName( pTempFilter->maType ) );
                        Sequence< PropertyValue > aValues;

                        if( aAny >>= aValues )
                        {
                            const sal_Int32 nValueCount( aValues.getLength() );
                            PropertyValue* pValues = aValues.getArray();
                            sal_Int32 nValue;

                            for( nValue = 0; nValue < nValueCount; nValue++, pValues++ )
                            {
/*
                                if( pValues->Name.equalsAscii( "MediaType" ) )
                                {
                                    pValues->Value >>= pTempFilter->maDocType;
                                } else
*/
                                if( pValues->Name.equalsAscii( "ClipboardFormat" ) )
                                {
                                    OUString aDocType;
                                    pValues->Value >>= aDocType;

                                    if( aDocType.match( sDocTypePrefix ) )
                                        aDocType = aDocType.copy( sDocTypePrefix.getLength() );

                                    pTempFilter->maDocType = aDocType;
                                }
                                else if( pValues->Name.equalsAscii( "Extensions" ) )
                                {
                                    Sequence< OUString > aExtensions;
                                    if( pValues->Value >>= aExtensions )
                                    {
                                        pTempFilter->maExtension = OUString();

                                        const sal_Int32 nCount( aExtensions.getLength() );
                                        OUString* pExtensions = aExtensions.getArray();
                                        sal_Int32 n;
                                        for( n = 0; n < nCount; n++ )
                                        {
                                            if( n > 0 )
                                                pTempFilter->maExtension += OUString( sal_Unicode(';') );
                                            pTempFilter->maExtension += (*pExtensions++);
                                        }
                                    }
                                }
                                else if( pValues->Name.equalsAscii( "DocumentIconID" ) )
                                {
                                    pValues->Value >>= pTempFilter->mnDocumentIconID;
                                }
                            }
                        }
                    }
                    catch( ::com::sun::star::container::NoSuchElementException& )
                    {
                        DBG_ERROR( "Type not found, user error?" ); // TODO: error?
                    }
                }

                // add entry to internal container and to ui filter list box
                maFilterVector.push_back( pTempFilter );
                mpFilterListBox->addFilterEntry( pTempFilter );


                pTempFilter = new filter_info_impl;
            }
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterSettingsDialog::initFilterList exception catched!" );
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
        aInfos.push_back( new application_info_impl(
            "com.sun.star.text.TextDocument",
            ResId( STR_APPL_NAME_WRITER, getXSLTDialogResMgr() ),
            "com.sun.star.comp.Writer.XMLImporter",
            "com.sun.star.comp.Writer.XMLExporter" ) );

        aInfos.push_back( new application_info_impl(
            "com.sun.star.sheet.SpreadsheetDocument",
            ResId( STR_APPL_NAME_CALC, getXSLTDialogResMgr() ),
            "com.sun.star.comp.Calc.XMLImporter",
            "com.sun.star.comp.Calc.XMLExporter" ) );

        aInfos.push_back( new application_info_impl(
            "sun.star.presentation.PresentationDocument",
            ResId( STR_APPL_NAME_IMPRESS, getXSLTDialogResMgr() ),
            "com.sun.star.comp.Impress.XMLImporter",
            "com.sun.star.comp.Impress.XMLExporter" ) );

        aInfos.push_back( new application_info_impl(
            "com.sun.star.drawing.DrawingDocument",
            ResId( STR_APPL_NAME_DRAW, getXSLTDialogResMgr() ),
            "com.sun.star.comp.Draw.XMLImporter",
            "com.sun.star.comp.Draw.XMLExporter" ) );
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
        if( rServiceName == (*aIter)->maDocumentService )
        {
            return (*aIter);
        }
        aIter++;
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
        OUString aRet( String( RESID( STR_UNKNOWN_APPLICATION ) ) );
        if( rServiceName.getLength() )
        {
            aRet += OUString::createFromAscii(" (");
            aRet += rServiceName;
            aRet += OUString::createFromAscii(")");
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

    WinBits nBits = WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP;
    pParent->SetFocusControl( this );
    SetWindowBits( nBits );
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

IMPL_LINK( XMLFilterListBox, TabBoxScrollHdl_Impl, SvTabListBox*, pList )
{
    mpHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterListBox, HeaderSelect_Impl, HeaderBar*, pBar )
{
    if ( pBar && pBar->GetCurItemId() != ITEMID_NAME )
        return 0;

    HeaderBarItemBits nBits = mpHeaderBar->GetItemBits(ITEMID_TYPE);
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    SvSortMode eMode = SortAscending;

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
        eMode = SortDescending;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }
    mpHeaderBar->SetItemBits( ITEMID_NAME, nBits );
    SvTreeList* pModel = GetModel();
    pModel->SetSortMode( eMode );
    pModel->Resort();
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterListBox, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( pBar && !pBar->GetCurItemId() )
        return 0;

    if ( !mpHeaderBar->IsItemMode() )
    {
        Size aSz;
        USHORT nTabs = mpHeaderBar->GetItemCount();
        long nTmpSz = 0;
        long nWidth = mpHeaderBar->GetItemSize(ITEMID_NAME);
        long nBarWidth = mpHeaderBar->GetSizePixel().Width();

        if(nWidth < 30)
            mpHeaderBar->SetItemSize( ITEMID_TYPE, 30);
        else if ( ( nBarWidth - nWidth ) < 30 )
            mpHeaderBar->SetItemSize( ITEMID_TYPE, nBarWidth - 30 );

        for ( USHORT i = 1; i <= nTabs; ++i )
        {
            long nWidth = mpHeaderBar->GetItemSize(i);
            aSz.Width() =  nWidth + nTmpSz;
            nTmpSz += nWidth;
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
    InsertEntry( aEntryStr, LIST_APPEND, 0xffff, (void*)pInfo );
}

// -----------------------------------------------------------------------

void XMLFilterListBox::changeEntry( const filter_info_impl* pInfo )
{
    const ULONG nCount = GetEntryCount();
    ULONG nPos;
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
    aEntryStr += String( getApplicationUIName( pInfo->maDocumentService ) );
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
    mnDocumentIconID(0)
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
    maDocType( rInfo.maDocType ),
    maDTD( rInfo.maDTD ),
    maExportXSLT( rInfo.maExportXSLT ),
    maImportXSLT( rInfo.maImportXSLT ),
    maImportTemplate( rInfo.maImportTemplate ),
    maFlags( rInfo.maFlags ),
    maFileFormatVersion( rInfo.maFileFormatVersion ),
    mnDocumentIconID( rInfo.mnDocumentIconID )
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
        maImportTemplate != r.maImportTemplate ||
        maFlags != r.maFlags ||
        maFileFormatVersion != r.maFileFormatVersion )
        return false;

    return true;
}

// -----------------------------------------------------------------------

Sequence< OUString > filter_info_impl::getFilterUserData() const
{
    Sequence< OUString > aUserData(8);

    aUserData[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.documentconversion.XSLTFilter" ) );
    const application_info_impl* pInfo = getApplicationInfo( maDocumentService );
    if( pInfo )
    {
        aUserData[2] = pInfo->maXMLImporter;
        aUserData[3] = pInfo->maXMLExporter;
    }
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

OUString xmlname_encode( const OUString & rText )
{

    static sal_Bool const aCharClass[]
    =  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* UricNoSlash */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  /*pqrstuvwxyz{|}~ */
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
    return rURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM("http:") ) == 0;
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
    catch(Exception&)
    {
        DBG_ERROR( "copyStreams() exception catched!" );
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