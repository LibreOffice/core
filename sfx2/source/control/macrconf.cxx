/*************************************************************************
 *
 *  $RCSfile: macrconf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
#if SUPD<582
#ifndef _SFXSFXJS_HXX //autogen
#include <sfxjs.hxx>
#endif
#ifndef _SJIMPL2_HXX //autogen
#include <sj2/sjimpl2.hxx>
#endif
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SBX_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <basic/sbmeth.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _BASIC_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#include <osl/mutex.hxx>

#include <com/sun/star/script/XEngine.hpp>

#pragma hdrstop

#include "msgpool.hxx"
#include "macrconf.hxx"
#include "sfxbasic.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "minfitem.hxx"
#include "imgmgr.hxx"
#include "evntconf.hxx"
#include "docfile.hxx"
#include "genlink.hxx"
#include <viewfrm.hxx>
#include <appdata.hxx>

static const sal_uInt16 nCompatVersion = 2;
static const sal_uInt16 nVersion = 3;

// Static member
SfxMacroConfig* SfxMacroConfig::pMacroConfig = NULL;

SfxMacroConfig* SfxMacroConfig::GetOrCreate()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !pMacroConfig )
        pMacroConfig = new SfxMacroConfig;
    return pMacroConfig;
}

void SfxMacroConfig::Release_Impl()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    DELETEZ( pMacroConfig );
}

//==========================================================================

struct SfxMacroConfig_Impl
{
    SfxMacroInfoArr_Impl    aArr;
    sal_uInt32                  nEventId;
    sal_Bool                    bWaitingForCallback;

                            SfxMacroConfig_Impl()
                            : nEventId( 0 )
                            , bWaitingForCallback( sal_False )
                            {}
};

//==========================================================================

SbMethod* SfxQueryMacro_Impl( BasicManager* pMgr , const String& rMacro,
    const String &rLibName, const String& rModule )
{
    const International &rInter = Application::GetAppInternational();

    sal_uInt16 nLibCount = pMgr->GetLibCount();
    for ( sal_uInt16 nLib = 0; nLib < nLibCount; ++nLib )
    {
        if ( COMPARE_EQUAL == rInter.Compare( pMgr->GetLibName( nLib ), rLibName, INTN_COMPARE_IGNORECASE ) )
        {
            StarBASIC* pLib = pMgr->GetLib( nLib );
            if( !pLib )
            {
                pMgr->LoadLib( nLib );
                pLib = pMgr->GetLib( nLib );
            }

            if( pLib )
            {
                sal_uInt16 nModCount = pLib->GetModules()->Count();
                for( sal_uInt16 nMod = 0; nMod < nModCount; ++nMod )
                {
                    SbModule* pMod = (SbModule*)pLib->GetModules()->Get( nMod );
                    if ( pMod &&
                         COMPARE_EQUAL == rInter.Compare( pMod->GetName(), rModule, INTN_COMPARE_IGNORECASE ) )
                    {
                        SbMethod* pMethod =
                                (SbMethod*)pMod->Find( rMacro, SbxCLASS_METHOD );
                        if( pMethod )
                            return pMethod;
                    }
                }
            }
        }
    }
    return 0;
}

SbMethod* SfxQueryMacro( BasicManager* pMgr , const String& rMacro )
{
    sal_uInt16 nLast = 0;
    String aMacro = rMacro;
    String aLibName = aMacro.GetToken( 0, '.', nLast );
    String aModule = aMacro.GetToken( 0, '.', nLast );
    aMacro.Erase( 0, nLast );

    return SfxQueryMacro_Impl( pMgr, aMacro, aLibName, aModule );
}

ErrCode SfxCallMacro( BasicManager* pMgr, const String& rCode,
                      SbxArray *pArgs, SbxValue *pRet )
{
    ErrCode nErr;
    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();
    SbMethod* pMethod = SfxQueryMacro( pMgr, rCode );
    if ( pMethod )
    {
        if ( pArgs )
            pMethod->SetParameters( pArgs );
        nErr = pMethod->Call( pRet );
    }
    else
        nErr = ERRCODE_BASIC_PROC_UNDEFINED;

    pApp->LeaveBasicCall();
    return nErr;
}

//==========================================================================

SfxMacroInfo::SfxMacroInfo( SfxObjectShell *pDoc ) :
    bAppBasic(pDoc == NULL),
    nSlotId(0),
    nRefCnt(0),
    pSlot(0),
    pHelpText(0)
{}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(SfxObjectShell *pDoc, const String& rLibName,
        const String& rModuleName, const String& rMethodName) :
    aLibName(rLibName),
    aModuleName(rModuleName),
    aMethodName(rMethodName),
    nSlotId(0),
    nRefCnt(0),
    pSlot(0),
    pHelpText(0)
{
    bAppBasic = (pDoc == 0);
}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(SfxObjectShell *pDoc, const String& rQualifiedName )
:   nSlotId(0),
    nRefCnt(0),
    pSlot(0),
    pHelpText(0)
{
    sal_uInt16 nCount = rQualifiedName.GetTokenCount('.');
    aMethodName = rQualifiedName.GetToken( nCount-1, '.' );
    if ( nCount > 1 )
        aModuleName = rQualifiedName.GetToken( nCount-2, '.' );
    if ( nCount > 2 )
        aLibName = rQualifiedName.GetToken( 0, '.' );
    bAppBasic = (pDoc == 0);
}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(SfxMacroInfo& rOther) :
    bAppBasic(rOther.bAppBasic),
    aLibName(rOther.aLibName),
    aModuleName(rOther.aModuleName),
    aMethodName(rOther.aMethodName),
    nSlotId(rOther.nSlotId),
    pSlot(0),
    nRefCnt(0),
    pHelpText(0)
{}

//==========================================================================

SfxMacroInfo::~SfxMacroInfo()
{
    delete pSlot;
    delete pHelpText;
}

//==========================================================================

sal_Bool SfxMacroInfo::operator==(const SfxMacroInfo& rOther) const
{
    if ( GetQualifiedName() == rOther.GetQualifiedName() &&
            bAppBasic   == rOther.bAppBasic )
        return sal_True;
    else
        return sal_False;
}

//==========================================================================

String SfxMacroInfo::GetMacroName() const
{
    String aMacroName = aMethodName;
    aMacroName += '(';
    aMacroName += aLibName;
    aMacroName += '.';
    aMacroName += aModuleName;
    aMacroName += ')';
    return aMacroName;
}

//==========================================================================

String SfxMacroInfo::GetQualifiedName() const
{
    String aMacroName;
    if( aMacroName.Len() || aLibName.Len() )
    {
        // Altes Format
        aMacroName = aLibName;
        aMacroName += '.';
        aMacroName += aModuleName;
        aMacroName += '.';
    }

    // Wg. ::com::sun::star::script::JavaScript kein Zerlegen des Strings mehr
    aMacroName += aMethodName;
    return aMacroName;
}

String SfxMacroInfo::GetFullQualifiedName() const
{
    // Liefert nur Unsinn, wenn f"ur ein ::com::sun::star::script::JavaScript aufgerufen !
    String aRet;
    if ( bAppBasic )
        aRet = SFX_APP()->GetName();
    aRet += '.';
    aRet += GetQualifiedName();
    return aRet;
}

//==========================================================================

BasicManager* SfxMacroInfo::GetBasicManager() const
{
    if (bAppBasic)
    {
        return SFX_APP()->GetBasicManager();
    }
    else
    {
        SfxObjectShell *pDocShell = SfxObjectShell::Current();
        return pDocShell ? pDocShell->GetBasicManager() :
                       SFX_APP()->GetBasicManager();
    }
}

//==========================================================================

String SfxMacroInfo::GetBasicName() const
{
    if (bAppBasic)
    {
        return SFX_APP()->GetName();
    }
    else
    {
        SfxObjectShell *pDocShell = SfxObjectShell::Current();
        if ( pDocShell )
            return pDocShell->GetTitle();
        else
            return SFX_APP()->GetName();
    }
}

String SfxMacroInfo::GetHelpText() const
{
    if ( pHelpText )
        return *pHelpText;
    return String();
}

String SfxMacroConfig::RequestHelp( sal_uInt16 nId )
{
    SfxMacroInfo *pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo( nId );
    if ( !pInfo )
        return String();

    if ( !pInfo->pHelpText )
    {
        SbMethod *pMethod =
            SfxQueryMacro_Impl( pInfo->GetBasicManager(), pInfo->aMethodName,
                pInfo->aLibName, pInfo->aModuleName );
        if ( pMethod && pMethod->GetInfo() )
            pInfo->pHelpText = new String( pMethod->GetInfo()->GetComment() );
    }

    return pInfo->GetHelpText();
}

void SfxMacroInfo::SetHelpText( const String& rName )
{
    if ( !pHelpText )
        pHelpText = new String;
    *pHelpText = rName;
}

//==========================================================================

SvStream& operator >> (SvStream& rStream, SfxMacroInfo& rInfo)
{
    sal_uInt16 nAppBasic, nFileVersion;
    String aDocName;

    rStream >> nFileVersion;
    if ( nVersion < nCompatVersion )
    {
        // In der 1.Version ohne Versionskennung
        nAppBasic = nVersion;
        nFileVersion = 1;
        rStream.ReadByteString(aDocName,RTL_TEXTENCODING_UTF8);
        rStream.ReadByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
        rStream.ReadByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
        rStream.ReadByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
    }
    else
    {
        String aInput;
        rStream >> nAppBasic;
        rStream.ReadByteString(aDocName,RTL_TEXTENCODING_UTF8);                 // Vorsicht: kann bei AppName Unsinn sein!
        rStream.ReadByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
        rStream.ReadByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
        rStream.ReadByteString(aInput,RTL_TEXTENCODING_UTF8);

        if ( nFileVersion == nCompatVersion )
            rInfo.aMethodName = aInput;
        else
        {
            sal_uInt16 nCount = aInput.GetTokenCount('.');
            rInfo.aMethodName = aInput.GetToken( nCount-1, '.' );
            if ( nCount > 1 )
                rInfo.aModuleName = aInput.GetToken( nCount-2, '.' );
            if ( nCount > 2 )
                rInfo.aLibName = aInput.GetToken( 0, '.' );
        }
    }

    rInfo.bAppBasic = (sal_Bool) nAppBasic;
    rInfo.nSlotId = SFX_APP()->GetMacroConfig()->GetSlotId(&rInfo);
    return rStream;
}

//==========================================================================

SvStream& operator << (SvStream& rStream, const SfxMacroInfo& rInfo)
{
    if ( rInfo.bAppBasic )
    {
        rStream << nVersion
                << (sal_uInt16) rInfo.bAppBasic;
        rStream.WriteByteString(rInfo.GetBasicName(),RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
    }
    else
    {
        rStream << nVersion
                << (sal_uInt16) rInfo.bAppBasic;
        rStream.WriteByteString(SFX_APP()->GetName(),RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aLibName,RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aModuleName,RTL_TEXTENCODING_UTF8);
        rStream.WriteByteString(rInfo.aMethodName,RTL_TEXTENCODING_UTF8);
    }

    return rStream;
}

sal_Bool SfxMacroInfo::Compare( const SvxMacro& rMacro ) const
{
    String aName = rMacro.GetLibName();
    aName += '.';
    aName += rMacro.GetMacName();
    if ( GetFullQualifiedName() == aName )
        return sal_True;
    return sal_False;
}

//==========================================================================

SfxMacroConfig::SfxMacroConfig()
{
    pImp = new SfxMacroConfig_Impl;
}

//==========================================================================

SfxMacroConfig::~SfxMacroConfig()
{
    if ( pImp->nEventId )
        Application::RemoveUserEvent( pImp->nEventId );
    delete pImp;
}

//==========================================================================

SFX_STATE_STUB( SfxApplication, MacroState_Impl )
SFX_EXEC_STUB( SfxApplication, MacroExec_Impl )

sal_uInt16 SfxMacroConfig::GetSlotId(SfxMacroInfoPtr pInfo)
{
    sal_uInt16 nCount = pImp->aArr.Count();      // Macro suchen
    sal_uInt16 i;
    for (i=0; i<nCount; i++)
        if ((*(pImp->aArr)[i]) == (*pInfo))
            break;

    if (i == nCount)
    {                                   // Macro noch unbekannt
        nCount = aIdArray.Count();
        sal_uInt16 n;
        for (n=0; n<nCount; n++) // freie SlotId suchen
            if (aIdArray[n] > SID_MACRO_START + n)
                break;

        sal_uInt16 nNewSlotId = SID_MACRO_START + n;
        if ( nNewSlotId > SID_MACRO_END )
            return 0;
        aIdArray.Insert( SID_MACRO_START + n, n );

        SfxSlot *pNewSlot = new SfxSlot;
        pNewSlot->nSlotId = SID_MACRO_START + n;
        pNewSlot->nGroupId = 0;
        pNewSlot->nFlags = SFX_SLOT_ASYNCHRON;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxApplication,MacroExec_Impl);
        pNewSlot->fnState = SFX_STUB_PTR(SfxApplication,MacroState_Impl);
        pNewSlot->pType = 0; HACK(SFX_TYPE(SfxVoidItem))
        pNewSlot->pName = pNewSlot->pMethodName = U2S(pInfo->aMethodName).getStr();
        pNewSlot->pLinkedSlot = 0;
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = 0;
        pNewSlot->pUnoName = 0;

        if (nCount)
        {
            SfxSlot *pSlot = (pImp->aArr)[0]->pSlot;
            pNewSlot->pNextSlot = pSlot->pNextSlot;
            pSlot->pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        // Macro uebernehmen
        SfxMacroInfoPtr pNewInfo = new SfxMacroInfo(*pInfo);
        pNewInfo->nSlotId = SID_MACRO_START + n;
        pImp->aArr.Insert(pNewInfo,n);
        pNewInfo->pSlot = pNewSlot;
        pInfo->nSlotId = pNewInfo->nSlotId;
        pNewInfo->nRefCnt++;
    }
    else
    {
        pInfo->nSlotId = (pImp->aArr)[i]->nSlotId;
        (pImp->aArr)[i]->nRefCnt++;
    }

    return pInfo->nSlotId;
}

//==========================================================================

void SfxMacroConfig::ReleaseSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId ist kein Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SfxMacroInfo *pInfo = (pImp->aArr)[i];
        if (pInfo->nSlotId == nId)
        {
            pInfo->nRefCnt--;
            if (pInfo->nRefCnt == 0)
            {
                // Slot wird nicht mehr referenziert, also holen
                SfxSlot *pSlot = pInfo->pSlot;

                // Slot aus der Verkettung rausnehmen
                while (pSlot->pNextSlot != pInfo->pSlot)
                    pSlot = (SfxSlot*) pSlot->pNextSlot;
                pSlot->pNextSlot = pInfo->pSlot->pNextSlot;

                // Slot selbst kurz schlie\sen
                pSlot = pInfo->pSlot;
                pSlot->pNextSlot = pSlot;

                // MacroInfo aus Array entfernen, damit sie kein Unheil
                // anrichten kann
                pImp->aArr.Remove(i);

                // SlotId wieder freigeben
                sal_uInt16 nCount = aIdArray.Count();
                for (sal_uInt16 n=0; n<nCount; n++)
                {
                    if (aIdArray[n] == nId)
                    {
                        aIdArray.Remove(n);
                        break;
                    }
                }

                // Falls ein Image belegt wwurde, kann das jetzt wieder frei
                // gegeben werden (wenn nicht aus dtor gerufen, da ist der
                // ImageManager schon weg)
                SfxImageManager *pImgMgr = SFX_IMAGEMANAGER();
                if (pImgMgr)
                    pImgMgr->ReplaceImage(nId, 0);

                // Sofern nicht die Applikation heruntergefahren wird, mu\s
                // der Slot asynchron gel"oscht werden, falls er in seinem
                // eigenen Execute abgeschossen wird!
                if ( !SFX_APP()->IsDowning() )
                    pImp->nEventId = Application::PostUserEvent( LINK(this, SfxMacroConfig, EventHdl_Impl), pInfo );
                else
                    EventHdl_Impl( pInfo );
            }
            return;
        }
    }

    DBG_ERROR("Macro-SlotId nicht gefunden!");
}

//==========================================================================

void SfxMacroConfig::RegisterSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId ist kein Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        if ((pImp->aArr)[i]->nSlotId == nId)
        {
            (pImp->aArr)[i]->nRefCnt++;
            return;
        }
    }

    DBG_ERROR("Macro-SlotId nicht gefunden!");
}

//==========================================================================

const SfxMacroInfoPtr SfxMacroConfig::GetMacroInfo(sal_uInt16 nId) const
{
    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ((pImp->aArr)[i]->nSlotId == nId)
            return (pImp->aArr)[i];

    return 0;
}

//==========================================================================

const SfxMacroInfoPtr SfxMacroConfig::GetMacroInfo_Impl( const SvxMacro *pMacro ) const
{
    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ((pImp->aArr)[i]->Compare(*pMacro) )
            return (pImp->aArr)[i];
    return 0;
}

//==========================================================================

sal_Bool SfxMacroConfig::ExecuteMacro( sal_uInt16 nId, const String& rArgs ) const
{
    const SfxMacroInfoPtr pInfo = GetMacroInfo( nId );
    if ( !pInfo )
        return sal_False;

    SfxObjectShell* pSh = SfxObjectShell::Current();

    SvxMacro aMacro( pInfo->GetQualifiedName(), pInfo->GetBasicName(), STARBASIC );
    sal_Bool bRet = ExecuteMacro( pSh, &aMacro, rArgs );

    // Release, da im Dispatcher-Execute ein Register gemacht wurde
    ((SfxMacroConfig*)this)->ReleaseSlotId( nId );
    return bRet;
}

sal_Bool SfxMacroConfig::ExecuteMacro( SfxObjectShell *pSh, const SvxMacro* pMacro, const String& rArgs ) const
{
    SfxApplication *pApp = SFX_APP();

    // Name des Macros oder Scripts bzw. ScriptCode
    String aCode( pMacro->GetMacName() );
    ErrCode nErr = ERRCODE_NONE;

    // Ist es ein Basic-Macro ?
    ScriptType eSType = pMacro->GetScriptType();
    sal_Bool bIsBasic = eSType == STARBASIC;
    sal_Bool bIsStarScript = ( eSType == EXTENDED_STYPE && pMacro->GetLibName().SearchAscii( "StarScript" ) != STRING_NOTFOUND );
    sal_Bool bIsBasicLibBased = bIsBasic || bIsStarScript || !pSh;

#if SUPD<582
    if ( !bIsBasicLibBased && pSh && pApp->GetIniManager()->IsJavaScriptEnabled() )
    {
        // AB: #49303# hier muss Window als this gesetzt werden
        SfxJS* pJS = pSh->GetMedium()->GetJavaScript();
        SjJScriptWindowObject* pWindow = pJS ? pJS->GetWindowObject() : NULL;

        Link aLink = LINK( this, SfxMacroConfig, CallbackHdl_Impl );
        pImp->bWaitingForCallback = sal_True;
        nErr = pSh->CallJavaScript( *pMacro, pWindow, &aLink );
        sal_uInt32 nTicks = Time::GetSystemTicks();
        nTicks += 5000;
        while ( pImp->bWaitingForCallback && nTicks > Time::GetSystemTicks() )
            Application::Yield();
        pImp->bWaitingForCallback = sal_False;
        return nErr != ERRCODE_NONE;
    }
    else
#endif

    if ( bIsBasicLibBased )
    {
        pApp->EnterBasicCall();
        BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
        if( bIsBasic )
        {
            // BasicManager von Document?
            BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

            // Da leider der Name zwischendurch h"aufig gewechselt hat ...
            if( SFX_APP()->GetName() == pMacro->GetLibName() ||
                    pMacro->GetLibName().EqualsAscii("StarDesktop") )
                pMgr = pAppMgr;
            else if ( pMgr == pAppMgr )
                pMgr = NULL;

            if ( pSh && pMgr && pMgr == pAppMgr )
            {
/*
                SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pSh, 0, sal_False );
                SbxVariable *pDocVar = pAppMgr->GetLib(0)->Find( "ThisDocument", SbxCLASS_PROPERTY );
                if ( pDocVar )
                    pDocVar->PutObject( pSh->GetSbxObject() );

                SbxVariable *pWinVar = pAppMgr->GetLib(0)->Find( "ThisWindow", SbxCLASS_PROPERTY );
                if ( pWinVar )
                    pWinVar->PutObject( pFrame->GetSbxObject() );
*/
                SFX_APP()->Get_Impl()->pThisDocument = pSh;
                SbxVariable *pCompVar = pAppMgr->GetLib(0)->Find( DEFINE_CONST_UNICODE("ThisComponent"), SbxCLASS_PROPERTY );
                if ( pCompVar )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface ( pSh->GetModel() , ::com::sun::star::uno::UNO_QUERY );
                    ::com::sun::star::uno::Any aAny;
                    aAny <<= xInterface;
                    pCompVar->PutObject( GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aAny ) );
                }

                nErr = Call( 0, aCode, pMgr );

                SFX_APP()->Get_Impl()->pThisDocument = NULL;
