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

class SwTxtFrm;     // SwTxtFrmInfo
class SwPaM;        // SwTxtFrmInfo
class SwTxtCursor;  // SwTxtFrmInfo


/*************************************************************************
 *                      class SwTxtFrmInfo
 *************************************************************************/

class SwTxtFrmInfo
{
    const SwTxtFrm *pFrm;

    // Wo beginnt der Text (ohne whitespaces)? (Dokument global !!)
    SwTwips GetLineStart( const SwTxtCursor &rLine ) const;

public:
    inline SwTxtFrmInfo( const SwTxtFrm *pTxtFrm ) : pFrm(pTxtFrm) { }

    // Passt der Absatz in eine Zeile?
    BOOL IsOneLine() const;

    // Ist die Zeile zu X% gefuellt?
    BOOL IsFilled( const BYTE nPercent ) const;

    // Wo beginnt der Text (ohne whitespaces)? (rel. im Frame !!)
    SwTwips GetLineStart() const;

    //returne die mittel Position des n. Charakters
    SwTwips GetCharPos( xub_StrLen nChar, BOOL bCenter = TRUE ) const;

    // Sammelt die whitespaces am Zeilenbeginn und -ende im Pam
    void GetSpaces( SwPaM &rPam, BOOL bWithLineBreak ) const;

    // Ist an der ersten Textposition ein Bullet/Symbol etc?
    BOOL IsBullet( xub_StrLen nTxtPos ) const;

    // Ermittelt Erstzeileneinzug
    SwTwips GetFirstIndent() const;

    // setze und erfrage den Frame;
    const SwTxtFrm* GetFrm() const { return pFrm; }
    SwTxtFrmInfo& SetFrm( const SwTxtFrm* pNew )
        { pFrm = pNew; return *this; }

    // liegt eine Gegenueberstellung vor? (returnt Pos im Frame)
    USHORT GetBigIndent( xub_StrLen& rFndPos,
                        const SwTxtFrm *pNextFrm = 0 ) const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
