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
#ifndef _WRT_FN_HXX
#define _WRT_FN_HXX

#include "hintids.hxx"		// fuer die Konstanten

namespace binfilter {

class SfxPoolItem;
class SfxItemSet; 

// einige Forward-Deklarationen
class SwFmt;
class SwNode;
class SwCntntNode;
class Writer;


/* Funktionspointer auf die Attribut-Write-Funktionen */
typedef Writer& (*FnAttrOut)( Writer&, const SfxPoolItem& );
typedef FnAttrOut SwAttrFnTab[ POOLATTR_END - POOLATTR_BEGIN ];

Writer& Out( const SwAttrFnTab, const SfxPoolItem&, Writer& );
Writer& Out_SfxItemSet( const SwAttrFnTab, Writer&, const SfxItemSet&,
                         BOOL bDeep, BOOL bTstForDefault = TRUE );


/* Funktionspointer auf die Node-Write-Funktionen */

enum RES_NODE
{
RES_NODE_BEGIN = 0,
    RES_TXTNODE = RES_NODE_BEGIN,
    RES_GRFNODE,
    RES_OLENODE,
RES_NODE_END
};

typedef Writer& (*FnNodeOut)( Writer&, SwCntntNode& );
typedef FnNodeOut SwNodeFnTab[ RES_NODE_END - RES_NODE_BEGIN ];

Writer& Out( const SwNodeFnTab, SwNode&, Writer & rWrt );




} //namespace binfilter
#endif	//	_WRT_FN_HXX