/*
                if ( pDocVar )
                    pDocVar->PutObject( NULL );
                if ( pWinVar )
                    pWinVar->PutObject( NULL );
 */
                if ( pCompVar )
                    pCompVar->PutObject( NULL );
            }
            else if ( pMgr )
                nErr = Call( 0, aCode, pMgr );
            else
                nErr = SbxERR_NO_METHOD;

        }
#if SUPD<582
        else if( bIsStarScript )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xSource;

            // Haben wir eine Shell?
            if( pSh )
            {
                // dann dort callen
                nErr = pSh->CallScript( pMacro->GetLibName(), aCode, xSource, NULL, NULL );
            }
            else
            {
                // ::com::sun::star::script::Engine von der Standard-Bibliothek anfordern
                StarBASIC* pStdLib = pAppMgr->GetStdLib();
                ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine >  xEngine= pStdLib->getEngine( pAppMgr );
                SfxMacroConfig::CallStarScript( xEngine, aCode, xSource, NULL, NULL );
            }
        }
#endif
        pApp->LeaveBasicCall();
    }
    else
    {
        nErr = SbxERR_NO_METHOD;
    }

    return ( nErr == ERRCODE_NONE );
}

sal_Bool SfxMacroConfig::CheckMacro( SfxObjectShell *pSh, const SvxMacro* pMacro ) const
{
    SfxApplication *pApp = SFX_APP();

    // Name des Macros oder Scripts bzw. ScriptCode
    String aCode( pMacro->GetMacName() );
    ErrCode nErr = ERRCODE_NONE;

#if SUPD<582
    // Ist es ein Basic-Macro ?
    sal_Bool bIsBasic = pMacro->GetScriptType() == STARBASIC;
    if ( !bIsBasic )
        return pSh != NULL;
#endif

    // BasicManager von Document oder Application
    pApp->EnterBasicCall();
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

    // Da leider der Name zwischendurch h"aufig gewechselt hat ...
    if( SFX_APP()->GetName() == pMacro->GetLibName() ||
            pMacro->GetLibName().EqualsAscii("StarDesktop") )
        pMgr = pAppMgr;
    else if ( pMgr == pAppMgr )
        pMgr = NULL;

    if ( !pMgr || !SfxQueryMacro( pMgr, aCode ) )
        nErr = SbxERR_NO_METHOD;
    pApp->LeaveBasicCall();
    return ( nErr == ERRCODE_NONE );
}

