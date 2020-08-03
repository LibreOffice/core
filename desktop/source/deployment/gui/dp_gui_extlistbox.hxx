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
#include <vcl/customweld.hxx>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/deployment/XPackage.hpp>

#include <memory>

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


struct Entry_Impl;

typedef std::shared_ptr< Entry_Impl > TEntry_Impl;

struct Entry_Impl
{
    bool            m_bActive       :1;
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

    sal_Int32 CompareTo(const CollatorWrapper *pCollator, const TEntry_Impl& rEntry) const;
    void checkDependencies();
};

class ExtensionBox_Impl;


class ExtensionRemovedListener : public ::cppu::WeakImplHelper<css::lang::XEventListener>
{
    ExtensionBox_Impl*   m_pParent;

public:

    explicit ExtensionRemovedListener( ExtensionBox_Impl *pParent ) { m_pParent = pParent; }
    virtual ~ExtensionRemovedListener() override;


    // XEventListener
    virtual void SAL_CALL disposing(css::lang::EventObject const& evt) override;
};

class ExtensionBox_Impl : public weld::CustomWidgetController
{
    bool m_bHasScrollBar : 1;
    bool m_bHasActive : 1;
    bool m_bNeedsRecalc : 1;
    bool m_bInCheckMode : 1;
    bool m_bAdjustActive : 1;
    bool m_bInDelete : 1;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    long m_nActive;
    long m_nTopIndex;
    long m_nStdHeight;
    long m_nActiveHeight;
    Image m_aSharedImage;
    Image m_aLockedImage;
    Image m_aWarningImage;
    Image m_aDefaultImage;

    rtl::Reference<ExtensionRemovedListener> m_xRemoveListener;

    TheExtensionManager      *m_pManager;
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

    std::unique_ptr<css::lang::Locale> m_pLocale;
    std::unique_ptr<CollatorWrapper>   m_pCollator;

    //Holds weak references to extensions to which is we have added an XEventListener
    std::vector< css::uno::WeakReference<
        css::deployment::XPackage> > m_vListenerAdded;

    std::unique_ptr<weld::ScrolledWindow> m_xScrollBar;

    //Removes the dead weak references from m_vListenerAdded
    void cleanVecListenerAdded();
    void addEventListenerOnce(css::uno::Reference<css::deployment::XPackage> const & extension);

    void CalcActiveHeight( const long nPos );
    long GetTotalHeight() const;
    void SetupScrollBar();
    void DrawRow(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const TEntry_Impl& rEntry);
    bool HandleCursorKey( sal_uInt16 nKeyCode );
    bool FindEntryPos( const TEntry_Impl& rEntry, long nStart, long nEnd, long &nFound );
    void DeleteRemoved();

    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );

    void Init();
public:
    explicit ExtensionBox_Impl(std::unique_ptr<weld::ScrolledWindow> xScroll);
    virtual ~ExtensionBox_Impl() override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle &rPaintRect ) override;
    virtual void Resize() override;
    virtual OUString RequestHelp(tools::Rectangle& rRect) override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    TEntry_Impl const & GetEntryData( long nPos ) { return m_vEntries[ nPos ]; }
    long            GetEntryCount() const { return static_cast<long>(m_vEntries.size()); }
    tools::Rectangle       GetEntryRect( const long nPos ) const;
    bool            HasActive() const { return m_bHasActive; }
    long            PointToPos( const Point& rPos );
    virtual void    RecalcAll();
    void            RemoveUnlocked();


    virtual void selectEntry( const long nPos );
    void addEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage,
                              bool bLicenseMissing = false );
    void updateEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage );
    void removeEntry(const css::uno::Reference<css::deployment::XPackage> &xPackage );

    void prepareChecking();
    void checkEntries();

    void setExtensionManager(TheExtensionManager* pManager) { m_pManager = pManager; }

    //These functions are used for automatic testing
public:
    enum { ENTRY_NOTFOUND = -1 };

    /** @return  The count of the entries in the list box. */
    sal_Int32 getItemCount() const;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns ENTRY_NOTFOUND */
    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns ENTRY_NOTFOUND */
    sal_Int32 getSelIndex() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
