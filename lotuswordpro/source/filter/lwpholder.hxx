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
 *  kinds of holder classes header file
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPDLVLISTHEADHOLDER_HXX_
#define _LWPDLVLISTHEADHOLDER_HXX_

#include "lwpobj.hxx"
#include "lwpdlvlist.hxx"
/**
 * @brief	VO_HEADHOLDER, LwpDLVListHeadHolder,
 * 		contains an id to the head of LwpDLVList
*/
class LwpDLVListHeadHolder : public LwpObject
{
public:
    LwpDLVListHeadHolder(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpDLVListHeadHolder(){};
    void Read();
    LwpObjectID* GetHeadID() { return &m_DLVHead;}
private:
    LwpObjectID m_DLVHead;
};
/**
 * @brief	VO_HEADTAILHOLDER, LwpDLVListHeadTailHolder,
 * 		contains a LwpDLVListHeadTail (Head and tail id)
*/
class LwpDLVListHeadTailHolder : public LwpObject
{
public:
    LwpDLVListHeadTailHolder(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpDLVListHeadTailHolder(){};
    void Read();
    LwpObjectID* GetHead(){ return m_HeadTail.GetHead();}
    LwpObjectID* GetTail(){ return m_HeadTail.GetTail();}
protected:
    LwpDLVListHeadTail m_HeadTail;
};
/**
 * @brief	LwpObjectHolder, which is LwpDLVList, contains an id to an object
 *		VO_OBJECTHOLDER
*/
class LwpObjectHolder : public LwpDLVList
{
public:
    LwpObjectHolder(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpObjectHolder(){};
    void Read();
    LwpObjectID* GetObject(){return &m_Object;}
protected:
    LwpObjectID m_Object;
};

/**
 * @brief		VO_LISTLIST object in .lwp file
 *
 */
class LwpListList : public LwpObjectHolder
{
public:
    LwpListList(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpListList(){};
    void Read();
    LwpObjectID* GetHead(){ return m_HeadTail.GetHead();}
    LwpObjectID* GetTail(){ return m_HeadTail.GetTail();}
protected:
    LwpDLVListHeadTail m_HeadTail;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