//==========================================================================

sal_Bool SfxMacroConfig::CheckMacro( sal_uInt16 nId ) const
{
    const SfxMacroInfoPtr pInfo = GetMacroInfo( nId );
    if ( !pInfo )
        return sal_False;

    // Basic nur initialisieren, wenn default nicht ::com::sun::star::script::JavaScript; dann mu\s
    // in IsBasic() sowieso das Basic angelegt werden
    SfxObjectShell* pSh = SfxObjectShell::Current();

    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();

    // BasicManager von Document oder Application
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

    if( SFX_APP()->GetName() == pInfo->GetBasicName() )
        pMgr = SFX_APP()->GetBasicManager();
    else if ( pMgr == pAppMgr )
        pMgr = NULL;

    String aFull( pInfo->GetQualifiedName() );
    sal_Bool bIsBasic = pMgr ? IsBasic( 0, aFull, pMgr ) : sal_False;
    pApp->LeaveBasicCall();
    return bIsBasic;
}

//==========================================================================

IMPL_LINK( SfxMacroConfig, CallbackHdl_Impl, SfxMacroConfig*, pConfig )
{
    pImp->bWaitingForCallback = sal_False;
    return 0;
}

IMPL_LINK( SfxMacroConfig, EventHdl_Impl, SfxMacroInfo*, pInfo )
{
    delete pInfo;
    pImp->nEventId = 0;
    return 0;
}

