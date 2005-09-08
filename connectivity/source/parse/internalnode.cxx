/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: internalnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:40:16 $
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
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include "internalnode.hxx"
#endif

#include <algorithm>
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

using namespace connectivity;

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const sal_Char* pNewValue,
                                   SQLNodeType eNodeType,
                                   sal_uInt32 nNodeID)
                 : OSQLParseNode(pNewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    OSQLParser::s_pGarbageCollector->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const ::rtl::OString &_NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(_NewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    OSQLParser::s_pGarbageCollector->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const sal_Unicode* pNewValue,
                                   SQLNodeType eNodeType,
                                   sal_uInt32 nNodeID)
                 :OSQLParseNode(pNewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    OSQLParser::s_pGarbageCollector->push_back(this);
}

//-----------------------------------------------------------------------------
OSQLInternalNode::OSQLInternalNode(const ::rtl::OUString &_NewValue,
                                 SQLNodeType eNodeType,
                                 sal_uInt32 nNodeID)
                :OSQLParseNode(_NewValue,eNodeType,nNodeID)
{
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    OSQLParser::s_pGarbageCollector->push_back(this);
}


//-----------------------------------------------------------------------------
OSQLInternalNode::~OSQLInternalNode()
{
    // remove the node from the garbage list
    OSL_ENSURE(OSQLParser::s_pGarbageCollector, "Collector not initialized");
    if (!OSQLParser::s_pGarbageCollector->empty())
    {
        OSQLParser::s_pGarbageCollector->erase(
        ::std::find(OSQLParser::s_pGarbageCollector->begin(), OSQLParser::s_pGarbageCollector->end(),
        this));
    }
}
