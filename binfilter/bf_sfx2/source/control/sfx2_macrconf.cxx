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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "macrconf.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "objshimp.hxx"

namespace binfilter {

static const sal_uInt16 nCompatVersion = 2;
static const sal_uInt16 nVersion = 3;

// Static member
SfxMacroConfig* SfxMacroConfig::pMacroConfig = NULL;

/*N*/ void SfxMacroConfig::Release_Impl()
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
/*N*/ 	DELETEZ( pMacroConfig );
/*N*/ }

//==========================================================================

/*?*/ struct SfxMacroConfig_Impl
/*?*/ {
/*?*/ 	SfxMacroInfoArr_Impl    aArr;
/*?*/ 	sal_uInt32					nEventId;
/*?*/ 	sal_Bool					bWaitingForCallback;
/*?*/ 
/*?*/ 							SfxMacroConfig_Impl()
/*?*/ 							: nEventId( 0 )
/*?*/ 							, bWaitingForCallback( sal_False )
/*?*/ 							{}
/*?*/ };

//==========================================================================

/*?*/ SfxMacroInfo::SfxMacroInfo( SfxObjectShell *pDoc ) :
/*?*/ 	bAppBasic(pDoc == NULL),
/*?*/ 	nSlotId(0),
/*?*/ 	nRefCnt(0),
/*?*/ 	pHelpText(0)
/*?*/ {}

//==========================================================================

/*?*/ SfxMacroInfo::~SfxMacroInfo()
/*?*/ {
/*?*/ 	delete pHelpText;
/*?*/ }

//==========================================================================

/*?*/ sal_Bool SfxMacroInfo::operator==(const SfxMacroInfo& rOther) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*?*/ }

//==========================================================================

/*?*/ String SfxMacroInfo::GetQualifiedName() const
/*?*/ {DBG_BF_ASSERT(0, "STRIP");return String();
/*?*/ }

//==========================================================================

/*?*/ String SfxMacroInfo::GetBasicName() const
/*?*/ {DBG_BF_ASSERT(0, "STRIP");return String();//STRIP001 
/*?*/ }

//==========================================================================

/*?*/ SvStream& operator >> (SvStream& rStream, SfxMacroInfo& rInfo)
/*?*/ {
/*?*/ 	sal_uInt16 nAppBasic, nFileVersion;
/*?*/ 	String aDocName;
/*?*/ 
/*?*/ 	rStream >> nFileVersion;
/*?*/ 	if ( nVersion < nCompatVersion )
/*?*/ 	{
/*?*/ 		// In der 1.Version ohne Versionskennung
/*?*/ 		nAppBasic = nVersion;
/*?*/ 		nFileVersion = 1;
/*?*/ 		rStream.ReadByteString(aDocName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.ReadByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.ReadByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.ReadByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		String aInput;
/*?*/ 		rStream	>> nAppBasic;
/*?*/ 		rStream.ReadByteString(aDocName,RTL_TEXTENCODING_UTF8);					// Vorsicht: kann bei AppName Unsinn sein!
/*?*/ 		rStream.ReadByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.ReadByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.ReadByteString(aInput,RTL_TEXTENCODING_UTF8);
/*?*/ 
/*?*/ 		if ( nFileVersion == nCompatVersion )
/*?*/ 			rInfo.aMethodName = aInput;
/*?*/ 		else
/*?*/ 		{
/*?*/ 			sal_uInt16 nCount = aInput.GetTokenCount('.');
/*?*/ 			rInfo.aMethodName = aInput.GetToken( nCount-1, '.' );
/*?*/ 			if ( nCount > 1 )
/*?*/ 				rInfo.aModuleName = aInput.GetToken( nCount-2, '.' );
/*?*/ 			if ( nCount > 2 )
/*?*/ 				rInfo.aLibName = aInput.GetToken( 0, '.' );
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	rInfo.bAppBasic = (sal_Bool) nAppBasic;
/*?*/ 	return rStream;
/*?*/ }

//==========================================================================

/*?*/ SvStream& operator << (SvStream& rStream, const SfxMacroInfo& rInfo)
/*?*/ {
/*?*/ 	if ( rInfo.bAppBasic )
/*?*/ 	{
/*?*/ 		rStream << nVersion
/*?*/ 				<< (sal_uInt16) rInfo.bAppBasic;
/*?*/ 		rStream.WriteByteString(rInfo.GetBasicName(),RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		rStream << nVersion
/*?*/ 				<< (sal_uInt16) rInfo.bAppBasic;
/*?*/ 		rStream.WriteByteString(SFX_APP()->GetName(),RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
/*?*/ 		rStream.WriteByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
/*?*/ 	}
/*?*/ 
/*?*/ 	return rStream;
/*?*/ }

//==========================================================================

/*?*/ SfxMacroConfig::~SfxMacroConfig()
/*?*/ {
/*?*/ 	if ( pImp->nEventId )
/*?*/ 		Application::RemoveUserEvent( pImp->nEventId );
/*?*/ 	delete pImp;
/*?*/ }

//==========================================================================

/*?*/ IMPL_LINK( SfxMacroConfig, CallbackHdl_Impl, SfxMacroConfig*, pConfig )
/*?*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pImp->bWaitingForCallback = sal_False;
/*?*/ 	return 0;
/*?*/ }

/*?*/ IMPL_LINK( SfxMacroConfig, EventHdl_Impl, SfxMacroInfo*, pInfo )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 	pImp->nEventId = 0;
/*?*/ 	return 0;
/*?*/ }

/*?*/ ErrCode SfxMacroConfig::Call( SbxObject* pVCtrl,
/*?*/ 	const String& rCode, BasicManager* pMgr, SbxArray *pArgs, SbxValue *pRet )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*?*/ }
}
