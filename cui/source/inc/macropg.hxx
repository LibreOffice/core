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
#pragma once

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <unotools/resmgr.hxx>

#include <unordered_map>
#include <vector>

typedef std::pair<OUString, OUString> EventPair;
typedef std::unordered_map<OUString, EventPair> EventsHash;

class SvxMacroTabPage_;
class SvTabListBox;
class SvxMacroTabPage_Impl;


class SvxMacroTabPage_ : public SfxTabPage
{
    DECL_LINK( SelectEvent_Impl, weld::TreeView&, void );
    DECL_LINK( AssignDeleteHdl_Impl, weld::Button&, void );
    DECL_LINK( DoubleClickHdl_Impl, weld::TreeView&, bool );
    DECL_LINK( DeleteAllHdl_Impl, weld::Button&, void );

    void GenericHandler_Impl(const weld::Button* pBtn);
    const EventPair* LookupEvent(const OUString& rEventName);

    css::uno::Reference< css::container::XNameReplace > m_xAppEvents;
protected:
    std::unique_ptr<SvxMacroTabPage_Impl> mpImpl;
    css::uno::Reference< css::container::XNameReplace > m_xDocEvents;
    css::uno::Reference< css::util::XModifiable > m_xModifiable;
    EventsHash m_appEventsHash;
    EventsHash m_docEventsHash;
    int m_nAssignedEvents;
    bool bDocModified, bAppEvents, bInitialized;

    SvxMacroTabPage_(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OUString& rID, const SfxItemSet& rItemSet);

    void                        EnableButtons();
    static css::uno::Any  GetPropsByName( const OUString& eventName, EventsHash& eventsHash );
    static EventPair GetPairFromAny(const css::uno::Any& aAny);

public:

    virtual                     ~SvxMacroTabPage_() override;

    void                        InitAndSetHandler( const css::uno::Reference< css::container::XNameReplace >& xAppEvents, const css::uno::Reference< css::container::XNameReplace >& xDocEvents, const css::uno::Reference< css::util::XModifiable >& xModifiable );
    virtual bool                FillItemSet( SfxItemSet* rSet ) override;

    virtual void                Reset( const SfxItemSet* ) override;

    void                        DisplayAppEvents(bool appEvents, int nSelectIndex);
    void                        SetReadOnly( bool bSet );
    bool                        IsReadOnly() const override;
};

class SvxMacroTabPage : public SvxMacroTabPage_
{
public:
    SvxMacroTabPage(
        weld::Container* pPage, weld::DialogController* pController,
        const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        css::uno::Reference< css::container::XNameReplace > const & xNameReplace,
        sal_uInt16 nSelectedIndex
    );
};

// class SvxMacroAssignDlg --------------------------------------------------

typedef const WhichRangesContainer & (*GetTabPageRanges)(); // gives international Which-values

class SvxMacroAssignSingleTabDialog : public SfxSingleTabDialogController
{
public:
    SvxMacroAssignSingleTabDialog(weld::Window* pParent, const SfxItemSet& rOptionsSet);

private:
    DECL_LINK(OKHdl_Impl, weld::Button&, void);
};

class SvxMacroAssignDlg : public SvxMacroAssignSingleTabDialog
{
public:
    SvxMacroAssignDlg(
        weld::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
        const SfxItemSet& rSet,
        const css::uno::Reference< css::container::XNameReplace >& xNameReplace,
        sal_uInt16 nSelectedIndex
    );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
