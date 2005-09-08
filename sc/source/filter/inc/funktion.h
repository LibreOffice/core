/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: funktion.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:20:06 $
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

#ifndef __FUNKTION_H__
#define __FUNKTION_H__

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

