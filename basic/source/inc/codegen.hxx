/*************************************************************************
 *
 *  $RCSfile: codegen.hxx,v $
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

#ifndef _CODEGEN_HXX
#define _CODEGEN_HXX

class SbiImage;
class SbiParser;
class SbModule;
#ifndef _OPCODES_HXX
#include "opcodes.hxx"
#endif
#ifndef _BUFFER_HXX
#include "buffer.hxx"
#endif

class SbiCodeGen {              // Code-Erzeugung:
    SbiParser* pParser;         // fuer Fehlermeldungen, Line, Column etc.
    SbModule& rMod;             // aktuelles Modul
    SbiBuffer aCode;                // Code-Puffer
    short  nLine, nCol;         // Zeile, Spalte fuer Stmnt-Befehl
    short  nForLevel;           // #29955 for-Schleifen-Ebene
    BOOL bStmnt;                // TRUE: Statement-Opcode liegt an
public:
    SbiCodeGen( SbModule&, SbiParser*, short );
    SbiParser* GetParser() { return pParser; }
    USHORT Gen( SbiOpcode );
    USHORT Gen( SbiOpcode, UINT16 );
    USHORT Gen( SbiOpcode, UINT16, UINT16 );
    void Patch( USHORT o, USHORT v ){ aCode.Patch( o, v ); }
    void BackChain( USHORT off )    { aCode.Chain( off );  }
    void Statement();
    void GenStmnt();            // evtl. Statement-Opcode erzeugen
    USHORT GetPC();
    USHORT GetOffset()              { return GetPC() + 1; }
    void Save();

    // #29955 for-Schleifen-Ebene pflegen
    void IncForLevel( void ) { nForLevel++; }
    void DecForLevel( void ) { nForLevel--; }
};

#endif
