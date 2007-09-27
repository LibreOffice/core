/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idxmrk.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:17:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHOPTIONS_HPP_
#include <com/sun/star/util/SearchOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _IDXMRK_HXX
#include <idxmrk.hxx>
#endif
#ifndef _TXTTXMRK_HXX
#include <txttxmrk.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _MULTMRK_HXX
#include <multmrk.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _INDEX_HRC
#include <index.hrc>
#endif
#ifndef _IDXMRK_HRC
#include <idxmrk.hrc>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#include <utlui.hrc>
#ifndef _SWCONT_HXX
#include <swcont.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif


/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertIdxMarkWrapper, FN_INSERT_IDX_ENTRY_DLG)

SwInsertIdxMarkWrapper::SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateIndexMarkFloatDlg( DLG_INSIDXMARK , pBindings, this, pParentWindow, pInfo );
    DBG_ASSERT(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();
    pWindow->Show();    // at this point,because before pSh has to be initialized in ReInitDlg()
                        // -> Show() will invoke StateChanged() and save pos
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertIdxMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();

    return aInfo;
}

void    SwInsertIdxMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}


/* -----------------07.09.99 08:15-------------------

 --------------------------------------------------*/
SFX_IMPL_CHILDWINDOW(SwInsertAuthMarkWrapper, FN_INSERT_AUTH_ENTRY_DLG)

SwInsertAuthMarkWrapper::SwInsertAuthMarkWrapper(   Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateAuthMarkFloatDlg( DLG_INSAUTHMARK, pBindings, this, pParentWindow, pInfo );
    DBG_ASSERT(pAbstDlg, "Dialogdiet fail!");
    pWindow = pAbstDlg->GetWindow();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}
/* -----------------07.09.99 09:14-------------------

 --------------------------------------------------*/
SfxChildWinInfo SwInsertAuthMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}
/* -----------------19.10.99 11:16-------------------

 --------------------------------------------------*/
void    SwInsertAuthMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    pAbstDlg->ReInitDlg(rWrtShell);
}

