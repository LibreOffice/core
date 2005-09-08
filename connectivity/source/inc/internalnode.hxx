/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: internalnode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:40:56 $
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
#define _CONNECTIVITY_SQLINTERNALNODE_HXX

#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif

namespace connectivity
{
    //==========================================================================
    //= OSQLInternalNode
    //==========================================================================
    /** special node for avoiding memory leaks
    */
    class OSQLInternalNode : public OSQLParseNode
    {
    public:
        OSQLInternalNode(const sal_Char* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const sal_Unicode* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OUString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);

        virtual ~OSQLInternalNode();
    };
}

#endif  //_CONNECTIVITY_SQLINTERNALNODE_HXX
