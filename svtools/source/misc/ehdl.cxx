/*************************************************************************
 *
 *  $RCSfile: ehdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:02 $
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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_RCID_H //autogen
#include <tools/rcid.h>
#endif
#ifndef _SV_WINTYPES_HXX //autogen
#include <vcl/wintypes.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#if defined(MAC) || defined(OS2)
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#endif

#pragma hdrstop

#include "ehdl.hxx"
#include "svtdata.hxx"
#include "svtools.hrc"
#include "sfxecode.hxx"

//=========================================================================

static USHORT aWndFunc(
    Window *pWin,            // Parent des Dialoges
    USHORT nFlags,
    const String &rErr,      // Fehlertext
    const String &rAction)   // Actiontext

/*  [Beschreibung]

    Bringt eine Fehlerbox auf den Schirm. Je nach nFlags werden
    Error/ Info usw. Boxen mit den gewuenschten Buttons angezeigt

    Rueckgabewert ist der gedrueckte Button

    */


{
#if 0
    // OS/2 und Mac haengen bei ModalDialogs in Drop
    // #38777: TH meint, das man das nicht mehr braucht
#if defined(MAC) || defined(OS2)
    if ( Application::IsUICaptured() )
    {
        Sound::Beep();
        return ERRCODE_BUTTON_OK;
    }
#endif
#endif

    NAMESPACE_VOS( OGuard ) aGuard( Application::GetSolarMutex() );

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

    String aErr(SvtResId(STR_ERR_HDLMESS));
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

SfxErrorHandler::SfxErrorHandler(USHORT nIdP, ULONG lStartP, ULONG lEndP,
                                 ResMgr *pMgrP)
    : nId(nIdP), pMgr(pMgrP), lStart(lStartP), lEnd(lEndP)
{
    RegisterDisplay(&aWndFunc);
}

//-------------------------------------------------------------------------

SfxErrorHandler::~SfxErrorHandler()
{
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
            for (USHORT i = 0; i < rStr.Len();)
            {
                i = rStr.SearchAndReplace(String::CreateFromAscii( "$(ARG1)" ),
                                          pMsgInfo->GetMessageArg(), i);
                if (i == STRING_NOTFOUND)
                    break;
                i += pMsgInfo->GetMessageArg().Len();
            }
            return TRUE;
        }
    }
    else if(GetErrorString(nErrCode, rStr, nFlags))
    {
        StringErrorInfo *pStringInfo=PTR_CAST(StringErrorInfo,pErr);
        if(pStringInfo)
            for (USHORT i = 0; i < rStr.Len();)
            {
                i = rStr.SearchAndReplace(String::CreateFromAscii( "$(ARG1)" ),
                                          pStringInfo->GetErrorString(), i);
                if (i == STRING_NOTFOUND)
                    break;
                i += pStringInfo->GetErrorString().Len();
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
    ResMgr * pResMgr = Resource::GetResManager();
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
        : Resource(rErrIdP),aResId(nId){}

    ~ErrorResource_Impl() { FreeResource(); }

    operator ResString(){ return ResString( aResId ); }
    operator BOOL(){return IsAvailableRes(aResId.SetRT(RSC_STRING));}

};


BOOL SfxErrorHandler::GetClassString(ULONG lClassId, String &rStr) const

/*  [Beschreibung]

    Erzeugt den String fuer die Klasse des Fehlers. Wird immer aus der
    Resource des Sfx gelesen

    */

{

    ResId aId(RID_ERRHDL);
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
    ResId *pResId= new ResId(nId, pMgr);
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
    NAMESPACE_VOS( OGuard ) aGuard( Application::GetSolarMutex() );

    BOOL bRet = FALSE;
    rStr=String(SvtResId(RID_ERRHDL_CLASS));
    ResId *pResId = new ResId(nId, pMgr);

    {
        ErrorResource_Impl aEr(*pResId, (USHORT)lErrId);
        if(aEr)
        {
            ResString aErrorString(aEr);

            USHORT nResFlags = aErrorString.GetFlags();
            if ( nResFlags )
                nFlags = nResFlags;

            String aErrorRepl(aErrorString.GetString());
            if(aErrorRepl.Len())
                aErrorRepl += (nFlags & ERRCODE_MSG_QUERY) ? '?' : '.';

            rStr.SearchAndReplace(String::CreateFromAscii( "$(ERROR)" ),aErrorRepl);
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
    USHORT nCtxIdP, Window *pWin, USHORT nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWin), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}

//-------------------------------------------------------------------------

SfxErrorContext::SfxErrorContext(
    USHORT nCtxIdP, const String &aArg1P, Window *pWin,
    USHORT nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWin), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP),
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
    FASTBOOL bRet = FALSE;
    ResId* pResId = new ResId( nResId, pMgr );
    {
        {
            NAMESPACE_VOS( OGuard ) aGuard( Application::GetSolarMutex() );
            ErrorResource_Impl aEr( *pResId, nCtxId );
            if ( aEr )
            {
                rStr = ( (ResString)aEr ).GetString();
                rStr.SearchAndReplace( String::CreateFromAscii( "$(ARG1)" ), aArg1 );
                bRet = TRUE;
            }
            else
            {
                DBG_ERRORFILE( "ErrorContext cannot find the resource" );
                bRet = FALSE;
            }
        }

        if ( bRet )
        {
            NAMESPACE_VOS( OGuard ) aGuard( Application::GetSolarMutex() );
            USHORT nId = ( nErrId & ERRCODE_WARNING_MASK ) ? ERRCTX_WARNING : ERRCTX_ERROR;
            ResId aSfxResId( RID_ERRCTX );
            ErrorResource_Impl aEr( aSfxResId, nId );
            rStr.SearchAndReplace( String::CreateFromAscii( "$(ERR)" ), ( (ResString)aEr ).GetString() );
        }
    }
    delete pResId;
    return bRet;
}


