/*************************************************************************
 *
 *  $RCSfile: swrenamexnameddlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:41:06 $
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

#pragma hdrstop

#include <swtypes.hxx>
#include <globals.hrc>
#include <misc.hrc>

#include <utlui.hrc>
#include <unotools.hrc>

#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSORSUPPLIER_HPP_
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSCREENCURSOR_HPP_
#include <com/sun/star/view/XScreenCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_DOCUMENTZOOMTYPE_HPP_
#include <com/sun/star/view/DocumentZoomType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen wg. SfxDispatcher
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen wg. SfxStringItem
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#include <shellio.hxx>
#ifndef _SWDOCSH_HXX //autogen wg. SwDocShell
#include <docsh.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen wg. SwView
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX //autogen wg. SwWrtShell
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#include "swrenamexnameddlg.hxx"


using namespace ::com::sun::star;
using namespace ::rtl;

SwRenameXNamedDlg::SwRenameXNamedDlg( Window* pWin,
            uno::Reference< container::XNamed > & xN,
            uno::Reference< container::XNameAccess > & xNA ) :
    ModalDialog(pWin, SW_RES(DLG_RENAME_XNAMED)),
   xNamed(xN),
   xNameAccess(xNA),
   aNewNameFT(this, ResId(FT_NEW_NAME)),
   aNewNameED(this, ResId(ED_NEW_NAME)),
   aNameFL(this, ResId(FL_NAME)),
   aOk(this, ResId(PB_OK)),
   aCancel(this, ResId(PB_CANCEL)),
   aHelp(this, ResId(PB_HELP))
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
/* -----------------09.06.99 15:34-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(SwRenameXNamedDlg, OkHdl, OKButton*, pOk)
{
    try
    {
        xNamed->setName(aNewNameED.GetText());
    }
    catch(uno::RuntimeException&)
    {
        DBG_ERROR("Name wurde nicht geaendert")
    }
    EndDialog(RET_OK);
    return 0;
}
/* -----------------09.06.99 15:48-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(SwRenameXNamedDlg, ModifyHdl, NoSpaceEdit*, pEdit)
{
    String sTmp(pEdit->GetText());

    // prevent from pasting illegal characters
    sal_uInt16 nLen = sTmp.Len();
    String sMsg;
    for(sal_uInt16 i = 0; i < pEdit->GetForbiddenChars().Len(); i++)
    {
        sal_uInt16 nTmpLen = sTmp.Len();
        sTmp.EraseAllChars(pEdit->GetForbiddenChars().GetChar(i));
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
