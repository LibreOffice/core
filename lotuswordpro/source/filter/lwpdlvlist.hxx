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
 *  list related class
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef LWPDLVLIST_HXX_
#define LWPDLVLIST_HXX_

#include "lwpatomholder.hxx"
#include "lwpobj.hxx"
/**
 * @brief	Double Linked Virtual List
*/
class LwpDLVList : public LwpObject
{
public:
    LwpDLVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpDLVList(){};
protected:
    LwpObjectID m_ListPrevious;
    LwpObjectID m_ListNext;
protected:
    void Read();
public:
    inline LwpObjectID* GetNext();
    inline LwpObjectID* GetPrevious();
};
LwpObjectID* LwpDLVList::GetNext()
{
    return &m_ListNext;
}
LwpObjectID* LwpDLVList::GetPrevious()
{
    return &m_ListPrevious;
}
/**
 * @brief	Double Linked Named Family Virtual List
*/
class LwpDLNFVList : public LwpDLVList
{
public:
    LwpDLNFVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpDLNFVList(){};
protected:
    LwpObjectID m_ChildHead;
    LwpObjectID m_ChildTail;
    LwpObjectID m_Parent;
    LwpAtomHolder m_Name;
protected:
    void Read();
    void ReadName(LwpObjectStream* pObjStrm);
public:
    // 01/17/2005
    inline LwpAtomHolder* GetName();
    //end
    inline LwpObjectID* GetChildHead();
    inline LwpObjectID* GetChildTail();
    inline LwpObjectID* GetParent();
};

LwpAtomHolder* LwpDLNFVList::GetName()
{
    return &m_Name;
}

LwpObjectID* LwpDLNFVList::GetChildHead()
{
    return &m_ChildHead;
}

LwpObjectID* LwpDLNFVList::GetChildTail()
{
    return &m_ChildTail;
}

LwpObjectID* LwpDLNFVList::GetParent()
{
    return &m_Parent;
}

class LwpPropList;
/**
 * @brief	Double Linked Named Family Properties Virtual List
*/
class LwpDLNFPVList : public LwpDLNFVList
{
public:
    LwpDLNFPVList(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpDLNFPVList();
protected:
    sal_Bool m_bHasProperties;
    LwpPropList* m_pPropList;
protected:
    void Read();
    void ReadPropertyList(LwpObjectStream* pObjStrm);
public:
    inline LwpPropList* GetPropList();
};

LwpPropList* LwpDLNFPVList::GetPropList()
{
    return m_pPropList;
}

/**
 * @brief	Double Linked Virtual List Head Tail
*/
class LwpDLVListHeadTail
{
public:
    LwpDLVListHeadTail(){};
    void Read(LwpObjectStream* pObjStrm);
private:
    LwpObjectID m_ListHead;
    LwpObjectID m_ListTail;
public:
    inline LwpObjectID* GetHead();
    inline LwpObjectID* GetTail();
};
LwpObjectID* LwpDLVListHeadTail::GetHead()
{
    return &m_ListHead;
}
LwpObjectID* LwpDLVListHeadTail::GetTail()
{
    return &m_ListTail;
}

/**
 * @brief	Double Linked Virtual List Head
*/
class LwpDLVListHead
{
public:
    LwpDLVListHead(){};
    ~LwpDLVListHead(){};
    void Read(LwpObjectStream* pObjStrm);
    inline LwpObjectID* GetFirst();
protected:
    LwpObjectID m_objHead;//LwpDLVList
};
LwpObjectID* LwpDLVListHead::GetFirst()
{
    return &m_objHead;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
