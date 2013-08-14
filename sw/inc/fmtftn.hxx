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
#ifndef _FMTFTN_HXX
#define _FMTFTN_HXX

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"

class SwDoc;
class SwTxtFtn;

// ATT_FTN

class SW_DLLPUBLIC SwFmtFtn: public SfxPoolItem
{
    friend class SwTxtFtn;
    SwTxtFtn* m_pTxtAttr;   ///< My TextAttribute.
    OUString m_aNumber;     ///< User-defined 'Number'.
    sal_uInt16 m_nNumber;   ///< Automatische Nummerierung
    bool    m_bEndNote;     ///< Is it an End note?

    /// Protected CopyCtor.
    SwFmtFtn& operator=(const SwFmtFtn& rFtn);
    SwFmtFtn( const SwFmtFtn& );

public:
    SwFmtFtn( bool bEndNote = false );
    virtual ~SwFmtFtn();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    OUString   GetNumStr() const { return m_aNumber; }
    sal_uInt16 GetNumber() const { return m_nNumber; }
    bool       IsEndNote() const { return m_bEndNote;}

    void SetNumStr( const OUString& rStr ) { m_aNumber = rStr; }
    void SetNumber( sal_uInt16 nNo )       { m_nNumber = nNo; }
    void SetEndNote( bool b );

    void SetNumber( const SwFmtFtn& rFtn )
    {
        m_nNumber = rFtn.m_nNumber;
        m_aNumber = rFtn.m_aNumber;
    }

    const SwTxtFtn *GetTxtFtn() const   { return m_pTxtAttr; }
          SwTxtFtn *GetTxtFtn()         { return m_pTxtAttr; }

    void GetFtnText( OUString& rStr ) const;

    /// Returns string to be displayed of footnote / endnote.
    OUString GetViewNumStr( const SwDoc& rDoc, sal_Bool bInclStrs = sal_False ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
