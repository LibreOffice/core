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
#ifndef _MACROPG_HXX
#define _MACROPG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/frame/XFrame.hpp>

class _SfxMacroTabPage;
class SvTabListBox;
class Edit;
namespace rtl {
    class OUString;
}
class SfxObjectShell;

class SfxConfigFunctionListBox_Impl;

class _HeaderTabListBox;
class _SfxMacroTabPage_Impl;

class SFX2_DLLPUBLIC _SfxMacroTabPage : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, DoubleClickHdl_Impl, Control* );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, ChangeScriptHdl_Impl, RadioButton * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, GetFocus_Impl, Edit* );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, TimeOut_Impl, Timer* );

protected:
    _SfxMacroTabPage_Impl*      mpImpl;

                                _SfxMacroTabPage( Window* pParent, const ResId& rId, const SfxItemSet& rItemSet );

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        FillMacroList();
    void                        EnableButtons( const rtl::OUString& rLanguage );

public:

    virtual                     ~_SfxMacroTabPage();

    void                        AddEvent( const rtl::OUString & rEventName, sal_uInt16 nEventId );

    const SvxMacroTableDtor&    GetMacroTbl() const;
    void                        SetMacroTbl( const SvxMacroTableDtor& rTbl );

    virtual void                ScriptChanged( const rtl::OUString& rLanguage );

    // --------- Erben aus der Basis -------------
    virtual sal_Bool                FillItemSet( SfxItemSet& rSet );
    virtual void                Reset( const SfxItemSet& rSet );

    void                        SetReadOnly( sal_Bool bSet );
    sal_Bool                        IsReadOnly() const;
    void                        SelectEvent( const rtl::OUString& rEventName, sal_uInt16 nEventId );
};

inline const SvxMacroTableDtor& _SfxMacroTabPage::GetMacroTbl() const
{
    return aTbl;
}

inline void _SfxMacroTabPage::SetMacroTbl( const SvxMacroTableDtor& rTbl )
{
    aTbl = rTbl;
}

class SFX2_DLLPUBLIC SfxMacroTabPage : public _SfxMacroTabPage
{
public:
    SfxMacroTabPage(
        Window* pParent,
        const ResId& rId,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet
    );

    // --------- Erben aus der Basis -------------
    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );
};

class SFX2_DLLPUBLIC SfxMacroAssignDlg : public SfxNoLayoutSingleTabDialog
{
public:
    SfxMacroAssignDlg(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame,
        SfxItemSet& rSet );
    SfxMacroAssignDlg(
        Window* pParent,
        const SfxObjectShell* _pShell,
        SfxItemSet& rSet );
    virtual ~SfxMacroAssignDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
