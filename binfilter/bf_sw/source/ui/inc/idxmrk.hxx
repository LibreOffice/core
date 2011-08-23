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
#ifndef _IDXMRK_HXX
#define _IDXMRK_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _BASEDLGS_HXX
#include <bf_sfx2/basedlgs.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <bf_sfx2/childwin.hxx>
#endif
#ifndef _TOXE_HXX
#include "toxe.hxx"
#endif
#ifndef _STDCTRL_HXX
#include <bf_svtools/stdctrl.hxx>
#endif
#ifndef  _COM_SUN_STAR_I18N_XEXTENDEDINDEXENTRYSUPPLIER_HPP_
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#endif
namespace binfilter {
class SwWrtShell;
class SwTOXMgr;
class SwTOXMark;
/*--------------------------------------------------------------------
     Beschreibung:	Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;
/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertIdxMarkWrapper : public SfxChildWindow
{
protected:
    SwInsertIdxMarkWrapper(	Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertIdxMarkWrapper);

public:

};

/* -----------------15.09.99 08:39-------------------

 --------------------------------------------------*/
class SwAuthMarkModalDlg;
/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertAuthMarkWrapper : public SfxChildWindow
{
protected:
    SwInsertAuthMarkWrapper(	Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertAuthMarkWrapper);

public:
};
} //namespace binfilter
#endif // _IDXMRK_HXX

