/*************************************************************************
 *
 *  $RCSfile: sbintern.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef _SB_INTERN_HXX
#define _SB_INTERN_HXX

#include <svtools/sbxfac.hxx>
#include "sb.hxx"

class SbUnoFactory;
class SbiInstance;
class SbModule;


class SbiFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

// Stack fuer die im Fehlerfall abgebaute SbiRuntime Kette
class SbErrorStackEntry
{
public:
    SbErrorStackEntry(SbMethodRef aM, xub_StrLen nL, xub_StrLen nC1, xub_StrLen nC2)
        : aMethod(aM), nLine(nL), nCol1(nC1), nCol2(nC2) {}
    SbMethodRef aMethod;
    xub_StrLen nLine;
    xub_StrLen nCol1, nCol2;
};

SV_DECL_PTRARR_DEL(SbErrorStack, SbErrorStackEntry*, 1, 1)



struct SbiGlobals
{
    SbiInstance*    pInst;          // alle aktiven Runtime-Instanzen
    SbiFactory*     pSbFac;         // StarBASIC-Factory
    SbUnoFactory*   pUnoFac;        // Factory fuer Uno-Structs bei DIM AS NEW
    SbModule*       pMod;           // aktuell aktives Modul
    SbModule*       pCompMod;       // aktuell compiliertes Modul
    short           nInst;          // Anzahl BASICs
    Link            aErrHdl;        // globaler Error-Handler
    Link            aBreakHdl;      // globaler Break-Handler
    SbError         nCode;          // aktueller Fehlercode
    xub_StrLen      nLine;          // aktuelle Zeile
    xub_StrLen      nCol1,nCol2;    // aktuelle Spalten (von,bis)
    BOOL            bCompiler;      // Flag fuer Compiler-Error
    BOOL            bCompWait;      // Flag, beim Compilieren WaitCursor eingeschaltet ist
    BOOL            bGlobalInitErr; // Beim GlobalInit trat ein Compiler-Fehler auf
    BOOL            bRunInit;       // TRUE, wenn RunInit vom Basic aktiv ist
    String          aErrMsg;        // Puffer fuer GetErrorText()
    SbLanguageMode  eLanguageMode;  // Flag fuer Visual-Basic-Script-Modus
    SbErrorStack*   pErrStack;      // Stack fuer die im Fehlerfall abgebaute SbiRuntime Kette

    SbiGlobals();
    ~SbiGlobals();
};

// Utility-Makros und -Routinen

SbiGlobals* GetSbData();

#define pINST       GetSbData()->pInst
#define pMOD        GetSbData()->pMod
#define pCMOD       GetSbData()->pCompMod
#define pSBFAC      GetSbData()->pSbFac
#define pUNOFAC     GetSbData()->pUnoFac

#endif

