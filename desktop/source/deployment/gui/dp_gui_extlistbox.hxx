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

#include <rtl/ustring.hxx>
#include <vcl/image.hxx>
#include <vcl/weld/ScrolledWindow.hxx>
#include <vcl/weld/customweld.hxx>
#include <vcl/weld/weld.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/deployment/XPackage.hpp>

#include <memory>
#include <optional>

#include "dp_gui.h"

namespace dp_gui {

#define SMALL_ICON_SIZE     16
#define TOP_OFFSET           5
#define ICON_HEIGHT         42
#define ICON_WIDTH          47
#define ICON_OFFSET         72
#define RIGHT_ICON_OFFSET    5
#define SPACE_BETWEEN        3

class TheExtensionManager;

struct Entry_Impl
{
    bool            m_bLocked       :1;
    bool            m_bHasOptions   :1;
    bool            m_bUser         :1;
    bool            m_bShared       :1;
    bool            m_bNew          :1;
    bool            m_bChecked      :1;
    bool            m_bMissingDeps  :1;
    bool            m_bHasButtons   :1;
    bool            m_bMissingLic   :1;
    PackageState    m_eState;
    OUString        m_sTitle;
    OUString        m_sVersion;
    OUString        m_sDescription;
    OUString        m_sPublisher;
    OUString        m_sPublisherURL;
    OUString        m_sErrorText;
    OUString        m_sLicenseText;
    Image           m_aIcon;
    tools::Rectangle m_aLinkRect;

    css::uno::Reference<css::deployment::XPackage> m_xPackage;

    Entry_Impl(const css::uno::Reference<css::deployment::XPackage> &xPackage,
               const PackageState eState, const bool bReadOnly);
    ~Entry_Impl();

    sal_Int32 CompareTo(const CollatorWrapper& rCollator, const Entry_Impl& rEntry) const;
    void checkDependencies();
};

typedef std::shared_ptr<Entry_Impl> TEntry_Impl;

class ExtensionBox;

class ExtensionRemovedListener : public ::cppu::WeakImplHelper<css::lang::XEventListener>
{
    ExtensionBox* m_pParent;

public:
    explicit ExtensionRemovedListener(ExtensionBox* pParent) { m_pParent = pParent; }
    virtual ~ExtensionRemovedListener() override;


    // XEventListener
    virtual void SAL_CALL disposing(css::lang::EventObject const& evt) override;
};

class ExtensionBox : public weld::CustomWidgetController
{
    bool m_bHasScrollBar : 1;
    bool m_bNeedsRecalc : 1;
    bool m_bInCheckMode : 1;
    bool m_bAdjustActive : 1;
    bool m_bInDelete : 1;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    tools::Long m_nActive;
    tools::Long m_nTopIndex;
    tools::Long m_nStdHeight;
    tools::Long m_nActiveHeight;
    Image m_aSharedImage;
    Image m_aLockedImage;
    Image m_aWarningImage;
    Image m_aDefaultImage;

    rtl::Reference<ExtensionRemovedListener> m_xRemoveListener;

    TheExtensionManager& m_rManager;
    //This mutex is used for synchronizing access to m_vEntries.
    //Currently it is used to synchronize adding, removing entries and
    //functions like getItemName, getItemDescription, etc. to prevent
    //that m_vEntries is accessed at an invalid index.
    //ToDo: There are many more places where m_vEntries is read and which may
    //fail. For example the Paint method is probable called from the main thread
    //while new entries are added / removed in a separate thread.
    mutable ::osl::Mutex    m_entriesMutex;
    std::vector< TEntry_Impl > m_vEntries;
    std::vector< TEntry_Impl > m_vRemovedEntries;

    std::optional<CollatorWrapper>   m_oCollator;

    //Holds weak references to extensions to which is we have added an XEventListener
    std::vector< css::uno::WeakReference<
        css::deployment::XPackage> > m_vListenerAdded;

    std::unique_ptr<weld::ScrolledWindow> m_xScrollBar;

    //Removes the dead weak references from m_vListenerAdded
    void cleanVecListenerAdded();
    void addEventListenerOnce(css::uno::Reference<css::deployment::XPackage> const & extension);

    void CalcActiveHeight( const tools::Long nPos );
    tools::Long GetTotalHeight() const;
    void SetupScrollBar();
    void DrawRow(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect,
                 const TEntry_Impl& rEntry, bool bActive);
    bool HandleCursorKey( sal_uInt16 nKeyCode );
    bool FindEntryPos( const TEntry_Impl& rEntry, tools::Long nStart, tools::Long nEnd, tools::Long &nFound );
    void DeleteRemoved();

    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );

    void Init();
public:
    explicit ExtensionBox(std::unique_ptr<weld::ScrolledWindow> xScroll,
                          TheExtensionManager& rManager);
    virtual ~ExtensionBox() override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle &rPaintRect ) override;
    virtual void Resize() override;
    virtual OUString RequestHelp(tools::Rectangle& rRect) override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    TEntry_Impl const & GetEntryData( tools::Long nPos ) { return m_vEntries[ nPos ]; }
    tools::Long            GetEntryCount() const { return static_cast<tools::Long>(m_vEntries.size()); }
    tools::Rectangle GetEntryRect(tools::Long nPos) const;
    bool            HasActive() const { return m_nActive >= 0; }
    tools::Long            PointToPos( const Point& rPos );
    virtual void    RecalcAll();
    void            RemoveUnlocked();


    virtual void selectEntry( const tools::Long nPos );
    void addEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage,
                              bool bLicenseMissing = false );
    void updateEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage );
    void removeEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage );

    void prepareChecking();
    void checkEntries();

public:
    enum { ENTRY_NOTFOUND = -1 };

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, then this function returns ENTRY_NOTFOUND. */
    sal_Int32 getSelIndex() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