sal_Bool SfxMacroConfig::IsBasic( SbxObject* pVCtrl, const String& rCode, BasicManager* pMgr )
{
    sal_Bool bFound;
    SFX_APP()->EnterBasicCall();
    bFound = SfxQueryMacro( pMgr, rCode ) != 0;
    SFX_APP()->LeaveBasicCall();
    return bFound;
}

ErrCode SfxMacroConfig::Call( SbxObject* pVCtrl,
    const String& rCode, BasicManager* pMgr, SbxArray *pArgs, SbxValue *pRet )
{
    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();
    SbMethod* pMethod = SfxQueryMacro( pMgr, rCode );
    ErrCode nErr = 0;
    if( pMethod )
    {
        if ( pArgs )
            pMethod->SetParameters( pArgs );
        nErr = pMethod->Call( pRet );
    }
    else
        nErr = ERRCODE_BASIC_PROC_UNDEFINED;

    pApp->LeaveBasicCall();
    return nErr;
}

#if SUPD<582
void SfxMacroConfig::CallStarScript( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine > & rxEngine, const String & rCode,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rSource, void *pArgs, void *pRet )
{
    static String aCallPrefix( "();" );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   aArgs;
    if( pArgs )
        aArgs = *(::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > *)pArgs;

    // HACK!!! TODO: Klaeren, wie das wirklich laufen soll
    String aCode = rCode;
    aCode += aCallPrefix;

    ::rtl::OUString aUCode = StringToOUString( aCode, RTL_TEXTENCODING_UTF8 );
    if ( pRet )
    {
        *((::com::sun::star::uno::Any*)pRet) = rxEngine->run( aUCode, rSource, aArgs );
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngineListener >  xCB;
        rxEngine->runAsync( aUCode, rSource, aArgs, xCB );
    }
}
#endif

sal_Bool SfxMacroConfig::IsMacroSlot( sal_uInt16 nId )
{
    return ( nId >= SID_MACRO_START && nId <= SID_MACRO_END );
}


