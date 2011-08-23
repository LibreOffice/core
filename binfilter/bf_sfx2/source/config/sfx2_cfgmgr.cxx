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

// include ---------------------------------------------------------------

#include <stdio.h>

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif

#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <sot/storinfo.hxx>

#ifndef _SVSTDARR_ULONGS_DECL
#define _SVSTDARR_ULONGS
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "cfgmgr.hxx"
#include "objsh.hxx"
#include "cfgimpl.hxx"
#include "docfile.hxx"
#include "evntconf.hxx"

namespace binfilter {

using namespace ::com::sun::star;

static const char pStorageName[] = "Configurations";

/*N*/ DBG_NAME(SfxConfigManager)

// ----------------------------------------------------------------------------

/*N*/ SfxConfigManager::SfxConfigManager( SotStorage* pStorage )
/*N*/ 	: pObjShell( NULL )
/*N*/ 	, nErrno( ERR_NO )
/*N*/ 	, bModified( FALSE )
/*N*/ {
/*N*/ 	DBG_CTOR(SfxConfigManager, 0);
/*N*/
/*N*/     // configuration stored in any storage
/*N*/     pItemArr = new SfxConfigItemArr_Impl;
/*N*/
/*N*/ 	if ( !pStorage )
/*N*/ 	{
/*N*/ 		// default config manager
/*N*/         pStorage = new SotStorage( String() );
/*N*/ 	}
/*N*/
/*N*/     if ( !pStorage->IsOLEStorage() )
/*N*/ 	{
/*N*/ 		// take the storage ( otherwise no configitems could be loaded )
/*N*/ 		m_xStorage = pStorage;
/*N*/
/*N*/         // any other configuration storage or document in 6.0 format
/*N*/         // read directory information
/*N*/         if ( !LoadConfiguration( *pStorage ) )
/*N*/             nErrno = ERR_READ;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         // any other configuration storage or document in 5.0 format
/*?*/         // create temporary storage for the converted data in 6.0 format
/*?*/         m_xStorage = new SotStorage( TRUE, String(), STREAM_STD_READWRITE, STORAGE_TRANSACTED );
/*?*/
/*?*/         // convert 5.0 configuration into own 6.0 storage
/*?*/ 		SfxConfigManagerImExport_Impl aImporter( pObjShell, pItemArr );
/*?*/         nErrno = aImporter.Import( pStorage, m_xStorage );
/*N*/ 	}
/*N*/ }

/*N*/ SfxConfigManager::SfxConfigManager( SfxObjectShell& rShell )
/*N*/     : pObjShell( &rShell )
/*N*/ 	, nErrno( ERR_NO )
/*N*/ 	, bModified( FALSE )
/*N*/ {
/*N*/ 	DBG_CTOR(SfxConfigManager, 0);
/*N*/
/*N*/     // configuration stored in a document or template
/*N*/     pItemArr = new SfxConfigItemArr_Impl;
/*N*/
/*N*/     // create temporary storage
/*N*/     m_xStorage = new SotStorage( TRUE, String(), STREAM_STD_READWRITE, STORAGE_TRANSACTED );
/*N*/
/*N*/     SotStorageRef xStorage = pObjShell->GetStorage();
/*N*/     if ( !xStorage->IsOLEStorage() )
/*N*/ 	{
/*?*/ 		// use the configuration substorage of the document
/*?*/ 		SotStorageRef xCfgStorage = xStorage->OpenSotStorage( String::CreateFromAscii(pStorageName), STREAM_STD_READWRITE );
/*?*/
/*?*/ 		// copy data, using the original storage is only possible for SvPersist objects
/*?*/ 		// the retransfer of the data is organized by the document
/*?*/ 		xCfgStorage->CopyTo( m_xStorage );
/*?*/
/*?*/         // read directory information
/*?*/         if ( !LoadConfiguration( *m_xStorage ) )
/*?*/             nErrno = ERR_READ;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         // convert 5.0 configuration into own 6.0 storage
/*N*/ 		SfxConfigManagerImExport_Impl aImporter( pObjShell, pItemArr );
/*N*/         nErrno = aImporter.Import( xStorage, m_xStorage );
/*N*/ 	}
/*N*/
/*N*/     rShell.SetConfigManager( this );
/*N*/ }

/*N*/ SfxConfigManager::~SfxConfigManager()
/*N*/ {
/*N*/     for( USHORT i = 0; i < pItemArr->Count(); ++i )
/*N*/     {
/*N*/         SfxConfigItem_Impl* pItem = (*pItemArr)[i];
/*N*/         if ( pItem->pCItem )
/*N*/         {
/*?*/             DBG_ERROR("SfxConfigItem not removed!");
/*?*/             pItem->pCItem->ReleaseConfigManager();
/*N*/         }
/*N*/ 		delete pItem;
/*N*/     }
/*N*/
/*N*/     delete pItemArr;
/*N*/ }

// ------------------------------------------------------------------------

/*?*/ SotStorage* SfxConfigManager::GetConfigurationStorage( SotStorage* pDocStor )
/*?*/ {
/*?*/ 	return pDocStor->OpenSotStorage( String::CreateFromAscii(pStorageName), STREAM_STD_READWRITE );
/*?*/ }

/*?*/ void SfxConfigManager::SetModified(BOOL bMod)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/ }

