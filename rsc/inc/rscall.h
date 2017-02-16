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
#ifndef INCLUDED_RSC_INC_RSCALL_H
#define INCLUDED_RSC_INC_RSCALL_H

#include <rsctools.hxx>
#include <rscerror.h>
#include <rscdef.hxx>
#include <rschash.hxx>
#include <rtl/alloc.h>
#include <o3tl/typed_flags_set.hxx>

/******************* T y p e s *******************************************/
typedef char * CLASS_DATA;  // Zeiger auf die Daten einer Klasse

/******************* C l a s s e s   F o r w a r d s *********************/
class RscCompiler;
class RscTop;
class RscTypCont;

/******************* G l o b a l   V a r i a b l e s *********************/
extern OString* pStdParType;
extern OString* pStdPar1;
extern OString* pStdPar2;
extern sal_uInt32       nRefDeep;
extern sal_uInt32       nRsc_DELTALANG;
extern sal_uInt32       nRsc_DELTASYSTEM;
extern AtomContainer*   pHS;

/******************* D e f i n e s ***************************************/

enum class CommandFlags {
    NONE         = 0x0000,
    Help         = 0x0001,  // Hilfe anzeigen
    NoPrePro     = 0x0002,  // kein Preprozesor
    NoSyntax     = 0x0004,  // keine Syntaxanalyse
    NoLink       = 0x0008,  // nicht linken
    NoResFile    = 0x0010,  // keine .res-Datei erzeugen
    Define       = 0x0020,  // es wurde Definitionen angegeben
    Include      = 0x0040,  // der Include-Pfad wurde erweitert
    Preload      = 0x0200,  // Alle Resourcen Preloaden
    SrsDefault   = 0x1000,  // immer der Default geschrieben
    NoSysResTest = 0x2000   // ueberprueft nicht die Richtigkeit von (bmp, ico, cur)
};
namespace o3tl {
    template<> struct typed_flags<CommandFlags> : is_typed_flags<CommandFlags, 0x327f> {};
}


/******************* T y p e s *******************************************/
enum RSCCLASS_TYPE  { RSCCLASS_BOOL, RSCCLASS_STRING, RSCCLASS_NUMBER,
                      RSCCLASS_CONST, RSCCLASS_COMPLEX, RSCCLASS_ENUMARRAY };

typedef void (* VarEnumCallbackProc)( void * pData, RSCCLASS_TYPE, Atom );

/******************* S t r u c t s ***************************************/
struct RSCINST
{
    RscTop *    pClass;
    CLASS_DATA  pData;

                RSCINST(){ pClass = nullptr; pData = nullptr; }
                RSCINST( RscTop * pCl, CLASS_DATA pClassData )
                    {
                        pClass = pCl;
                        pData = pClassData;
                    }
    bool        IsInst() const { return( pData != nullptr ); }
};

/********************** S U B I N F O S T R U C T ************************/
struct SUBINFO_STRUCT
{
    SUBINFO_STRUCT(){ nPos = 0; pClass = nullptr; };
    RscId        aId;    // Identifier der Resource
    sal_uInt32   nPos;   // Position der Resource
    RscTop *     pClass; // Klasse des Eintrages
};

/******************* F u n c t i o n *************************************/
void InitRscCompiler();

#endif // INCLUDED_RSC_INC_RSCALL_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
