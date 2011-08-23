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

/**********************************************************************************************************************
 * @file
 *   index manager implementation
 *  Three record types are related with index information, and two ways are used.
*  1. VO_ROOTOBJINDEX works with one or more VO_LEAFOBJINDEX records to
*  provide the map of all object ids and their offsets.
*  VO_ROOTOBJINDEX includes k (object id, offset) and timetable which is used to map index to actual low id
 *  2. VO_ROOTLEAFOBJINDEX is used when the file is so small that the number of objects is less than 256(?)
 *  VO_ROOTLEAFOBJINDEX contains directly the (object id, offset) map and time table.

 * Change History
 Jan 2005			Created
**********************************************************************************************************************/

#include "lwpidxmgr.hxx"

const sal_uInt8 LwpIndexManager::MAXOBJECTIDS = 255;

LwpIndexManager::LwpIndexManager()
    :m_nKeyCount(0), m_nLeafCount(0)
{

    m_TempVec.resize( LwpIndexManager::MAXOBJECTIDS );

}

LwpIndexManager::~LwpIndexManager()
{
    //Clear m_ObjectKeys
    std::vector<LwpKey*>::iterator it;

    for( it = m_ObjectKeys.begin(); it != m_ObjectKeys.end(); it++ )
    {
        LwpKey* pKey = *it;
        if( pKey )
        {
            delete pKey;
            pKey = NULL;
        }
    }
    m_ObjectKeys.clear();
}

/**
 * @descr	Read all index records, VO_ROOTLEAFOBJINDEX, VO_ROOTOBJINDEX, VO_LEAFOBJINDEX
 */
void LwpIndexManager::Read(LwpSvStream* pStrm)
{
    //Read index obj
    LwpObjectHeader ObjHdr;
    ObjHdr.Read(*pStrm);
    LwpObjectStream* pObjStrm = new LwpObjectStream(pStrm, ObjHdr.IsCompressed(),
            static_cast<sal_uInt16>(ObjHdr.GetSize()) );

    if( ObjHdr.GetTag() == VO_ROOTLEAFOBJINDEX )
    {
        ReadLeafData(pObjStrm);
        ReadTimeTable(pObjStrm);
        delete pObjStrm;
    }
    else
    {
        ReadRootData(pObjStrm);
        delete pObjStrm;

        for (sal_uInt16 k = 0; k < m_nLeafCount; k++)
        {
            //Read leaf
            pStrm->Seek(m_ChildIndex[k]+LwpSvStream::LWP_STREAM_BASE);


            //Old Code
            //ReadLeafIndex(pStrm);
            //New Code
            ReadObjIndex( pStrm );


            //Read object in root, these objects are between the leaf objects
            if(k!=m_nLeafCount-1)
            {
                m_ObjectKeys.push_back(m_RootObjs[k]);
                m_nKeyCount ++;
            }
        }
        m_RootObjs.clear();
    }
}

/**
 * @descr	Read data in VO_ROOTOBJINDEX
 */
void LwpIndexManager::ReadRootData(LwpObjectStream* pObjStrm)
{

    sal_uInt16 KeyCount = 0;
    pObjStrm->QuickRead(&KeyCount, sizeof(KeyCount));
    m_nLeafCount = KeyCount + 1;

    if(KeyCount)
    {
        //read object keys
        LwpKey* akey = new LwpKey();
        akey->id.Read(pObjStrm);
        m_RootObjs.push_back(akey);


        //sal_uInt8 k = 0;
        sal_uInt16 k = 0;

        for (k = 1; k < KeyCount; k++)
        {
            akey = new LwpKey();
            akey->id.ReadCompressed(pObjStrm, m_RootObjs[k-1]->id);
            m_RootObjs.push_back(akey);
        }

        for (k = 0; k < KeyCount; k++)
        {
            pObjStrm->QuickRead(&(m_RootObjs[k]->offset), sizeof(sal_uInt32));
        }

        //read leaf index offset
        for (k = 0; k < m_nLeafCount; k++)
        {
            pObjStrm->QuickRead(&(m_ChildIndex[k]), sizeof(sal_uInt32));
        }
    }

    ReadTimeTable(pObjStrm);

}


//Add new method to handle ObjIndex data
/**
 * @descr	Read data in VO_OBJINDEX
 */
