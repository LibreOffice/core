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
/*****************************************************************************
* Change History
* <<Date>> <<Name of editor>> <<Description>>
    2005/2		draft code for Implementation of chart xml hander
****************************************************************************/
/**
* @file
*  For LWP filter architecture prototype
*/

#include "xfchartxmlhandler.hxx"

Any SAL_CALL XFChartXMLHandler::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface( rType ,SAL_STATIC_CAST( XDocumentHandler*, this ));
    if ( a.hasValue() )
        return a;
    else
        return OWeakObject::queryInterface( rType );
}

/**
* @short   constructor of chart xml handler
* @descr
* @param   pOutputStream the output stream
*/
XFChartXMLHandler::XFChartXMLHandler(IXFStream* pOutputStream)
{
    m_pOutputStream = pOutputStream;
}

/**
* @short   handle the start element
* @descr   ignore some elements here
* @param   aName	element name
* @param   xAttribs the attribute list
*/
void SAL_CALL XFChartXMLHandler::startElement( const ::rtl::OUString& aName, const Reference< XAttributeList >& xAttribs )
throw (SAXException, RuntimeException)
{
    IXFAttrList* pList = m_pOutputStream->GetAttrList();
    pList->Clear();
    {
        INT16 nLen = xAttribs->getLength();
        for( INT16 nC=0;nC<nLen;nC++)
        {
            OUString aName = xAttribs->getNameByIndex(nC);
            OUString aVal = xAttribs->getValueByIndex(nC);
            pList->AddAttribute(aName, aVal);
        }
    }

    m_pOutputStream->StartElement(aName);
}

/**
* @short   handle the end element
* @descr   ignore some elements here
* @param   aName	element name
*/
void SAL_CALL XFChartXMLHandler::endElement( const ::rtl::OUString& aName )
throw (SAXException, RuntimeException)
{
    m_pOutputStream->EndElement(aName);
}

void SAL_CALL XFChartXMLHandler::characters( const ::rtl::OUString& aChars )
throw (SAXException, RuntimeException)
{
    m_pOutputStream->Characters(aChars);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
