/*************************************************************************
 *
 *  $RCSfile: disas.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
