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

#include <unotools/resmgr.hxx>
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
#include <errtxt.hrc>
#include <strings.hxx>

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
    MessBoxStyle eBits = MessBoxStyle::NONE;
    if ( (nFlags & (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry)) == (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry))
        eBits = MessBoxStyle::RetryCancel;
    else if ( (nFlags & DialogMask::ButtonsOkCancel) == DialogMask::ButtonsOkCancel )
        eBits = MessBoxStyle::OkCancel;
    else if ( (nFlags & DialogMask::ButtonsOk) == DialogMask::ButtonsOk )
        eBits = MessBoxStyle::Ok;
    else if ( (nFlags & DialogMask::ButtonsYesNoCancel) == DialogMask::ButtonsYesNoCancel )
        eBits = MessBoxStyle::YesNoCancel;
    else if ( (nFlags & DialogMask::ButtonsYesNo) == DialogMask::ButtonsYesNo )
        eBits = MessBoxStyle::YesNo;

    switch(nFlags & DialogMask(0x0f00))
    {
      case DialogMask::ButtonDefaultsOk:
            eBits |= MessBoxStyle::DefaultOk;
            break;

      case DialogMask::ButtonDefaultsCancel:
            eBits |= MessBoxStyle::DefaultCancel;
            break;

      case DialogMask::ButtonDefaultsYes:
            eBits |= MessBoxStyle::DefaultYes;
            break;

      case DialogMask::ButtonDefaultsNo:
            eBits |= MessBoxStyle::DefaultNo;
            break;
      default: break;
    }

    OUString aErr("$(ACTION)$(ERROR)");
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

SfxErrorHandler::SfxErrorHandler(const ErrMsgCode* pIdPs, ErrCodeArea lStartP, ErrCodeArea lEndP, const std::locale& rLocale)
    : lStart(lStartP), lEnd(lEndP), pIds(pIdPs), rResLocale(rLocale)
{
    ErrorRegistry::RegisterDisplay(&aWndFunc);
}

SfxErrorHandler::~SfxErrorHandler()
{
}

bool SfxErrorHandler::CreateString(const ErrorInfo *pErr, OUString &rStr) const

/*  [Description]

    Assemble error string for the ErrorInfo pErr.

    */

{
    ErrCode nErrCode(sal_uInt32(pErr->GetErrorCode()) & ERRCODE_ERROR_MASK);
    if (pErr->GetErrorCode().GetArea() < lStart || lEnd < pErr->GetErrorCode().GetArea())
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

void SfxErrorHandler::GetClassString(ErrCodeClass lClassId, OUString &rStr)

/*  [Description]

    Creates the string for the class of the error. Will always
    be read from the resource of the Sfx.

    */

{
    for (const std::pair<const char*, ErrCodeClass>* pItem = RID_ERRHDL_CLASS; pItem->first; ++pItem)
    {
        if (pItem->second == lClassId)
        {
            rStr = SvtResId(pItem->first);
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
    rStr = "$(CLASS)$(ERROR)";

    for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
    {
        if (pItem->second.GetRest() == lErrId.GetRest())
        {
            rStr = rStr.replaceAll("$(ERROR)", Translate::get(pItem->first, rResLocale));
            bRet = true;
            break;
        }
    }

    if( bRet )
    {
        OUString aErrStr;
        GetClassString(lErrId.GetClass(), aErrStr);
        if(!aErrStr.isEmpty())
            aErrStr += ".\n";
        rStr = rStr.replaceAll("$(CLASS)",aErrStr);
    }

    return bRet;
}

SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, vcl::Window *pWindow, const ErrMsgCode* pIdsP, const std::locale& rResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), rResLocale(rResLocaleP)
{
    if (!pIds)
        pIds = getRID_ERRCTX();
}


SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, const OUString &aArg1P, vcl::Window *pWindow,
    const ErrMsgCode* pIdsP, const std::locale& rResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), rResLocale(rResLocaleP),
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
    for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
    {
        if (sal_uInt32(pItem->second) == nCtxId)
        {
            rStr = Translate::get(pItem->first, rResLocale);
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
                rStr = rStr.replaceAll("$(ERR)", Translate::get(pItem->first, rResLocale));
                break;
            }
        }
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
