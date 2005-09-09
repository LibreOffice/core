/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementmark.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:16:21 $
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

#ifndef _ELEMENTMARK_HXX
#define _ELEMENTMARK_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_ELEMENTMARKTYPE_HPP_
#include <com/sun/star/xml/crypto/sax/ElementMarkType.hpp>
#endif

class BufferNode;

class ElementMark
/****** elementmark.hxx/CLASS ElementMark *************************************
 *
 *   NAME
 *  ElementMark -- Class to manipulate an element mark
 *
 *   FUNCTION
 *  This class maintains the security id, buffer id and its type for a
 *  buffer node.
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
protected:
    /* the BufferNode maintained by this object */
    BufferNode* m_pBufferNode;

    /* the security Id */
    sal_Int32 m_nSecurityId;

    /* the buffer Id */
    sal_Int32 m_nBufferId;

    /*
     * the type value, is one of following values:
     * TYPEOFELEMENTMARK - the default value, represents an blocker if
     *                     not changed
     * TYPEOFELEMENTCOLLECTOR - represents an ElementCollector
     */
    com::sun::star::xml::crypto::sax::ElementMarkType m_type;

public:
    ElementMark(sal_Int32 nSecurityId, sal_Int32 nBufferId);
    virtual ~ElementMark() {};

    BufferNode* getBufferNode() const;
    void setBufferNode(const BufferNode* pBufferNode);
    sal_Int32 getSecurityId() const;
    void setSecurityId(sal_Int32 nSecurityId);
    com::sun::star::xml::crypto::sax::ElementMarkType getType() const;
    sal_Int32 getBufferId() const;
};

#endif

