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
