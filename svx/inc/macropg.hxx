/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macropg.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:09:41 $
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

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>

#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <rtl/ustring.hxx>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <hash_map>

typedef ::std::hash_map< ::rtl::OUString, ::std::pair< ::rtl::OUString, ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > EventsHash;
typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > UIEventsStringHash;

class _SvxMacroTabPage;
class SvStringsDtor;
class SvTabListBox;
class Edit;
class String;

class _HeaderTabListBox;
class _SvxMacroTabPage_Impl;


class _SvxMacroTabPage : public SfxTabPage
{
#if _SOLAR__PRIVATE
    DECL_STATIC_LINK( _SvxMacroTabPage, SelectEvent_Impl, SvTabListBox * );
    DECL_STATIC_LINK( _SvxMacroTabPage, AssignDeleteHdl_Impl, PushButton * );
    DECL_STATIC_LINK( _SvxMacroTabPage, DoubleClickHdl_Impl, SvTabListBox * );

    static long GenericHandler_Impl( _SvxMacroTabPage* pThis, PushButton* pBtn );

#endif
protected:
    _SvxMacroTabPage_Impl*      mpImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xAppEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xDocEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable > m_xModifiable;
    EventsHash m_appEventsHash;
    EventsHash m_docEventsHash;
    bool bReadOnly, bDocModified, bAppEvents, bInitialized;
    UIEventsStringHash aUIStrings;

                                _SvxMacroTabPage( Window* pParent, const ResId& rId, const SfxItemSet& rItemSet );

    void                        EnableButtons( const String& rLanguage );
    ::com::sun::star::uno::Any  GetPropsByName( const ::rtl::OUString& eventName, EventsHash& eventsHash );
    ::std::pair< ::rtl::OUString, ::rtl::OUString > GetPairFromAny( ::com::sun::star::uno::Any aAny );

public:

    virtual                     ~_SvxMacroTabPage();
    void                        InitResources();

    void                        InitAndSetHandler( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xAppEvents, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xDocEvents, ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable > xModifiable );
    virtual BOOL                FillItemSet( SfxItemSet& rSet );

    using SfxTabPage::Reset;
    virtual void                Reset();

    void                        DisplayAppEvents( bool appEvents);
    void                        SetReadOnly( BOOL bSet );
    BOOL                        IsReadOnly() const;
};

class SvxMacroTabPage : public _SvxMacroTabPage
{
public:
    SvxMacroTabPage(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xNameReplace,
        sal_uInt16 nSelectedIndex
    );
    virtual ~SvxMacroTabPage();
};

// class SvxMacroAssignDlg --------------------------------------------------

typedef USHORT* (*GetTabPageRanges)(); // liefert internationale Which-Werte

class SVX_DLLPUBLIC SvxMacroAssignSingleTabDialog : public SfxModalDialog
{
public:
    SvxMacroAssignSingleTabDialog( Window* pParent, const SfxItemSet& rOptionsSet, USHORT nUniqueId );

    virtual             ~SvxMacroAssignSingleTabDialog();

    void                SetTabPage( SfxTabPage* pTabPage );
    // SfxTabPage*          GetTabPage() const { return pPage; }

    // OKButton*            GetOKButton() const { return pOKBtn; }
    // CancelButton*        GetCancelButton() const { return pCancelBtn; }

private:
    SfxViewFrame*       pFrame;

    FixedLine*          pFixedLine;

    OKButton*           pOKBtn;
    CancelButton*       pCancelBtn;
    HelpButton*         pHelpBtn;

    SfxTabPage*         pPage;
    const SfxItemSet*   pOptions;
    SfxItemSet*         pOutSet;

#if _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( OKHdl_Impl, Button * );
#endif
};


class SVX_DLLPUBLIC SvxMacroAssignDlg : public SvxMacroAssignSingleTabDialog
{
public:
    SvxMacroAssignDlg(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >& xNameReplace,
        sal_uInt16 nSelectedIndex
    );
    virtual ~SvxMacroAssignDlg();
};

#endif
