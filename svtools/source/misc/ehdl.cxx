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

#include <tools/rcid.h>
#include <tools/simplerm.hxx>
#include <tools/wintypes.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/soerr.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/sfxecode.hxx>
#include <memory>
#include "errtxt.hrc"
#include "strings.hxx"

static DialogMask aWndFunc(
    vcl::Window *pWin,            // Parent of the dialog
    DialogMask nFlags,
    const OUString &rErr,      // error text
    const OUString &rAction)   // action text

/*  [Description]

    Draw an errorbox on the screen. Depending on nFlags
    Error/Info etc. boxes with the requested buttons are shown.

    Returnvalue is the button pressed

    */


{
    SolarMutexGuard aGuard;

    // determine necessary WinBits from the flags
    WinBits eBits=0;
    if ( (nFlags & (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry)) == (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry))
        eBits = WB_RETRY_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsOkCancel) == DialogMask::ButtonsOkCancel )
        eBits = WB_OK_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsOk) == DialogMask::ButtonsOk )
        eBits = WB_OK;
    else if ( (nFlags & DialogMask::ButtonsYesNoCancel) == DialogMask::ButtonsYesNoCancel )
        eBits = WB_YES_NO_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsYesNo) == DialogMask::ButtonsYesNo )
        eBits = WB_YES_NO;

    switch(nFlags & DialogMask(0x0f00))
    {
      case DialogMask::ButtonDefaultsOk:
            eBits |= WB_DEF_OK;
            break;

      case DialogMask::ButtonDefaultsCancel:
            eBits |= WB_DEF_CANCEL;
            break;

      case DialogMask::ButtonDefaultsYes:
            eBits |= WB_DEF_YES;
            break;

      case DialogMask::ButtonDefaultsNo:
            eBits |= WB_DEF_NO;
            break;
      default: break;
    }

    OUString aErr(STR_ERR_HDLMESS);
    OUString aAction(rAction);
    if ( !aAction.isEmpty() )
        aAction += ":\n";
    aErr = aErr.replaceAll("$(ACTION)", aAction);
    aErr = aErr.replaceAll("$(ERROR)", rErr);

    VclPtr<MessBox> pBox;
    switch ( nFlags & DialogMask(0xf000) )
    {
        case DialogMask::MessageError:
            pBox.reset(VclPtr<ErrorBox>::Create(pWin, eBits, aErr));
            break;

        case DialogMask::MessageWarning:
            pBox.reset(VclPtr<WarningBox>::Create(pWin, eBits, aErr));
            break;

        case DialogMask::MessageInfo:
            pBox.reset(VclPtr<InfoBox>::Create(pWin, aErr));
            break;

        default:
        {
            SAL_WARN( "svtools.misc", "no MessBox type");
            return DialogMask::ButtonsOk;
        }
    }

    DialogMask nRet = DialogMask::NONE;
    switch ( pBox->Execute() )
    {
        case RET_OK:
            nRet = DialogMask::ButtonsOk;
            break;
        case RET_CANCEL:
            nRet = DialogMask::ButtonsCancel;
            break;
        case RET_RETRY:
            nRet = DialogMask::ButtonsRetry;
            break;
        case RET_YES:
            nRet = DialogMask::ButtonsYes;
            break;
        case RET_NO:
            nRet = DialogMask::ButtonsNo;
            break;
        default:
            SAL_WARN( "svtools.misc", "Unknown MessBox return value" );
            break;
    }
    pBox.disposeAndClear();
    return nRet;
}

SfxErrorHandler::SfxErrorHandler(const ErrMsgCode* pIdPs, ErrCode lStartP, ErrCode lEndP, const std::locale* pLocale)
    : lStart(lStartP), lEnd(lEndP), pIds(pIdPs), pResLocale(pLocale)
{
    ErrorRegistry::RegisterDisplay(&aWndFunc);
    if (!pResLocale)
    {
        xFreeLocale.reset(new std::locale(Translate::Create("svt", Application::GetSettings().GetUILanguageTag())));
        pResLocale = xFreeLocale.get();
    }
}

SfxErrorHandler::~SfxErrorHandler()
{
}

bool SfxErrorHandler::CreateString(const ErrorInfo *pErr, OUString &rStr) const

/*  [Description]

    Assemble error string for the ErrorInfo pErr.

    */