/*N*/ BOOL SfxConfigManager::HasConfiguration( SotStorage& rStorage )
/*N*/ {
/*N*/     if ( rStorage.IsOLEStorage() )
/*N*/ 	{
/*N*/         return SfxConfigManagerImExport_Impl::HasConfiguration( rStorage );
/*N*/     }
/*N*/     else
/*N*/     {
/*?*/         return rStorage.IsStorage( String::CreateFromAscii(pStorageName) );
/*N*/     }
/*N*/ }

// ----------------------------------------------------------------------------

/*N*/ BOOL SfxConfigManager::LoadConfiguration( SotStorage& rStorage )
/*N*/ {
/*N*/     // for the case that there are several layers of configuration
/*N*/     USHORT nOldCount = pItemArr->Count();
/*N*/
/*N*/     SvStorageInfoList aList;
/*N*/     rStorage.FillInfoList( &aList );
/*N*/     for( USHORT i = 0; i < aList.Count(); i++ )
/*N*/     {
/*?*/         SvStorageInfo& rInfo = aList.GetObject( i );
/*?*/         if ( rInfo.IsStream() )
/*?*/         {
/*?*/             // get StreamName and ItemType
/*?*/             SfxConfigItem_Impl* pItem = NULL;
/*?*/             String aStreamName = rInfo.GetName();
/*?*/             USHORT nType = SfxConfigManagerImExport_Impl::GetType( aStreamName );
/*?*/
/*?*/             // check items from upper layers if one of them is of the same type
/*?*/             for ( USHORT nPos=0; nPos<nOldCount; nPos++ )
/*?*/             {
/*?*/                 if ( (*pItemArr)[nPos]->nType == nType )
/*?*/                 {
/*?*/                     pItem = (*pItemArr)[nPos];
/*?*/                     break;
/*?*/                 }
/*?*/             }
/*?*/
/*?*/             if ( !pItem )
/*?*/             {
/*?*/                 // new item
/*?*/                 pItem = new SfxConfigItem_Impl;
/*?*/                 pItemArr->Insert( pItem, pItemArr->Count() );
/*?*/                 pItem->aStreamName = aStreamName;
/*?*/                 pItem->nType = nType;
/*?*/
/*?*/                 // compatibility for 5.0 format
/*?*/                 pItem->bDefault = FALSE;
/*?*/             }
/*?*/
/*?*/             // every item will hold a reference to its storage
/*?*/             pItem->xStorage = &rStorage;
/*?*/         }
/*N*/     }
/*N*/
/*N*/     return TRUE;
/*N*/ }