void LwpIndexManager::ReadObjIndexData(LwpObjectStream* pObjStrm)
{
    sal_uInt16 KeyCount = 0;
    sal_uInt16 LeafCount = 0;
    pObjStrm->QuickRead(&KeyCount, sizeof(KeyCount));
    LeafCount = KeyCount + 1;

    std::vector<LwpKey*> vObjIndexs;

    if(KeyCount)
    {
        LwpKey* akey = new LwpKey();
        akey->id.Read(pObjStrm);
        vObjIndexs.push_back(akey);

        sal_uInt16 k = 0;

        for (k = 1; k < KeyCount; k++)
        {
            akey = new LwpKey();
            akey->id.ReadCompressed(pObjStrm, vObjIndexs[k-1]->id);
            vObjIndexs.push_back(akey);
        }

        for (k = 0; k < KeyCount; k++)
            pObjStrm->QuickRead(&(vObjIndexs[k]->offset), sizeof(sal_uInt32));

        for (k = 0; k < LeafCount; k++)
            pObjStrm->QuickRead(&(m_TempVec[k]), sizeof(sal_uInt32));
    }

    for( sal_uInt16 j=0; j<LeafCount; j++ )
    {
        pObjStrm->GetStream()->Seek( m_TempVec[j]+LwpSvStream::LWP_STREAM_BASE);
        ReadLeafIndex(pObjStrm->GetStream());

        if(j!=LeafCount-1)
        {
            m_ObjectKeys.push_back(vObjIndexs[j]);

            m_nKeyCount ++;
        }
    }

    vObjIndexs.clear();
    m_TempVec.clear();
}


/**
 * @descr	Read VO_OBJINDEX
 */
void LwpIndexManager::ReadObjIndex( LwpSvStream *pStrm )
{

    LwpObjectHeader ObjHdr;
    ObjHdr.Read(*pStrm);
    LwpObjectStream* pObjStrm = new LwpObjectStream(pStrm, ObjHdr.IsCompressed(),
            static_cast<sal_uInt16>(ObjHdr.GetSize()) );

    if( (sal_uInt32)VO_OBJINDEX == ObjHdr.GetTag() )
    {
        ReadObjIndexData( pObjStrm );
    }
    else if( (sal_uInt32)VO_LEAFOBJINDEX == ObjHdr.GetTag() )
    {
        ReadLeafData(pObjStrm);
    }

    delete pObjStrm;
}


/**
 * @descr	Read VO_LEAFOBJINDEX
 */
void LwpIndexManager::ReadLeafIndex( LwpSvStream *pStrm )
{
    LwpObjectHeader ObjHdr;
    ObjHdr.Read(*pStrm);
    LwpObjectStream* pObjStrm = new LwpObjectStream(pStrm, ObjHdr.IsCompressed(),
            static_cast<sal_uInt16>(ObjHdr.GetSize()) );

    ReadLeafData(pObjStrm);

    delete pObjStrm;
}
/**
 * @descr	Read data in VO_LEAFOBJINDEX
 */
void LwpIndexManager::ReadLeafData( LwpObjectStream *pObjStrm )
{
    sal_uInt16 KeyCount;
    pObjStrm->QuickRead(&KeyCount, sizeof(KeyCount));

    if(KeyCount)
    {
        LwpKey* akey = new LwpKey();
        //read object keys: id & offset
        akey->id.Read(pObjStrm);
        m_ObjectKeys.push_back(akey);

        for (sal_uInt8 k = 1; k < KeyCount; k++)
        {
            akey = new LwpKey();
            akey->id.ReadCompressed(pObjStrm, m_ObjectKeys[m_nKeyCount+k-1]->id);
            m_ObjectKeys.push_back(akey);
        }

        for (sal_uInt8 j = 0; j < KeyCount; j++)
        {
            pObjStrm->QuickRead(&(m_ObjectKeys[m_nKeyCount+j]->offset), sizeof(sal_uInt32));
        }
    }
    m_nKeyCount += KeyCount;
}
/**
 * @descr	Read time table in VO_ROOTLEAFOBJINDEX and VO_ROOTOBJINDEX
 */
void LwpIndexManager::ReadTimeTable(LwpObjectStream *pObjStrm)
{
    sal_uInt16 nTimeCount;
    pObjStrm->QuickRead(&nTimeCount, sizeof(nTimeCount));

    sal_uInt32 atime;
    for(sal_uInt16 i=0; i<nTimeCount; i++)
    {
        pObjStrm->QuickRead( &atime, sizeof(atime) );
        m_TimeTable.push_back(atime);
    }
}
/**
 * @descr		get object offset per the object id
 */
sal_uInt32 LwpIndexManager::GetObjOffset( LwpObjectID objid )
{

    //sal_uInt16 L, U, M;
    sal_uInt32 L, U, M;


    L = 0;
    U = m_nKeyCount;
    while (L != U)
    {
        M = (L + U) >> 1;

        if (objid.GetLow() > m_ObjectKeys[M]->id.GetLow())
            L = M + 1;
        else if (objid.GetLow() < m_ObjectKeys[M]->id.GetLow())
            U = M;
        else if (objid.GetHigh() > m_ObjectKeys[M]->id.GetHigh())
            L = M + 1;
        else if (objid.GetHigh() < m_ObjectKeys[M]->id.GetHigh())
            U = M;
        else
        {
            return(m_ObjectKeys[M]->offset);
        }
    }
    return BAD_OFFSET;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
