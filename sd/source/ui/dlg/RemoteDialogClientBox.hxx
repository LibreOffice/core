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

#include <rtl/ustring.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>

#include <memory>

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

    explicit ClientBoxEntry(const std::shared_ptr<ClientInfo>& pClientInfo);
   ~ClientBoxEntry();

};

// class ExtensionBox_Impl

class ClientBox : public Control
{
    bool m_bHasScrollBar : 1;
    bool m_bHasActive : 1;
    bool m_bNeedsRecalc : 1;
    bool m_bAdjustActive : 1;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    long m_nActive;
    long m_nTopIndex;
    long m_nStdHeight;
    long m_nActiveHeight;

    VclPtr<NumericBox> m_aPinBox;
    VclPtr<PushButton> m_aDeauthoriseButton;
    ::tools::Rectangle m_sPinTextRect;

    VclPtr<ScrollBar> m_aScrollBar;

    //This mutex is used for synchronizing access to m_vEntries.
    //Currently it is used to synchronize adding, removing entries and
    //functions like getItemName, getItemDescription, etc. to prevent
    //that m_vEntries is accessed at an invalid index.
    //ToDo: There are many more places where m_vEntries is read and which may
    //fail. For example the Paint method is probable called from the main thread
    //while new entries are added / removed in a separate thread.
    mutable ::osl::Mutex    m_entriesMutex;
    std::vector< TClientBoxEntry > m_vEntries;

    void CalcActiveHeight();
    long GetTotalHeight() const;
    void SetupScrollBar();
    void DrawRow(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, const TClientBoxEntry& rEntry);
    bool HandleCursorKey( sal_uInt16 nKeyCode );

    DECL_LINK( ScrollHdl, ScrollBar*, void );
    DECL_LINK( DeauthoriseHdl, Button*, void );

public:
    ClientBox( vcl::Window* pParent, WinBits nStyle );
    virtual ~ClientBox() override;
    virtual void dispose() override;

    void MouseButtonDown( const MouseEvent& rMEvt ) override;
    void Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle &rPaintRect ) override;
    void Resize() override;
    Size GetOptimalSize() const override;
    bool EventNotify( NotifyEvent& rNEvt ) override;

    TClientBoxEntry const & GetEntryData( long nPos ) { return m_vEntries[ nPos ]; }
    long GetActiveEntryIndex();
    ::tools::Rectangle GetEntryRect( const long nPos ) const;
    long PointToPos( const Point& rPos );
    void DoScroll( long nDelta );
    void RecalcAll();

    void selectEntry( const long nPos );
    void addEntry(const std::shared_ptr<ClientInfo>& pClientInfo);
    void clearEntries();

    OUString getPin();
    void populateEntries();
};

} // end namespace sd

#endif // INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOGCLIENTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
