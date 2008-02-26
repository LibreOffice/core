/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: text.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:50:18 $
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

#include "text.hxx"
namespace DOM
{
    CText::CText(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_TEXT_NODE;
        init_characterdata(aNodePtr);
    }

    void SAL_CALL CText::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        i_xHandler->characters(getData());
    }

    void CText::init_text(const xmlNodePtr aNodePtr)
    {
        init_characterdata(aNodePtr);
    }

  Reference< XText > SAL_CALL CText::splitText(sal_Int32 /*offset*/)
             throw (RuntimeException)
    {
        return Reference< XText >(this);
    }

    OUString SAL_CALL CText::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#text");
    }

}
