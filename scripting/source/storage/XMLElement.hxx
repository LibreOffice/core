/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
         throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByName( ::rtl::OUString const & rName )
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByName( ::rtl::OUString const & rName )
        throw ( css::uno::RuntimeException );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
