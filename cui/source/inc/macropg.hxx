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

#include <unordered_map>
#include <vector>

class SvTreeListBox;

typedef std::unordered_map< OUString, std::pair< OUString, OUString >,
                            OUStringHash > EventsHash;

struct EventDisplayName
{
    const sal_Char* pAsciiEventName;
    sal_uInt16          nEventResourceID;
    EventDisplayName( const sal_Char* _pAsciiName, const sal_uInt16 _nResId )
        : pAsciiEventName( _pAsciiName )
        , nEventResourceID( _nResId )
    {
    }
};
typedef ::std::vector< EventDisplayName >   EventDisplayNames;

class SvxMacroTabPage_;
class SvTabListBox;

class SvxMacroTabPage_Impl;


class SvxMacroTabPage_ : public SfxTabPage
{
    DECL_LINK_TYPED( SelectEvent_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( AssignDeleteHdl_Impl, Button *, void );
    DECL_LINK_TYPED( DoubleClickHdl_Impl, SvTreeListBox*, bool );

    static long GenericHandler_Impl( SvxMacroTabPage_* pThis, PushButton* pBtn );

protected:
    SvxMacroTabPage_Impl*      mpImpl;
    css::uno::Reference< css::container::XNameReplace > m_xAppEvents;
    css::uno::Reference< css::container::XNameReplace > m_xDocEvents;
    css::uno::Reference< css::util::XModifiable > m_xModifiable;
    EventsHash m_appEventsHash;
    EventsHash m_docEventsHash;
    bool bReadOnly, bDocModified, bAppEvents, bInitialized;
    EventDisplayNames aDisplayNames;

    SvxMacroTabPage_( vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet& rItemSet );

    void                        EnableButtons();
    static css::uno::Any  GetPropsByName( const OUString& eventName, EventsHash& eventsHash );
    static ::std::pair< OUString, OUString > GetPairFromAny( const css::uno::Any& aAny );

public:

    virtual                     ~SvxMacroTabPage_();
    virtual void                dispose() override;
    void                        InitResources();

    void                        InitAndSetHandler( const css::uno::Reference< css::container::XNameReplace >& xAppEvents, const css::uno::Reference< css::container::XNameReplace >& xDocEvents, const css::uno::Reference< css::util::XModifiable >& xModifiable );
    virtual bool                FillItemSet( SfxItemSet* rSet ) override;

    virtual void                Reset( const SfxItemSet* ) override;

    void                        DisplayAppEvents( bool appEvents);
    void                        SetReadOnly( bool bSet );
    bool                        IsReadOnly() const override;
};

class SvxMacroTabPage : public SvxMacroTabPage_
{
public:
    SvxMacroTabPage(
        vcl::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        css::uno::Reference< css::container::XNameReplace > xNameReplace,
        sal_uInt16 nSelectedIndex
    );
};

// class SvxMacroAssignDlg --------------------------------------------------

typedef const sal_uInt16* (*GetTabPageRanges)(); // gives international Which-values

class SvxMacroAssignSingleTabDialog : public SfxSingleTabDialog
{
public:
    SvxMacroAssignSingleTabDialog(vcl::Window* pParent, const SfxItemSet& rOptionsSet);

private:
    DECL_DLLPRIVATE_LINK_TYPED( OKHdl_Impl, Button *, void );
};


class SvxMacroAssignDlg : public SvxMacroAssignSingleTabDialog
{
public:
    SvxMacroAssignDlg(
        vcl::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        const css::uno::Reference< css::container::XNameReplace >& xNameReplace,
        sal_uInt16 nSelectedIndex
    );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
