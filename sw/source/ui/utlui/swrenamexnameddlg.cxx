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

#include <swtypes.hxx>
#include <globals.hrc>
#include <misc.hrc>

#include <utlui.hrc>
#include <unotools.hrc>
#include <unoprnms.hxx>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <unocrsr.hxx>

#include "swrenamexnameddlg.hxx"


using namespace ::com::sun::star;
using ::rtl::OUString;

SwRenameXNamedDlg::SwRenameXNamedDlg( Window* pWin,
            uno::Reference< container::XNamed > & xN,
            uno::Reference< container::XNameAccess > & xNA ) :
    ModalDialog(pWin, SW_RES(DLG_RENAME_XNAMED)),
   aNameFL(this, SW_RES(FL_NAME)),
    aNewNameFT(this, SW_RES(FT_NEW_NAME)),
   aNewNameED(this, SW_RES(ED_NEW_NAME)),
   aOk(this, SW_RES(PB_OK)),
   aCancel(this, SW_RES(PB_CANCEL)),
   aHelp(this, SW_RES(PB_HELP)),
   xNamed(xN),
   xNameAccess(xNA)
{
    FreeResource();
    sRemoveWarning = String(SW_RES(STR_REMOVE_WARNING));

    String sTmp(GetText());
    aNewNameED.SetText(xNamed->getName());
    aNewNameED.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    sTmp += String(xNamed->getName());
    SetText(sTmp);

    aOk.SetClickHdl(LINK(this, SwRenameXNamedDlg, OkHdl));
    aNewNameED.SetModifyHdl(LINK(this, SwRenameXNamedDlg, ModifyHdl));
    aOk.Enable(sal_False);
}

IMPL_LINK_NOARG(SwRenameXNamedDlg, OkHdl)
{
    try
    {
        xNamed->setName(aNewNameED.GetText());
    }
    catch (const uno::RuntimeException&)
    {
        OSL_FAIL("name wasn't changed");
    }
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK(SwRenameXNamedDlg, ModifyHdl, NoSpaceEdit*, pEdit)
{
    String sTmp(pEdit->GetText());

    // prevent from pasting illegal characters
    sal_uInt16 nLen = sTmp.Len();
    String sMsg;
    for(sal_uInt16 i = 0; i < pEdit->GetForbiddenChars().Len(); i++)
    {
        sal_uInt16 nTmpLen = sTmp.Len();
        sTmp = comphelper::string::remove(sTmp, pEdit->GetForbiddenChars().GetChar(i));
        if(sTmp.Len() != nTmpLen)
            sMsg += pEdit->GetForbiddenChars().GetChar(i);
    }
    if(sTmp.Len() != nLen)
    {
        pEdit->SetText(sTmp);
        String sWarning(sRemoveWarning);
        sWarning += sMsg;
        InfoBox(this, sWarning).Execute();
    }

    aOk.Enable(sTmp.Len() && !xNameAccess->hasByName(sTmp)
    && (!xSecondAccess.is() || !xSecondAccess->hasByName(sTmp))
    && (!xThirdAccess.is() || !xThirdAccess->hasByName(sTmp))
    );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