/*N*/ BOOL SfxConfigManager::StoreConfiguration( SotStorage* pStorage )
/*N*/ {
/*N*/     // What about the Modified flag ?! ( see also appmisc, config dialog, objcont! )
/*N*/     BOOL bOwnSaveDone = FALSE;
/*N*/     BOOL bRet = TRUE;
/*N*/     if ( m_xStorage.Is() )
/*N*/     {
/*N*/         // first update own storage
/*N*/         bRet = !bModified || StoreConfiguration_Impl( m_xStorage ) && m_xStorage->Commit();
/*N*/         bOwnSaveDone = TRUE;
/*N*/         if ( !pStorage && pObjShell )
/*N*/         {
/*?*/             // pStorage == NULL means : storage of document should be updated also
/*?*/             SotStorage* pDocumentStorage = pObjShell->GetStorage();
/*?*/             if ( !pDocumentStorage->IsOLEStorage() )
/*?*/             {
/*?*/                 // use the configuration substorage of the document
/*?*/                 SotStorageRef xCfgStorage = pDocumentStorage->OpenSotStorage(
/*?*/                         String::CreateFromAscii(pStorageName), STREAM_STD_READWRITE, STORAGE_TRANSACTED );
/*?*/                 bRet = m_xStorage->CopyTo( xCfgStorage ) && xCfgStorage->Commit();
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 // 5.0 format : store compact configuration stream
/*?*/                 SfxConfigManagerImExport_Impl aExporter( pObjShell, pItemArr );
/*?*/                 nErrno = aExporter.Export( m_xStorage, pDocumentStorage );
/*?*/                 bRet = ( nErrno == ERR_NO );
/*?*/             }
/*?*/
/*?*/             if ( bRet )
/*?*/             {
/*?*/                 // can't commit changes if the documents' storage is under control of the document
/*?*/                 if( pObjShell->GetCreateMode() == SFX_CREATE_MODE_ORGANIZER )
/*?*/                     bRet = pDocumentStorage->Commit();
/*?*/                 else
/*?*/                 {
/*?*/ 					// The configuration will be stored together the document, so the modified flag
/*?*/ 					// must remain set!
/*?*/                     return TRUE;
/*?*/                 }
/*?*/             }
/*N*/         }
/*N*/
/*N*/         if ( bRet && !pStorage || pStorage == (SotStorage*) m_xStorage )
/*N*/         {
/*N*/             // only storing into own storage was requested
/*N*/             bModified = FALSE;
/*N*/             return TRUE;
/*N*/         }
/*N*/     }
/*N*/ 	else
/*?*/ 		DBG_ASSERT( pStorage, "Can't save configuration!" );
/*?*/
/*?*/     if ( !bRet || !pStorage )
/*?*/ 		return FALSE;
/*?*/
/*?*/     // store also into storage passed as parameter, but don't commit the changes,  because this will be done by the caller
/*?*/     if ( !pStorage->IsOLEStorage() )
/*?*/ 	{
/*?*/         // 6.0 format
/*?*/         if ( bOwnSaveDone )
/*?*/         {
/*?*/             // if own storage is updated, just copy it to the destination storage
/*?*/             bRet = m_xStorage->CopyTo( pStorage );
/*?*/         }
/*?*/         else
/*?*/         {
/*?*/             bRet = StoreConfiguration_Impl( pStorage );
/*?*/         }
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/         // 5.0 format : store compact configuration stream
/*?*/ 		SfxConfigManagerImExport_Impl aExporter( pObjShell, pItemArr );
/*?*/         nErrno = aExporter.Export( m_xStorage, pStorage );
/*?*/         bRet = ( nErrno == ERR_NO );
/*?*/ 	}
/*?*/
/*?*/ 	bModified = !bRet;
/*?*/     return bRet;
/*N*/ }

/*?*/ BOOL SfxConfigManager::StoreConfiguration_Impl( SotStorage* pStorage )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/     BOOL bRet = TRUE;
/*?*/     return bRet;
/*?*/ }

