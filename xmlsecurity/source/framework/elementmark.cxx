/*************************************************************************
 *
 *  $RCSfile: elementmark.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "elementmark.hxx"
#include "buffernode.hxx"

namespace cssxc = com::sun::star::xml::crypto;

ElementMark::ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId)
    :m_nSecurityId(nSecurityId),
     m_nBufferId(nBufferId),
     m_pBufferNode(NULL),
     m_type(cssxc::sax::ElementMarkType_TYPEOFELEMENTMARK)
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



