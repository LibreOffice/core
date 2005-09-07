/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: disas.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:32:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DISAS_HXX
#define _DISAS_HXX

#include "image.hxx"
#include "opcodes.hxx"

class SvStream;

class SbiDisas {
    const SbiImage& rImg;
    SbModule* pMod;
    char     cLabels[ 8192 ];       // Bitvektor fuer Labels
    USHORT   nOff;                  // aktuelle Position
    USHORT   nPC;                   // Position des Opcodes
    SbiOpcode eOp;                  // Opcode
    USHORT   nOp1, nOp2;            // Operanden
    short    nParts;                // 1, 2 oder 3
    short    nLine;                 // aktuelle Zeile
    BOOL     DisasLine( String& );
    BOOL     Fetch();               // naechster Opcode
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
