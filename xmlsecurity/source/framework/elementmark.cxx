/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementmark.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:16:06 $
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

#include "elementmark.hxx"
#include "buffernode.hxx"

namespace cssxc = com::sun::star::xml::crypto;

ElementMark::ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId)
    :m_nSecurityId(nSecurityId),
     m_nBufferId(nBufferId),
     m_pBufferNode(NULL),
     m_type(cssxc::sax::ElementMarkType_ELEMENTMARK)
/****** ElementMark/ElementMark ***********************************************
 *
 *   NAME
 *  ElementMark -- constructor method
 *
 *   SYNOPSIS
 *  ElementMark(nSecurityId, nBufferId);
 *
 *   FUNCTION
 *  construct an ElementMark object.
 *
 *   INPUTS
 *  nSecurityId -   represents which security entity the buffer node is
 *          related with. Either a signature or an encryption is
 *          a security entity.
 *  nBufferId - the id of the element bufferred in the document
 *          wrapper component. The document wrapper component
 *          uses this id to search the particular bufferred
 *          element.
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
}

BufferNode* ElementMark::getBufferNode() const
{
    return m_pBufferNode;
}

void ElementMark::setBufferNode(const BufferNode* pBufferNode)
{
    m_pBufferNode = (BufferNode*)pBufferNode;
}

sal_Int32 ElementMark::getSecurityId() const
{
    return m_nSecurityId;
}

void ElementMark::setSecurityId(sal_Int32 nSecurityId)
{
    m_nSecurityId = nSecurityId;
}

com::sun::star::xml::crypto::sax::ElementMarkType ElementMark::getType() const
{
    return m_type;
}

sal_Int32 ElementMark::getBufferId() const
{
    return m_nBufferId;
}



