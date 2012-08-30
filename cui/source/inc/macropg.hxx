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
#ifndef _MACROPG_HXX
#define _MACROPG_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <svl/macitem.hxx>
#include <vcl/lstbox.hxx>
#include <rtl/ustring.hxx>

#include <boost/unordered_map.hpp>
#include <vector>

typedef ::boost::unordered_map< ::rtl::OUString, ::std::pair< ::rtl::OUString, ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > EventsHash;

struct EventDisplayName
{
    const sal_Char* pAsciiEventName;
    sal_uInt16          nEventResourceID;
    EventDisplayName() : pAsciiEventName( NULL ), nEventResourceID(0) { }
    EventDisplayName( const sal_Char* _pAsciiName, const sal_uInt16 _nResId )
        : pAsciiEventName( _pAsciiName )
        , nEventResourceID( _nResId )
    {
    }
};
typedef ::std::vector< EventDisplayName >   EventDisplayNames;

class _SvxMacroTabPage;
class SvTabListBox;

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
    EventDisplayNames aDisplayNames;

                                _SvxMacroTabPage( Window* pParent, const ResId& rId, const SfxItemSet& rItemSet );

    void                        EnableButtons();
    ::com::sun::star::uno::Any  GetPropsByName( const ::rtl::OUString& eventName, EventsHash& eventsHash );
    ::std::pair< ::rtl::OUString, ::rtl::OUString > GetPairFromAny( ::com::sun::star::uno::Any aAny );

public:

    virtual                     ~_SvxMacroTabPage();
    void                        InitResources();

    void                        InitAndSetHandler( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xAppEvents, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xDocEvents, ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable > xModifiable );
    virtual sal_Bool                FillItemSet( SfxItemSet& rSet );

    using SfxTabPage::Reset;
    virtual void                Reset();

    void                        DisplayAppEvents( bool appEvents);
    void                        SetReadOnly( sal_Bool bSet );
    sal_Bool                        IsReadOnly() const;
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

typedef sal_uInt16* (*GetTabPageRanges)(); // gives international Which-values

class SvxMacroAssignSingleTabDialog : public SfxModalDialog
{
public:
    SvxMacroAssignSingleTabDialog( Window* pParent, const SfxItemSet& rOptionsSet, sal_uInt16 nUniqueId );

    virtual             ~SvxMacroAssignSingleTabDialog();

    void                SetTabPage( SfxTabPage* pTabPage );

private:
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


class SvxMacroAssignDlg : public SvxMacroAssignSingleTabDialog
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
