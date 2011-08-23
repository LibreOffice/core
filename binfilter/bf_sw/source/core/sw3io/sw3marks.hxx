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
#ifndef _SW3MARKS_HXX
#define _SW3MARKS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _CNTNRSRT_HXX
#include <bf_svtools/cntnrsrt.hxx>
#endif
namespace binfilter {

enum Sw3MarkType {
    SW3_TOX_POINT, SW3_TOX_MARK,
    SW3_BOOK_POINT, SW3_BOOK_MARK,
    SW3_REDLINE_START, SW3_REDLINE_END,
    SW3_MARKTYPE_END
};

class Sw3Mark
{
    friend int sw3mark_compare( const Sw3Mark& r1, const Sw3Mark& r2 );

    Sw3MarkType	eType;					// Art des Eintrags
    ULONG		nNodePos;				// Node-Index
    xub_StrLen	nNodeOff;				// Position-Index
    USHORT		nId;					// ID des Eintrags

public:

    Sw3Mark() : eType(SW3_TOX_POINT), nNodePos(0),nNodeOff(0), nId(0) {}
    Sw3Mark( Sw3Mark& r ) : eType(r.eType), nNodePos(r.nNodePos),
                            nNodeOff(r.nNodeOff), nId(r.nId) {}

    Sw3MarkType	GetType() const 	{ return  eType; }
    ULONG		GetNodePos() const 	{ return  nNodePos; }
    xub_StrLen	GetNodeOff() const 	{ return  nNodeOff; }
    USHORT		GetId() const 		{ return  nId; }

    void SetType( Sw3MarkType nSet ){ eType = nSet; }
    void SetNodePos( ULONG nSet ) 	{ nNodePos = nSet; }
    void SetNodeOff( xub_StrLen nSet ) 	{ nNodeOff = nSet; }
    void SetId( USHORT nSet ) 		{ nId = nSet; }
};

DECLARE_CONTAINER_SORT_DEL( Sw3Marks, Sw3Mark )

} //namespace binfilter
#endif
