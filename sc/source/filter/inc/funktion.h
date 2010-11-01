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

#ifndef SC_FUNKTION_H
#define SC_FUNKTION_H

// Bearbeitungsfunktionen
void P0( void );    // 0 Parameter
void P1( void );    // 1 Parameter
void P2( void );    // 2 Parameter
void P3( void );    // 3 Parameter
void P4( void );    // 4 Parameter
void P5( void );    // 5 Parameter
void Pn( void );    // n Parameter
void NI( void );    // nicht implementiert
void ConstFloat( void );    //    0
void Variable( void );      //    1
void LotusRange( void );    //    2
void FormulaReturn( void ); //    3
void Klammer( void );       //    4
void ConstInt( void );      //    5
void ConstString( void );   //    6
// ACHTUNG: unbekannte Funktionen -> P0()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
