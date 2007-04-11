/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macropg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:23:15 $
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
#ifndef _MACROPG_HXX
#define _MACROPG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
class _SfxMacroTabPage;
class SvStringsDtor;
class SvTabListBox;
class Edit;
class String;

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

    void                        AddEvent( const String & rEventName, USHORT nEventId );

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
    virtual BOOL                FillItemSet( SfxItemSet& rSet );
    virtual void                Reset( const SfxItemSet& rSet );

    void                        SetReadOnly( BOOL bSet );
    BOOL                        IsReadOnly() const;
    void                        SelectEvent( const String& rEventName, USHORT nEventId );
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
    SfxMacroTabPage( Window* pParent, const ResId& rId,
                    const SfxItemSet& rSet );

    // --------- Erben aus der Basis -------------
    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );
};

class SFX2_DLLPUBLIC SfxMacroAssignDlg : public SfxSingleTabDialog
{
public:
    SfxMacroAssignDlg( Window* pParent, SfxItemSet& rSet );
    virtual ~SfxMacroAssignDlg();
};

#endif