{
    ErrCode nErrCode = pErr->GetErrorCode().IgnoreWarning();
    if( nErrCode>=lEnd || nErrCode<=lStart )
        return false;
    if(GetErrorString(nErrCode, rStr))
    {
        const StringErrorInfo *pStringInfo = dynamic_cast<const StringErrorInfo *>(pErr);
        if(pStringInfo)
        {
            rStr = rStr.replaceAll("$(ARG1)", pStringInfo->GetErrorString());
        }
        else
        {
            const TwoStringErrorInfo * pTwoStringInfo = dynamic_cast<const TwoStringErrorInfo* >(pErr);
            if (pTwoStringInfo)
            {
                rStr = rStr.replaceAll("$(ARG1)", pTwoStringInfo->GetArg1());
                rStr = rStr.replaceAll("$(ARG2)", pTwoStringInfo->GetArg2());
            }
        }
        return true;
    }
    return false;
}

void SfxErrorHandler::GetClassString(sal_uLong lClassId, OUString &rStr)

/*  [Description]

    Creates the string for the class of the error. Will always
    be read from the resource of the Sfx.

    */

{
    std::locale loc(Translate::Create("svt", Application::GetSettings().GetUILanguageTag()));
    for (const ErrMsgCode* pItem = getRID_ERRHDL(); pItem->second; ++pItem)
    {
        if (sal_uInt32(pItem->second) == lClassId)
        {
            rStr = Translate::get(pItem->first, loc);
            break;
        }
    }
}

bool SfxErrorHandler::GetErrorString(ErrCode lErrId, OUString &rStr) const

/*  [Description]

    Creates the error string for the actual error
    without its class

    */

{
    bool bRet = false;
    rStr = RID_ERRHDL_CLASS;

    for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
    {
        if (pItem->second == lErrId)
        {
            rStr = rStr.replaceAll("$(ERROR)", Translate::get(pItem->first, *pResLocale));
            bRet = true;
            break;
        }
    }

    if( bRet )
    {
        OUString aErrStr;
        GetClassString((sal_uInt32)lErrId & ERRCODE_CLASS_MASK,
                       aErrStr);
        if(!aErrStr.isEmpty())
            aErrStr += ".\n";
        rStr = rStr.replaceAll("$(CLASS)",aErrStr);
    }

    return bRet;
}

SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, vcl::Window *pWindow, const ErrMsgCode* pIdsP, const std::locale* pResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), pResLocale(pResLocaleP)
{
    if (!pIds)
        pIds = getRID_ERRCTX();
}


SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, const OUString &aArg1P, vcl::Window *pWindow,
    const ErrMsgCode* pIdsP, const std::locale* pResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), pResLocale(pResLocaleP),
    aArg1(aArg1P)
{
    if (!pIds)
        pIds = getRID_ERRCTX();
}

bool SfxErrorContext::GetString(ErrCode nErrId, OUString &rStr)

/*  [Description]

    Constructs the description of a error context
    */

{
    bool bRet = false;
    std::locale* pFreeLocale = nullptr;
    if (!pResLocale)
    {
        pFreeLocale = new std::locale(Translate::Create("svt", Application::GetSettings().GetUILanguageTag()));
        pResLocale = pFreeLocale;
    }
    if (pResLocale)
    {
        for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
        {
            if (sal_uInt32(pItem->second) == nCtxId)
            {
                rStr = Translate::get(pItem->first, *pResLocale);
                rStr = rStr.replaceAll("$(ARG1)", aArg1);
                bRet = true;
                break;
            }
        }

        SAL_WARN_IF(!bRet, "svtools.misc", "ErrorContext cannot find the resource");

        if ( bRet )
        {
            sal_uInt16 nId = nErrId.IsWarning() ? ERRCTX_WARNING : ERRCTX_ERROR;
            for (const ErrMsgCode* pItem = getRID_ERRCTX(); pItem->second; ++pItem)
            {
                if (sal_uInt32(pItem->second) == nId)
                {
                    rStr = rStr.replaceAll("$(ERR)", Translate::get(pItem->first, *pResLocale));
                    break;
                }
            }
        }
    }

    if (pFreeLocale)
    {
        delete pFreeLocale;
        pResLocale = nullptr;
    }
    return bRet;
}

const ErrMsgCode* getRID_ERRHDL()
{
    return RID_ERRHDL;
}

const ErrMsgCode* getRID_ERRCTX()
{
    return RID_ERRCTX;
}

const ErrMsgCode* getRID_SO_ERROR_HANDLER()
{
    return RID_SO_ERROR_HANDLER;
}

const ErrMsgCode* getRID_SO_ERRCTX()
{
    return RID_SO_ERRCTX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
