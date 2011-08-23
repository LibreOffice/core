/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/rcid.h>
#include <vcl/wintypes.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#if defined(OS2)
#include <vcl/sound.hxx>
#endif

#include <bf_svtools/ehdl.hxx>
#include <bf_svtools/svtdata.hxx>
#include <bf_svtools/svtools.hrc>
#include "sfxecode.hxx"

namespace binfilter
{

//=========================================================================

static USHORT aWndFunc(
    Window *pWin,            // Parent des Dialoges
    USHORT nFlags,
    const String &rErr,	     // Fehlertext
    const String &rAction)   // Actiontext

/*  [Beschreibung]

    Bringt eine Fehlerbox auf den Schirm. Je nach nFlags werden
    Error/ Info usw. Boxen mit den gewuenschten Buttons angezeigt

    Rueckgabewert ist der gedrueckte Button

    */


{
    SolarMutexGuard aGuard;

    // aus den Flags die benoetigten WinBits ermitteln
    WinBits eBits=0;
    if ( (ERRCODE_BUTTON_CANCEL|ERRCODE_BUTTON_RETRY) == (nFlags & (ERRCODE_BUTTON_CANCEL|ERRCODE_BUTTON_RETRY)) )
        eBits = WB_RETRY_CANCEL;
    else if ( ERRCODE_BUTTON_OK_CANCEL == (nFlags & ERRCODE_BUTTON_OK_CANCEL) )
        eBits = WB_OK_CANCEL;
    else if ( ERRCODE_BUTTON_OK == (nFlags & ERRCODE_BUTTON_OK) )
        eBits = WB_OK;
    else if ( ERRCODE_BUTTON_YES_NO_CANCEL == (nFlags & ERRCODE_BUTTON_YES_NO_CANCEL) )
        eBits = WB_YES_NO_CANCEL;
    else if ( ERRCODE_BUTTON_YES_NO == (nFlags & ERRCODE_BUTTON_YES_NO) )
        eBits = WB_YES_NO;

    switch(nFlags & 0x0f00)
    {
      case ERRCODE_BUTTON_DEF_OK:
            eBits |= WB_DEF_OK;
            break;

      case ERRCODE_BUTTON_DEF_CANCEL:
            eBits |= WB_DEF_CANCEL;
            break;

      case ERRCODE_BUTTON_DEF_YES:
            eBits |= WB_DEF_YES;
            break;

      case ERRCODE_BUTTON_DEF_NO:
            eBits |= WB_DEF_NO;
            break;
    }

    String aErr(String(RTL_CONSTASCII_USTRINGPARAM("$(ACTION)$(ERROR)")));
    String aAction(rAction);
    if ( aAction.Len() )
        aAction += String::CreateFromAscii( ":\n" );
    aErr.SearchAndReplace(String::CreateFromAscii( "$(ACTION)" ), aAction);
    aErr.SearchAndReplace(String::CreateFromAscii( "$(ERROR)" ), rErr);

    MessBox* pBox;
    switch ( nFlags & 0xf000 )
    {
        case ERRCODE_MSG_ERROR:
            pBox = new ErrorBox(pWin, eBits, aErr);
            break;

        case ERRCODE_MSG_WARNING:
            pBox = new WarningBox(pWin, eBits, aErr);
            break;

        case ERRCODE_MSG_INFO:
            pBox = new InfoBox(pWin, aErr);
            break;

        case ERRCODE_MSG_QUERY:
            pBox = new QueryBox(pWin, eBits, aErr);
            break;

        default:
        {
            DBG_ERRORFILE( "no MessBox type");
            pBox = NULL;
            return ERRCODE_BUTTON_OK;
        }
    }

    USHORT nRet = RET_CANCEL;
    switch ( pBox->Execute() )
    {
        case RET_OK:
            nRet = ERRCODE_BUTTON_OK;
            break;
        case RET_CANCEL:
            nRet = ERRCODE_BUTTON_CANCEL;
            break;
        case RET_RETRY:
            nRet = ERRCODE_BUTTON_RETRY;
            break;
        case RET_YES:
            nRet = ERRCODE_BUTTON_YES;
            break;
        case RET_NO:
            nRet = ERRCODE_BUTTON_NO;
            break;
        default:
            DBG_ERRORFILE( "Unknown MessBox return value" );
            break;
    }
    delete pBox;
    return nRet;
}

//-------------------------------------------------------------------------

SfxErrorHandler::SfxErrorHandler(USHORT nIdP, ULONG lStartP, ULONG lEndP, ResMgr *pMgrP) :

