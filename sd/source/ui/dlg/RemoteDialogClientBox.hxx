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

#ifndef INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOGCLIENTBOX_HXX
#define INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOGCLIENTBOX_HXX

#include "rtl/ustring.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/fixed.hxx"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/field.hxx"

#include "svtools/extensionlistbox.hxx"
#include <cppuhelper/implbase.hxx>

#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/deployment/XPackage.hpp"

#include <memory>

#include "sdresid.hxx"

namespace sd {

#define SMALL_ICON_SIZE     16
#define TOP_OFFSET           5
#define ICON_HEIGHT         42
#define ICON_WIDTH          47
#define ICON_OFFSET         72
#define RIGHT_ICON_OFFSET    5
#define SPACE_BETWEEN        3

// struct ClientBoxEntry
struct ClientBoxEntry;
struct ClientInfo;

typedef std::shared_ptr<ClientBoxEntry> TClientBoxEntry;

struct ClientBoxEntry
{
    bool m_bActive :1;
    std::shared_ptr<ClientInfo> m_pClientInfo;

    ClientBoxEntry(std::shared_ptr<ClientInfo> pClientInfo);
   ~ClientBoxEntry();

};

// class ExtensionBox_Impl
class ClientBox;

class ClientRemovedListener : public ::cppu::WeakImplHelper<css::lang::XEventListener>
{
    VclPtr<ClientBox> m_pParent;

public:

    ClientRemovedListener(ClientBox *pParent)
    {
        m_pParent = pParent;
    }
    virtual ~ClientRemovedListener();

    // XEventListener
    virtual void SAL_CALL disposing(css::lang::EventObject const & evt)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ClientBox : public Control
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
    long m_nExtraHeight;
    Size            m_aOutputSize;

    VclPtr<NumericBox> m_aPinBox;
    VclPtr<PushButton> m_aDeauthoriseButton;
    Rectangle m_sPinTextRect;

    VclPtr<ScrollBar> m_aScrollBar;

    css::uno::Reference< ClientRemovedListener > m_xRemoveListener;

    //This mutex is used for synchronizing access to m_vEntries.
    //Currently it is used to synchronize adding, removing entries and
    //functions like getItemName, getItemDescription, etc. to prevent
    //that m_vEntries is accessed at an invalid index.
    //ToDo: There are many more places where m_vEntries is read and which may
    //fail. For example the Paint method is probable called from the main thread
    //while new entries are added / removed in a separate thread.
    mutable ::osl::Mutex    m_entriesMutex;
    std::vector< TClientBoxEntry > m_vEntries;
    std::vector< TClientBoxEntry > m_vRemovedEntries;

    void CalcActiveHeight( const long nPos );
    long GetTotalHeight() const;
    void SetupScrollBar();
    void DrawRow(vcl::RenderContext& rRenderContext, const Rectangle& rRect, const TClientBoxEntry& rEntry);
    bool HandleTabKey( bool bReverse );
    bool HandleCursorKey( sal_uInt16 nKeyCode );
    void DeleteRemoved();

    DECL_DLLPRIVATE_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    DECL_DLLPRIVATE_LINK_TYPED( DeauthoriseHdl, Button*, void );
    //Index starts with 1.
    //Throws an com::sun::star::lang::IllegalArgumentException, when the index is invalid.
    void checkIndex(sal_Int32 pos) const;

public:
    ClientBox( vcl::Window* pParent, WinBits nStyle );
    virtual ~ClientBox();
    virtual void dispose() SAL_OVERRIDE;

    void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    void Paint( vcl::RenderContext& rRenderContext, const Rectangle &rPaintRect ) SAL_OVERRIDE;
    void Resize() SAL_OVERRIDE;
    Size GetOptimalSize() const SAL_OVERRIDE;
    bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    const Size GetMinOutputSizePixel() const;
    void SetExtraSize( long nSize ) { m_nExtraHeight = nSize; }
    TClientBoxEntry GetEntryData( long nPos ) { return m_vEntries[ nPos ]; }
    long GetActiveEntryIndex();
    long GetEntryCount() { return (long) m_vEntries.size(); }
    Rectangle GetEntryRect( const long nPos ) const;
    bool HasActive() { return m_bHasActive; }
    long PointToPos( const Point& rPos );
    void SetScrollHdl( const Link<>& rLink );
    void DoScroll( long nDelta );
    void RecalcAll();
    void RemoveUnlocked();

    void selectEntry( const long nPos );
    long addEntry(std::shared_ptr<ClientInfo> pClientInfo);
    void clearEntries();

    void prepareChecking();
    void checkEntries();

    OUString getPin();
    void populateEntries();
};

} // end namespace sd

#endif // INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOGCLIENTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
