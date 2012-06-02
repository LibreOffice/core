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

#ifndef _FRMINF_HXX
#define _FRMINF_HXX

#include "swtypes.hxx"

class SwTxtFrm;
class SwPaM;
class SwTxtCursor;

class SwTxtFrmInfo
{
    const SwTxtFrm *pFrm;

    // Where does the text (w/o whitespaces) start (document is global!)?
    SwTwips GetLineStart( const SwTxtCursor &rLine ) const;

public:
    inline SwTxtFrmInfo( const SwTxtFrm *pTxtFrm ) : pFrm(pTxtFrm) { }

    // Does the paragraph fit into a single line?
    sal_Bool IsOneLine() const;

    // Is the line filled to X%?
    sal_Bool IsFilled( const sal_uInt8 nPercent ) const;

    // Where does the text (w/o whitespaces) start (rel. in frame)?
    SwTwips GetLineStart() const;

    // return center position of the next character
    SwTwips GetCharPos( xub_StrLen nChar, sal_Bool bCenter = sal_True ) const;

    // collect all whitespaces at the beginning and end of a line in Pam
    void GetSpaces( SwPaM &rPam, sal_Bool bWithLineBreak ) const;

    // Is a bullet point/symbol/etc. at the first text position?
    sal_Bool IsBullet( xub_StrLen nTxtPos ) const;

    // determine intentation for first line
    SwTwips GetFirstIndent() const;

    const SwTxtFrm* GetFrm() const { return pFrm; }
    SwTxtFrmInfo& SetFrm( const SwTxtFrm* pNew )
        { pFrm = pNew; return *this; }

    // Is it a comparison? Returns position in frame.
    sal_uInt16 GetBigIndent( xub_StrLen& rFndPos,
                        const SwTxtFrm *pNextFrm = 0 ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
