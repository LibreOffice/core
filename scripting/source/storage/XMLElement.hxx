/*************************************************************************
 *
 *  $RCSfile: XMLElement.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: npower $ $Date: 2002-10-01 10:45:14 $
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
#ifndef _SCRIPTING_XML_ELEMENT_HXX_
#define _SCRIPTING_XML_ELEMENT_HXX_

#include <vector>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

/*##################################################################################################

 EXPORTING

##################################################################################################*/

//==================================================================================================
class XMLElement : public ::cppu::WeakImplHelper1< css::xml::sax::XAttributeList >
{
public:
    inline XMLElement( ::rtl::OUString const & name, ::rtl::OUString const & chars )
    SAL_THROW( () )
            : _name( name ), _chars( chars )
    {
    }

    inline XMLElement( ::rtl::OUString const & name )
    SAL_THROW( () )
            : _name( name )
    {
    }

    /** Adds a sub element of element.

     @param xElem element reference
    */
    void SAL_CALL addSubElement(
        css::uno::Reference< css::xml::sax::XAttributeList > const & xElem )
        SAL_THROW( () );

    /** Gets sub element of given index.  The index follows order in which sub elements were added.

     @param nIndex index of sub element
    */
    css::uno::Reference< css::xml::sax::XAttributeList > SAL_CALL getSubElement(
        sal_Int32 nIndex )
        SAL_THROW( () );

    /** Adds an attribute to elements.

     @param rAttrName qname of attribute
     @param rValue value string of element
    */
    void SAL_CALL addAttribute( ::rtl::OUString const & rAttrName,
        ::rtl::OUString const & rValue )
        SAL_THROW( () );

    /** Gets the tag name (qname) of element.

     @return
       qname of element
    */
    inline ::rtl::OUString SAL_CALL getName() SAL_THROW( () )
    {
        return _name;
    }

    /** Dumps out element (and all sub elements).

     @param xOut document handler to be written to
    */
    void SAL_CALL dump(
        css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > const & xOut );
    /** Dumps out sub elements (and all further sub elements).

     @param xOut document handler to be written to
    */
    void SAL_CALL dumpSubElements(
        css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > const & xOut );

    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
    throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
    throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
    throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName( ::rtl::OUString const & rName )
    throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
    throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName( ::rtl::OUString const & rName )
    throw (css::uno::RuntimeException);

protected:
    ::rtl::OUString _name;

    ::rtl::OUString _chars;

    ::std::vector< ::rtl::OUString > _attrNames;
    ::std::vector< ::rtl::OUString > _attrValues;

    ::std::vector< css::uno::Reference<
    css::xml::sax::XAttributeList > > _subElems;
};

}

#endif
