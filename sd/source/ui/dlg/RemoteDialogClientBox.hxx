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

#include "rtl/ustring.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/fixed.hxx"
#include "vcl/dialog.hxx"
#include "vcl/field.hxx"

#include "svtools/extensionlistbox.hxx"
#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/deployment/XPackage.hpp"

#include <boost/shared_ptr.hpp>

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

typedef ::boost::shared_ptr< ClientBoxEntry > TClientBoxEntry;

struct ClientBoxEntry
{
    bool            m_bActive       :1;
    ClientInfo*     m_pClientInfo;


    ClientBoxEntry( ClientInfo* pClientInfo );
   ~ClientBoxEntry();

};


// class ExtensionBox_Impl
class ClientBox;


class ClientRemovedListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
    ClientBox   *m_pParent;

public:

    ClientRemovedListener( ClientBox *pParent ) { m_pParent = pParent; }
   ~ClientRemovedListener();


    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException, std::exception);
};

class ClientBox:
    public Control
{
    bool            m_bHasScrollBar;
    bool            m_bHasActive;
    bool            m_bNeedsRecalc;
    bool            m_bInCheckMode;
    bool            m_bAdjustActive;
    bool            m_bInDelete;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    long            m_nActive;
    long            m_nTopIndex;
    long            m_nStdHeight;
    long            m_nActiveHeight;
    long            m_nExtraHeight;
    Size            m_aOutputSize;
    Link            m_aClickHdl;

    NumericBox      m_aPinBox;

    ScrollBar       m_aScrollBar;

    com::sun::star::uno::Reference< ClientRemovedListener > m_xRemoveListener;

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

    void            CalcActiveHeight( const long nPos );
    long            GetTotalHeight() const;
    void            SetupScrollBar();
    void            DrawRow( const Rectangle& rRect, const TClientBoxEntry pEntry );
    bool            HandleTabKey( bool bReverse );
    bool            HandleCursorKey( sal_uInt16 nKeyCode );
    void            DeleteRemoved();


    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar* );

    //Index starts with 1.
    //Throws an com::sun::star::lang::IllegalArgumentException, when the index is invalid.
    void checkIndex(sal_Int32 pos) const;


public:
                    ClientBox( Window* pParent, WinBits nStyle );
                   ~ClientBox();

    void    MouseButtonDown( const MouseEvent& rMEvt );
    void    Paint( const Rectangle &rPaintRect );
    void    Resize();
    Size    GetOptimalSize() const;
    bool    Notify( NotifyEvent& rNEvt );

    const Size      GetMinOutputSizePixel() const;
    void            SetExtraSize( long nSize ) { m_nExtraHeight = nSize; }
    TClientBoxEntry     GetEntryData( long nPos ) { return m_vEntries[ nPos ]; }
    long            GetActiveEntryIndex();
    long            GetEntryCount() { return (long) m_vEntries.size(); }
    Rectangle       GetEntryRect( const long nPos ) const;
    bool            HasActive() { return m_bHasActive; }
    long            PointToPos( const Point& rPos );
    void            SetScrollHdl( const Link& rLink );
    void            DoScroll( long nDelta );
    void            SetHyperlinkHdl( const Link& rLink ){ m_aClickHdl = rLink; }
    void    RecalcAll();
    void            RemoveUnlocked();


    void    selectEntry( const long nPos );
    long            addEntry( ClientInfo* pClientInfo );
    void            updateEntry( const ClientInfo* rPackageInfo );
    void            removeEntry( const ClientInfo* rPackageInfo );

    void            prepareChecking();
    void            checkEntries();

    OUString getPin();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
