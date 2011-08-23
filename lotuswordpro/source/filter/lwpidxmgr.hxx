/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  index manager, to maintain the map between object id and offset
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPINDEXMANAGER_HXX
#define _LWPINDEXMANAGER_HXX

#include <rtl/ustring.hxx>
#include "lwpobjhdr.hxx"
#include "lwpobjstrm.hxx"

/**
 * @brief	key structure used to map id to offset
*/
struct LwpKey
{
    LwpObjectID id;
    sal_uInt32 offset;
};
/**
 * @brief	LwpIndexManager, to read all index records and maintain the index information
*/
class LwpIndexManager
{
public:
    LwpIndexManager();
    ~LwpIndexManager();
protected:
    static const sal_uInt8 MAXOBJECTIDS;
    std::vector<LwpKey*> m_ObjectKeys;	//the <id, offset> ordered vector
    std::vector<LwpKey*> m_RootObjs;		//For those object ids in RootObjIndex
    std::vector<sal_uInt32> m_TimeTable;	//Time table for object low id compression

    //sal_uInt16 m_nKeyCount;
    sal_uInt32 m_nKeyCount;		//the count of all object
    std::vector<sal_uInt32> m_TempVec;	// a temp vector , store the offset of leafindex

    sal_uInt32 m_ChildIndex[256];	//Offset of leaf index
    sal_uInt16 m_nLeafCount;
protected:
    void ReadRootData(LwpObjectStream *pObjStrm );	//Read root index data


    //Add new method to handle ObjIndex
    void ReadObjIndexData(LwpObjectStream* pObjStrm);
    void ReadObjIndex( LwpSvStream *pStrm );	//Read Obj index


    void ReadLeafIndex( LwpSvStream *pStrm );	//Read leaf index obj
    void ReadLeafData( LwpObjectStream *pStrm );	//Read leaf index obj data
    void ReadTimeTable( LwpObjectStream *pStrm );
public:
    void Read( LwpSvStream *pStrm );
    sal_uInt32 GetObjOffset( LwpObjectID objid );
    sal_uInt32 GetObjTime( sal_uInt16 index ) { return m_TimeTable.at(index-1); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