// ----------------------------------------------------------------------------

/*N*/ void SfxConfigManager::AddConfigItem( SfxConfigItem& rCItem )
/*N*/ {
/*N*/     for( USHORT i = 0; i < pItemArr->Count(); ++i )
/*N*/     {
/*N*/         SfxConfigItem_Impl* pItem = (*pItemArr)[i];
/*N*/         if ( pItem->nType == rCItem.GetType() )
/*N*/         {
/*N*/             if ( pItem->pCItem )
/*N*/             {
/*N*/                 SfxConfigItems_Impl& rItems = pItem->aItems;
/*N*/                 USHORT nCount = rItems.Count();
/*N*/ #ifdef DBG_UTIL
/*N*/                 for ( USHORT nItem=0; nItem<nCount; nItem++ )
/*N*/                     if ( rItems[nItem] == &rCItem )
/*N*/                         DBG_ERROR("Item already inserted!");
/*N*/ #endif
/*N*/                 rItems.Insert( &rCItem, nCount );
/*N*/             }
/*N*/             else
/*N*/                 pItem->pCItem = &rCItem;
/*N*/             return;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     SfxConfigItem_Impl* pItem = new SfxConfigItem_Impl( &rCItem );
/*N*/     pItemArr->Insert( pItem, pItemArr->Count() );
/*N*/     pItem->bDefault = rCItem.IsDefault();
/*N*/     pItem->aStreamName = SfxConfigManagerImExport_Impl::GetStreamName( pItem->nType );
/*N*/ }

/*N*/ void SfxConfigManager::RemoveConfigItem( SfxConfigItem& rCItem )
/*N*/ {
/*N*/     for( USHORT i = 0; i < pItemArr->Count(); ++i )
/*N*/     {
/*N*/         SfxConfigItem_Impl* pItem = (*pItemArr)[i];
/*N*/         if ( pItem->nType == rCItem.GetType() )
/*N*/         {
/*N*/ 			if ( pItem->pCItem == &rCItem )
/*N*/ 			{
/*N*/             	if ( pItem->aItems.Count() )
/*N*/             	{
/*?*/                 	pItem->pCItem = pItem->aItems[0];
/*?*/                 	pItem->aItems.Remove( 0 );
/*N*/             	}
/*N*/             	else
/*N*/                 	pItem->pCItem = NULL;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/                 USHORT nCount = pItem->aItems.Count();
/*N*/                 for ( USHORT nItem=0; nItem<nCount; nItem++ )
/*N*/                 	if ( pItem->aItems[nItem] == &rCItem )
/*N*/ 					{
/*N*/ 						pItem->aItems.Remove(nItem);
/*N*/ 						break;
/*N*/ 					}
/*N*/ 			}
/*N*/
/*N*/             return;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     DBG_ERROR( "Item not registered!" );
/*N*/ }

/*N*/ BOOL SfxConfigManager::LoadConfigItem( SfxConfigItem& rCItem )
/*N*/ {
/*N*/     DBG_ASSERT( m_xStorage.Is(), "No storage for configurations!" );
/*N*/
/*N*/     // search for item
/*N*/     for( USHORT i = 0; i < pItemArr->Count(); ++i )
/*N*/     {
/*N*/         SfxConfigItem_Impl* pItem = (*pItemArr)[i];
/*N*/         if ( pItem->nType == rCItem.GetType() )
/*N*/         {
/*N*/             // update configuration befor using it
/*N*/             // find last modified item
/*N*/             SfxConfigItem* pCfgItem = NULL;
/*N*/             if ( pItem->pCItem && pItem->pCItem->IsModified() )
/*N*/                 pCfgItem = pItem->pCItem;
/*N*/
/*N*/             USHORT nCount = pItem->aItems.Count();
/*N*/             for ( USHORT nItem=0; nItem<nCount; nItem++ )
/*N*/                 if ( pItem->aItems[nItem]->IsModified() )
/*N*/                     pCfgItem = pItem->aItems[nItem];
/*N*/
/*N*/             if ( pCfgItem )
/*?*/                 pCfgItem->StoreConfig();
/*N*/
/*N*/             if ( pItem->bDefault )
/*N*/             {
/*N*/                 // no configuration in storage
/*N*/                 rCItem.UseDefault();
/*N*/             }
/*N*/             else if ( pItem->xStorage.Is() )
/*N*/             {
/*N*/                 // item configuration is in upper layer of configuration
/*N*/                 if ( rCItem.Load( *pItem->xStorage ) != SfxConfigItem::ERR_OK )
/*N*/ 					rCItem.UseDefault();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 DBG_ERROR("Item without Storage!");
/*N*/                 rCItem.UseDefault();
/*N*/                 return FALSE;
/*N*/             }
/*N*/
/*N*/             return TRUE;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     DBG_ERROR("ItemType not registered!");
/*N*/     return FALSE;
/*N*/ }

/*?*/ BOOL SfxConfigManager::StoreConfigItem( SfxConfigItem& rCItem )
/*?*/ {DBG_BF_ASSERT(0, "STRIP");return FALSE;//STRIP001
/*?*/ }

/*N*/ static const char pHeader[] = "Star Framework Config File";
/*N*/ #define CFG_STREAM_BUFFER_SIZE  5000
/*N*/ static const USHORT nVersion = 26;
/*N*/ static const char cCtrlZ = 26;
/*N*/ 
/*N*/ static const USHORT nTypesCount = 82;

/*N*/ BOOL SfxConfigManagerImExport_Impl::HasConfiguration( SotStorage& rStorage )
/*N*/ {
/*N*/     return rStorage.IsStream( String::CreateFromAscii(pHeader) );
/*N*/ }

/*N*/ USHORT SfxConfigManagerImExport_Impl::Import( SotStorage* pInStorage, SotStorage* pStorage )
/*N*/ {
/*N*/ 	SotStorageStreamRef aStr =
/*N*/         pInStorage->OpenSotStream( String::CreateFromAscii(pHeader), STREAM_STD_READ );
/*N*/ 	if ( aStr->GetError() )
/*N*/ 		return SfxConfigManager::ERR_OPEN;
/*N*/ 
/*N*/ 	SvStream* pStream = aStr;
/*N*/ 	pStream->SetVersion( SOFFICE_FILEFORMAT_40 );
/*N*/ 	pStream->SetBufferSize( CFG_STREAM_BUFFER_SIZE );
/*N*/ 	pStream->Seek(0);
/*N*/ 	USHORT nRet = SfxConfigManager::ERR_NO;
/*N*/ 
/*N*/ 	// check header
/*N*/ 	const unsigned nLen = strlen( pHeader );
/*N*/ 	char *pBuf = new char[nLen+1];
/*N*/ 	pStream->Read( pBuf, nLen );
/*N*/ 	pBuf[nLen] = 0;
/*N*/ 	if( strcmp( pBuf, pHeader ) )
/*N*/ 	{
/*N*/ 		delete[] pBuf;
/*?*/ 		return SfxConfigManager::ERR_FILETYPE;
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] pBuf;
/*N*/ 
/*N*/ 	// compare version
/*N*/ 	char c;
/*N*/ 	USHORT nFileVersion;
/*N*/ 	(*pStream) >> c;					// skip ASCII 26 (CTRL-Z)
/*N*/ 	(*pStream) >> nFileVersion;
/*N*/ 	if( nFileVersion != nVersion )
/*N*/ 	{
/*?*/ 		return SfxConfigManager::ERR_VERSION;
/*N*/ 	}
/*N*/ 
/*N*/ 	// get position of directory
/*N*/ 	long lDirPos, lStart = pStream->Tell();
/*N*/ 	(*pStream) >> lDirPos;
/*N*/ 	pStream->Seek(lDirPos);
/*N*/ 
/*N*/ 	// get number of items
/*N*/ 	USHORT nCount;
/*N*/ 	(*pStream) >> nCount;
/*N*/ 
/*N*/ 	// read in directory and convert items
/*N*/ 	for( USHORT i=0; i < nCount; ++i )
/*N*/ 	{
/*N*/ 		SfxConfigItem_Impl* pItem = new SfxConfigItem_Impl;
/*N*/ 		pItemArr->Insert( pItem, pItemArr->Count() );
/*N*/ 
/*N*/ 		// retrieve type of item and human readable name
/*N*/ 		long lLength, lPos;	            // dummies
/*N*/         (*pStream) >> pItem->nType >> lPos >> lLength;
/*N*/ 		pItem->bDefault = ( lPos == -1L );
/*N*/         pStream->ReadByteString( pItem->aName );
/*N*/         if ( pStream->GetError() )
/*N*/         {
/*?*/             pItem->bDefault = TRUE;
/*?*/             return SfxConfigManager::ERR_READ;
/*N*/         }
/*N*/ 
/*N*/ 		// convert into new format
/*N*/         if ( !pItem->bDefault )
/*N*/         {
/*N*/             // stream name for new format
/*N*/             pItem->aStreamName = GetStreamName( pItem->nType );
/*N*/             if ( pItem->aStreamName.Len() )
/*N*/             {
/*N*/                 long lOldPos = pStream->Tell();
/*N*/                 pStream->Seek( lPos );
/*N*/ 
/*N*/                 // check for correct type id, inequality is allowed for userdef toolboxes
/*N*/                 USHORT nType;
/*N*/                 (*pStream) >> nType;
/*N*/                 BOOL bOk = ( nType == pItem->nType ||
/*N*/                             1294 <= nType && nType <= 1301 &&
/*N*/                             1294 <= pItem->nType && pItem->nType <= 1301 );
/*N*/ 
/*N*/                 if ( !bOk || !ImportItem( pItem, pStream, pStorage ) )
/*N*/                 {
/*?*/                     pItem->bDefault = TRUE;
/*?*/                     nRet = SfxConfigManager::ERR_IMPORT;
/*N*/                 }
/*N*/ 
/*N*/                 pStream->Seek( lOldPos );
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 DBG_ERROR("Couldn't convert old configuration!");
/*N*/                 // force error message that saving this document would lose some configuration information
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         if ( !pItem->aStreamName.Len() )
/*N*/ 		{
/*N*/             // no real config item anymore
/*N*/ 			delete pItem;
/*N*/ 			pItemArr->Remove( pItemArr->Count() - 1 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nRet;
/*N*/ }

/*N*/ BOOL SfxConfigManagerImExport_Impl::ImportItem( SfxConfigItem_Impl* pItem, SvStream* pStream, SotStorage* pStor )
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	if ( pItem->nType == SFX_ITEMTYPE_DOCEVENTCONFIG )
        {
            BOOL bOK = SfxEventConfiguration::Import( *pStream, NULL, pObjShell );
             if ( bRet )
                 bRet = bOK;
        }

         return bRet;
/*N*/ }

/*?*/ USHORT SfxConfigManagerImExport_Impl::Export( SotStorage* pStor, SotStorage *pOut )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }

/*N*/ String SfxConfigManagerImExport_Impl::GetStreamName( USHORT nType )
/*N*/ {
/*N*/         if ( 11 == nType )
/*N*/             return String::CreateFromAscii( "eventbindings.xml" );
/*?*/ 
/*?*/     return String();
/*N*/ }

/*?*/ USHORT SfxConfigManagerImExport_Impl::GetType( const String& rStreamName )
/*?*/ {
/*NBFF*/     ByteString aCmp( rStreamName, RTL_TEXTENCODING_ASCII_US );
/*NBFF*/ 
/*NBFF*/         if ( !strcmp( aCmp.GetBuffer(), "eventbindings.xml" ) )
/*NBFF*/             return 11;
/*NBFF*/ 
/*?*/     return 0;
/*?*/ }

}
