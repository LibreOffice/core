/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxres.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:52:19 $
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

#ifndef _SBXRES_HXX
#define _SBXRES_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

// Zur Zeit sind Ressources im SVTOOLS-Projekt nicht vorgesehen.
// Da es sich um unkritische Ressourcen handelt (BASIC-Keywords),
// koennen wir mit Dummies arbeiten.

#define STRING_TYPES        0
#define STRING_EMPTY        0
#define STRING_NULL         1
#define STRING_INTEGER      2
#define STRING_LONG         3
#define STRING_SINGLE       4
#define STRING_DOUBLE       5
#define STRING_CURRENCY     6
#define STRING_DATE         7
#define STRING_STRING       8
#define STRING_OBJECT       9
#define STRING_ERROR        10
#define STRING_BOOL         11
#define STRING_VARIANT      12
#define STRING_ANY          13
#define STRING_CHAR         16
#define STRING_BYTE         17
#define STRING_USHORT       18
#define STRING_ULONG        19
#define STRING_INT          22
#define STRING_UINT         23
#define STRING_LPSTR        30
#define STRING_LPWSTR       31
#define STRING_AS           32
#define STRING_OPTIONAL     33
#define STRING_BYREF        34

#define STRING_NAMEPROP     35
#define STRING_PARENTPROP   36
#define STRING_APPLPROP     37
#define STRING_COUNTPROP    38
#define STRING_ADDMETH      39
#define STRING_ITEMMETH     40
#define STRING_REMOVEMETH   41

#define STRING_ERRORMSG     42
#define STRING_FALSE        43
#define STRING_TRUE         44

#define SBXRES_MAX          44

class SbxRes : public String
{
public:
    SbxRes( USHORT );
};

const char* GetSbxRes( USHORT );


#endif
