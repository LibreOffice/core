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
#ifndef INCLUDED_CUI_SOURCE_INC_MACROPG_HXX
#define INCLUDED_CUI_SOURCE_INC_MACROPG_HXX

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

typedef ::boost::unordered_map< OUString, ::std::pair< OUString, OUString >, OUStringHash, ::std::equal_to< OUString > > EventsHash;

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
    DECL_STATIC_LINK( _SvxMacroTabPage, SelectEvent_Impl, SvTabListBox * );
    DECL_STATIC_LINK( _SvxMacroTabPage, AssignDeleteHdl_Impl, PushButton * );
    DECL_STATIC_LINK( _SvxMacroTabPage, DoubleClickHdl_Impl, SvTabListBox * );

    static long GenericHandler_Impl( _SvxMacroTabPage* pThis, PushButton* pBtn );

protected:
    _SvxMacroTabPage_Impl*      mpImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xAppEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xDocEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable > m_xModifiable;
    EventsHash m_appEventsHash;
    EventsHash m_docEventsHash;
    bool bReadOnly, bDocModified, bAppEvents, bInitialized;
    EventDisplayNames aDisplayNames;

    _SvxMacroTabPage( Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet& rItemSet );

    void                        EnableButtons();
    ::com::sun::star::uno::Any  GetPropsByName( const OUString& eventName, EventsHash& eventsHash );
    ::std::pair< OUString, OUString > GetPairFromAny( ::com::sun::star::uno::Any aAny );

public:

    virtual                     ~_SvxMacroTabPage();
    void                        InitResources();

    void                        InitAndSetHandler( ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xAppEvents, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > xDocEvents, ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable > xModifiable );
    virtual sal_Bool                FillItemSet( SfxItemSet& rSet );

    virtual void        Reset( const SfxItemSet& );

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
};

// class SvxMacroAssignDlg --------------------------------------------------

typedef sal_uInt16* (*GetTabPageRanges)(); // gives international Which-values

class SvxMacroAssignSingleTabDialog : public SfxSingleTabDialog
{
public:
    SvxMacroAssignSingleTabDialog(Window* pParent, const SfxItemSet& rOptionsSet);

private:
    DECL_DLLPRIVATE_LINK( OKHdl_Impl, Button * );
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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
