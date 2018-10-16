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
#ifndef INCLUDED_SW_INC_FMTFTN_HXX
#define INCLUDED_SW_INC_FMTFTN_HXX

#include <rtl/ustring.hxx>
#include <cppuhelper/weakref.hxx>
#include <svl/poolitem.hxx>

#include "swdllapi.h"
#include "calbck.hxx"

namespace com { namespace sun { namespace star {
    namespace text { class XFootnote; }
} } }

class SwDoc;
class SwTextFootnote;

// ATT_FTN

class SW_DLLPUBLIC SwFormatFootnote
    : public SfxPoolItem
    , public SwModify
    , public sw::BroadcasterMixin
{
    friend class SwTextFootnote;
    SwTextFootnote* m_pTextAttr;   ///< My TextAttribute.
    OUString m_aNumber;     ///< User-defined 'Number'.
    sal_uInt16 m_nNumber;   ///< automatic sequence number
    sal_uInt16 m_nNumberRLHidden; ///< automatic sequence number (hidden redlines)
    bool    m_bEndNote;     ///< Is it an End note?

    css::uno::WeakReference<css::text::XFootnote> m_wXFootnote;

    SwFormatFootnote& operator=(const SwFormatFootnote& rFootnote) = delete;
    SwFormatFootnote( const SwFormatFootnote& ) = delete;

public:
    SwFormatFootnote( bool bEndNote = false );
    virtual ~SwFormatFootnote() override;

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;

    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew)
        override;

    void InvalidateFootnote();

    const OUString& GetNumStr() const { return m_aNumber; }
    sal_uInt16 GetNumber() const { return m_nNumber; }
    sal_uInt16 GetNumberRLHidden() const { return m_nNumberRLHidden; }
    bool       IsEndNote() const { return m_bEndNote;}

    void SetNumStr( const OUString& rStr ) { m_aNumber = rStr; }
    void SetEndNote( bool b );

    void SetNumber( const SwFormatFootnote& rFootnote )
    {
        m_nNumber = rFootnote.m_nNumber;
        m_nNumberRLHidden = rFootnote.m_nNumberRLHidden;
        m_aNumber = rFootnote.m_aNumber;
    }

    const SwTextFootnote *GetTextFootnote() const   { return m_pTextAttr; }
          SwTextFootnote *GetTextFootnote()         { return m_pTextAttr; }

    void GetFootnoteText( OUString& rStr ) const;

    /// Returns string to be displayed of footnote / endnote.
    OUString GetViewNumStr( const SwDoc& rDoc, bool bInclStrs = false ) const;

    css::uno::WeakReference<css::text::XFootnote> const& GetXFootnote() const
        { return m_wXFootnote; }
    void SetXFootnote(css::uno::Reference<css::text::XFootnote> const& xNote)
        { m_wXFootnote = xNote; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