    lStart(lStartP), lEnd(lEndP), nId(nIdP), pMgr(pMgrP), pFreeMgr( NULL )

{
    RegisterDisplay(&aWndFunc);
    if( ! pMgr )
    {
        com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pFreeMgr = pMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(ofa), aLocale );
    }
}

//-------------------------------------------------------------------------

SfxErrorHandler::~SfxErrorHandler()
{
    if( pFreeMgr )
        delete pFreeMgr;
}

//-------------------------------------------------------------------------

BOOL SfxErrorHandler::CreateString(
    const ErrorInfo *pErr, String &rStr, USHORT& nFlags) const

/*  [Beschreibung]

    Der Fehlerstring fuer die ErrorInfo pErr wird zusammengesetzt.

    */

{
    ULONG nErrCode = pErr->GetErrorCode() & ERRCODE_ERROR_MASK;
    if( nErrCode>=lEnd || nErrCode<=lStart )
        return FALSE;
    MessageInfo *pMsgInfo=PTR_CAST(MessageInfo,pErr);
    if(pMsgInfo)
    {
        if(GetMessageString(nErrCode, rStr, nFlags))
        {
            for (xub_StrLen i = 0; i < rStr.Len();)
            {
                i = rStr.SearchAndReplace(String::CreateFromAscii( "$(ARG1)" ),
                                          pMsgInfo->GetMessageArg(), i);
                if (i == STRING_NOTFOUND)
                    break;
                i = i + pMsgInfo->GetMessageArg().Len();
            }
            return TRUE;
        }
    }
    else if(GetErrorString(nErrCode, rStr, nFlags))
    {
        StringErrorInfo *pStringInfo=PTR_CAST(StringErrorInfo,pErr);
        if(pStringInfo)
            for (xub_StrLen i = 0; i < rStr.Len();)
            {
                i = rStr.SearchAndReplace(String::CreateFromAscii( "$(ARG1)" ),
                                          pStringInfo->GetErrorString(), i);
                if (i == STRING_NOTFOUND)
                    break;
                i = i + pStringInfo->GetErrorString().Len();
            }
        else
        {
            TwoStringErrorInfo * pTwoStringInfo = PTR_CAST(TwoStringErrorInfo,
                                                           pErr);
            if (pTwoStringInfo)
                for (USHORT i = 0; i < rStr.Len();)
                {
                    USHORT nArg1Pos = rStr.Search(String::CreateFromAscii( "$(ARG1)" ), i);
                    USHORT nArg2Pos = rStr.Search(String::CreateFromAscii( "$(ARG2)" ), i);
                    if (nArg1Pos < nArg2Pos)
                    {
                        rStr.Replace(nArg1Pos, 7, pTwoStringInfo->GetArg1());
                        i = nArg1Pos + pTwoStringInfo->GetArg1().Len();
                    }
                    else if (nArg2Pos < nArg1Pos)
                    {
                        rStr.Replace(nArg2Pos, 7, pTwoStringInfo->GetArg2());
                        i = nArg2Pos + pTwoStringInfo->GetArg2().Len();
                    }
                    else break;
                }
        }
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

class ResString: public String

/*  [Beschreibung]

    Hilfsklasse zum Auslesen eines Strings und optionaler ExtraData aus
    einer String Resource.

    */

{
    USHORT nFlags;
  public:
    USHORT GetFlags() const {return nFlags;}
    const String & GetString() const {return *this;}
    ResString( ResId &rId);
};

//-------------------------------------------------------------------------

ResString::ResString(ResId & rId):
    String(rId.SetAutoRelease(FALSE)),
    nFlags(0)
{
    ResMgr * pResMgr = rId.GetResMgr();
     // String ctor temporarily sets global ResManager
    if (pResMgr->GetRemainSize())
        nFlags = USHORT(pResMgr->ReadShort());
    rId.SetAutoRelease(TRUE);
    pResMgr->PopContext();
}

//-------------------------------------------------------------------------

struct ErrorResource_Impl : private Resource

/*  [Beschreibung]

    Hilfsklasse zum Zugriff auf String SubResourcen einer Resource
    */

{

    ResId aResId;

    ErrorResource_Impl(ResId& rErrIdP, USHORT nId)
        : Resource(rErrIdP),aResId(nId,*rErrIdP.GetResMgr()){}

    ~ErrorResource_Impl() { FreeResource(); }

    operator ResString(){ return ResString( aResId ); }
    operator BOOL(){return IsAvailableRes(aResId.SetRT(RSC_STRING));}

};


BOOL SfxErrorHandler::GetClassString(ULONG lClassId, String &rStr) const
{
    ResId aId(RID_ERRHDL, *pMgr);
    ErrorResource_Impl aEr(aId, (USHORT)lClassId);
    if(aEr)
    {
        rStr=((ResString)aEr).GetString();
        return TRUE;
    }
    else
        return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxErrorHandler::GetMessageString(
    ULONG lErrId, String &rStr, USHORT &nFlags) const

/*  [Beschreibung]

    Erzeugt den String fuer die Ausgabe in einer MessageBox

    */

{
    BOOL bRet = FALSE;
    ResId *pResId= new ResId(nId, *pMgr);

    ErrorResource_Impl aEr(*pResId, (USHORT)lErrId);
    if(aEr)
    {
        ResString aErrorString(aEr);
        USHORT nResFlags = aErrorString.GetFlags();
        if( nResFlags )
            nFlags=aErrorString.GetFlags();
        rStr = aErrorString.GetString();
        bRet = TRUE;
    }

    delete pResId;
    return bRet;
}

//-------------------------------------------------------------------------

BOOL SfxErrorHandler::GetErrorString(
    ULONG lErrId, String &rStr, USHORT &nFlags) const

/*  [Beschreibung]
    Erzeugt den Fehlerstring fuer den eigentlichen Fehler ohne
    dessen Klasse

    */

{
    SolarMutexGuard aGuard;

    BOOL bRet = FALSE;
    rStr=String(String(RTL_CONSTASCII_USTRINGPARAM("$(CLASS)$(ERROR)")));
    ResId *pResId = new ResId(nId, *pMgr);

    {
        ErrorResource_Impl aEr(*pResId, (USHORT)lErrId);
        if(aEr)
        {
            ResString aErrorString(aEr);

            USHORT nResFlags = aErrorString.GetFlags();
            if ( nResFlags )
                nFlags = nResFlags;
            rStr.SearchAndReplace(
                String::CreateFromAscii("$(ERROR)"), aErrorString.GetString());
            bRet = TRUE;
        }
        else
            bRet = FALSE;
    }

    if( bRet )
    {
        String aErrStr;
        GetClassString(lErrId & ERRCODE_CLASS_MASK,
                       aErrStr);
        if(aErrStr.Len())
            aErrStr+=String::CreateFromAscii( ".\n" );
        rStr.SearchAndReplace(String::CreateFromAscii( "$(CLASS)" ),aErrStr);
    }

    delete pResId;
    return bRet;
}

//-------------------------------------------------------------------------

SfxErrorContext::SfxErrorContext(
    USHORT nCtxIdP, const String &aArg1P, Window *pWindow,
    USHORT nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP),
    aArg1(aArg1P)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}

//-------------------------------------------------------------------------

BOOL SfxErrorContext::GetString(ULONG nErrId, String &rStr)

/*  [Beschreibung]

    Baut die Beschreibung eines ErrorContextes auf
    */

{
    bool bRet = false;
    ResMgr* pFreeMgr = NULL;
    if( ! pMgr )
    {
        com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pFreeMgr = pMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(ofa), aLocale );
    }
    if( pMgr )
    {
        SolarMutexGuard aGuard;

        ResId aResId( nResId, *pMgr );

        ErrorResource_Impl aTestEr( aResId, nCtxId );
        if ( aTestEr )
        {
            rStr = ( (ResString)aTestEr ).GetString();
            rStr.SearchAndReplace( String::CreateFromAscii( "$(ARG1)" ), aArg1 );
            bRet = true;
        }
        else
        {
            DBG_ERRORFILE( "ErrorContext cannot find the resource" );
            bRet = false;
        }

        if ( bRet )
        {
            USHORT nId = ( nErrId & ERRCODE_WARNING_MASK ) ? ERRCTX_WARNING : ERRCTX_ERROR;
            ResId aSfxResId( RID_ERRCTX, *pMgr );
            ErrorResource_Impl aEr( aSfxResId, nId );
            rStr.SearchAndReplace( String::CreateFromAscii( "$(ERR)" ), ( (ResString)aEr ).GetString() );
        }
    }

    if( pFreeMgr )
    {
        delete pFreeMgr;
        pMgr = NULL;
    }
    return bRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
