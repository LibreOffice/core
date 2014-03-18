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
#ifndef INCLUDED_CUI_SOURCE_INC_MACROASS_HXX
#define INCLUDED_CUI_SOURCE_INC_MACROASS_HXX

#include "sal/config.h"

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/frame/XFrame.hpp>

class _SfxMacroTabPage;
class SvTabListBox;

class _SfxMacroTabPage_Impl;

class _SfxMacroTabPage : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, DoubleClickHdl_Impl, Control* );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, TimeOut_Impl, Timer* );

protected:
    _SfxMacroTabPage_Impl*      mpImpl;

                                _SfxMacroTabPage( Window* pParent, const SfxItemSet& rItemSet );

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        FillMacroList();
    void                        EnableButtons();

public:

    virtual                     ~_SfxMacroTabPage();

    void                        AddEvent( const OUString & rEventName, sal_uInt16 nEventId );

    const SvxMacroTableDtor&    GetMacroTbl() const;
    void                        SetMacroTbl( const SvxMacroTableDtor& rTbl );

    virtual void                ScriptChanged();
    virtual void                PageCreated (SfxAllItemSet aSet);
    using TabPage::ActivatePage; // FIXME WTF is this nonsense?
    virtual void                ActivatePage( const SfxItemSet& );
    void                        LaunchFillGroup();

    // --------- inherit from the base -------------
    virtual sal_Bool                FillItemSet( SfxItemSet& rSet );
    virtual void                Reset( const SfxItemSet& rSet );

    bool                        IsReadOnly() const;
};

inline const SvxMacroTableDtor& _SfxMacroTabPage::GetMacroTbl() const
{
    return aTbl;
}

inline void _SfxMacroTabPage::SetMacroTbl( const SvxMacroTableDtor& rTbl )
{
    aTbl = rTbl;
}

class SfxMacroTabPage : public _SfxMacroTabPage
{
public:
    SfxMacroTabPage(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet
    );

    // --------- inherit from the base -------------
    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );
};

class SfxMacroAssignDlg : public SfxSingleTabDialog
{
public:
    SfxMacroAssignDlg(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
