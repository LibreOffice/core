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

#ifndef _DISAS_HXX
#define _DISAS_HXX

#include "image.hxx"
#include "opcodes.hxx"
// find a place for this limit ( also used in
class SvStream;
#define MAX_LABELS 0x2000L
class SbiDisas {
    const SbiImage& rImg;
    SbModule* pMod;
    char     cLabels[ MAX_LABELS ];     // Bitvektor fuer Labels
    sal_uInt32   nOff;                  // aktuelle Position
    sal_uInt32   nPC;                   // Position des Opcodes
    SbiOpcode eOp;                  // Opcode
    sal_uInt32   nOp1, nOp2;            // Operanden
    sal_uInt32   nParts;                // 1, 2 oder 3
    sal_uInt32   nLine;                 // aktuelle Zeile
    sal_Bool     DisasLine( String& );
    sal_Bool     Fetch();               // naechster Opcode
public:
    SbiDisas( SbModule*, const SbiImage* );
    void Disas( SvStream& );
    void Disas( String& );
                                    // NICHT AUFRUFEN
    void     StrOp( String& );
    void     Str2Op( String& );
    void     ImmOp( String& );
    void     OnOp( String& );
    void     LblOp( String& );
    void     ReturnOp( String& );
    void     ResumeOp( String& );
    void     PromptOp( String& );
    void     CloseOp( String& );
    void     CharOp( String& );
    void     VarOp( String& );
    void     VarDefOp( String& );
    void     OffOp( String& );
    void     TypeOp( String& );
    void     CaseOp( String& );
    void     StmntOp( String& );
    void     StrmOp( String& );
};

#endif
