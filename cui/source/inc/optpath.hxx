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
#ifndef _SVX_OPTPATH_HXX
#define _SVX_OPTPATH_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

#ifdef _SVX_OPTPATH_CXX
#include <svtools/headbar.hxx>
#else
class HeaderBar;
#endif
#include "ControlFocusHelper.hxx"

#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <svtools/dialogclosedlistener.hxx>

// forward ---------------------------------------------------------------

namespace svx
{
    class OptHeaderTabListBox;
}
struct OptPath_Impl;
class SvxPathTabPage;

// class SvxPathTabPage --------------------------------------------------

class SvxPathTabPage : public SfxTabPage
{
private:
    FixedLine           aStdBox;
    FixedText           aTypeText;
    FixedText           aPathText;
    SvxControlFocusHelper aPathCtrl;
    PushButton          aStandardBtn;
    PushButton          aPathBtn;

    HeaderBar*                  pHeaderBar;
    ::svx::OptHeaderTabListBox* pPathBox;
    OptPath_Impl*               pImpl;

    ::com::sun::star::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker > xFolderPicker;

#ifdef _SVX_OPTPATH_CXX
    void        ChangeCurrentEntry( const String& _rFolder );

    DECL_LINK(PathHdl_Impl, void *);
    DECL_LINK(StandardHdl_Impl, void *);

    DECL_LINK(PathSelect_Impl, void *);
    DECL_LINK(  HeaderSelect_Impl, HeaderBar * );
    DECL_LINK(  HeaderEndDrag_Impl, HeaderBar * );

    DECL_LINK( DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );

    void        GetPathList( sal_uInt16 _nPathHandle, String& _rInternalPath,
                             String& _rUserPath, String& _rWritablePath, sal_Bool& _rReadOnly );
    void        SetPathList( sal_uInt16 _nPathHandle,
                             const String& _rUserPath, const String& _rWritablePath );
#endif

public:
    SvxPathTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxPathTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
