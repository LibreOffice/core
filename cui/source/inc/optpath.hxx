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

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
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
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker2 > xFolderPicker;

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
