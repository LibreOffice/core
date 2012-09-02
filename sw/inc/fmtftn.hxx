/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FMTFTN_HXX
#define _FMTFTN_HXX

#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"

class SwDoc;
class SwTxtFtn;

// ATT_FTN

class SW_DLLPUBLIC SwFmtFtn: public SfxPoolItem
{
    friend class SwTxtFtn;
    SwTxtFtn* pTxtAttr;     ///< My TextAttribute.
    String  aNumber;        ///< User-defined 'Number'.
    sal_uInt16  nNumber;    ///< Automatische Nummerierung
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

    const String &GetNumStr() const { return aNumber; }
    const sal_uInt16 &GetNumber() const { return nNumber; }
          bool    IsEndNote() const { return m_bEndNote;}

    void SetNumStr( const String& rStr )    { aNumber = rStr; }
    void SetNumber( sal_uInt16 nNo )            { nNumber = nNo; }
    void SetEndNote( bool b );

    void SetNumber( const SwFmtFtn& rFtn )
        { nNumber = rFtn.nNumber; aNumber = rFtn.aNumber; }

    const SwTxtFtn *GetTxtFtn() const   { return pTxtAttr; }
          SwTxtFtn *GetTxtFtn()         { return pTxtAttr; }

    void GetFtnText( String& rStr ) const;

    /// Returns string to be displayed of footnote / endnote.
    String GetViewNumStr( const SwDoc& rDoc, sal_Bool bInclStrs = sal_False ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
