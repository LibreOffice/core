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

#ifndef _NDTYP_HXX
#define _NDTYP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

// Ids fuer die verschiedenden Nodes; in der Basisklasse steht der Member,
// der angibt, um was fuer einen es sich handelt
const BYTE ND_ENDNODE		= 0x01;
const BYTE ND_STARTNODE		= 0x02;
const BYTE ND_TABLENODE		= 0x06;
const BYTE ND_TEXTNODE 		= 0x08;
const BYTE ND_GRFNODE 		= 0x10;
const BYTE ND_OLENODE 		= 0x20;

const BYTE ND_CONTENTNODE	= 0x38;	// ContentNode (eines von den 3 Bits)
const BYTE ND_NOTXTNODE		= 0x30;	// NoTxtNode (eines von den 2 Bits)

const BYTE ND_SECTIONNODE   = 0x42;
// nur fuer internen Gebrauch!!
const BYTE ND_SECTIONDUMMY  = 0x40; //(ND_SECTIONNODE & ~ND_STARTNODE);

// spezielle Types der StartNodes, die keine Ableitungen sind, aber
// "Bereiche" zusammenhalten.
enum SwStartNodeType
{
    SwNormalStartNode = 0,
    SwTableBoxStartNode,
    SwFlyStartNode,
    SwFootnoteStartNode,
    SwHeaderStartNode,
    SwFooterStartNode
};

// is the node the first and/or last node of a section?
// This information is used for the export filters. Our layout never have a
// distance before or after if the node is the first or last in a section.
const BYTE ND_HAS_PREV_LAYNODE = 0x01;
const BYTE ND_HAS_NEXT_LAYNODE = 0x02;



} //namespace binfilter
#endif
