/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
class SvStringsDtor;
class SvTabListBox;
class Edit;
class String;
class SfxObjectShell;

typedef SvStringsDtor* (*FNGetRangeHdl)( _SfxMacroTabPage*, const String& rLanguage );
typedef SvStringsDtor* (*FNGetMacrosOfRangeHdl)( _SfxMacroTabPage*, const String& rLanguage, const String& rRange );

class SfxConfigGroupListBox_Impl;
class SfxConfigFunctionListBox_Impl;

class _HeaderTabListBox;
class _SfxMacroTabPage_Impl;

class SFX2_DLLPUBLIC _SfxMacroTabPage : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;
//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, DoubleClickHdl_Impl, Control* );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton * );

    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, ChangeScriptHdl_Impl, RadioButton * );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, GetFocus_Impl, Edit* );
    DECL_DLLPRIVATE_STATIC_LINK( _SfxMacroTabPage, TimeOut_Impl, Timer* );
//#endif
protected:
    _SfxMacroTabPage_Impl*      mpImpl;

                                _SfxMacroTabPage( Window* pParent, const ResId& rId, const SfxItemSet& rItemSet );

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        FillMacroList();
    void                        EnableButtons( const String& rLanguage );

public:

    virtual                     ~_SfxMacroTabPage();

    void                        AddEvent( const String & rEventName, sal_uInt16 nEventId );

    const SvxMacroTableDtor&    GetMacroTbl() const;
    void                        SetMacroTbl( const SvxMacroTableDtor& rTbl );
    void                        ClearMacroTbl();

    virtual void                ScriptChanged( const String& rLanguage );

    // zum setzen / abfragen der Links
    void                        SetGetRangeLink( FNGetRangeHdl pFn );
    FNGetRangeHdl               GetGetRangeLink() const;
    void                        SetGetMacrosOfRangeLink( FNGetMacrosOfRangeHdl pFn );
    FNGetMacrosOfRangeHdl       GetGetMacrosOfRangeLink() const;

    // --------- Erben aus der Basis -------------
    virtual sal_Bool                FillItemSet( SfxItemSet& rSet );
    virtual void                Reset( const SfxItemSet& rSet );

    void                        SetReadOnly( sal_Bool bSet );
    sal_Bool                        IsReadOnly() const;
    void                        SelectEvent( const String& rEventName, sal_uInt16 nEventId );
};

inline const SvxMacroTableDtor& _SfxMacroTabPage::GetMacroTbl() const
{
    return aTbl;
}

inline void _SfxMacroTabPage::SetMacroTbl( const SvxMacroTableDtor& rTbl )
{
    aTbl = rTbl;
}

inline void _SfxMacroTabPage::ClearMacroTbl()
{
    aTbl.DelDtor();
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

class SFX2_DLLPUBLIC SfxMacroAssignDlg : public SfxSingleTabDialog
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
