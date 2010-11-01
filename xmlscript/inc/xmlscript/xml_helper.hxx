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
#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#define _XMLSCRIPT_XML_HELPER_HXX_

#include <vector>
#include <rtl/byteseq.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


namespace xmlscript
{

/*##################################################################################################

    EXPORTING

##################################################################################################*/

//==================================================================================================
class XMLElement
    : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XAttributeList >
{
public:
    inline XMLElement( ::rtl::OUString const & name )
        SAL_THROW( () )
        : _name( name )
        {}

    /** Adds a sub element of element.

        @param xElem element reference
    */
    void SAL_CALL addSubElement(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > const & xElem )
        SAL_THROW( () );

    /** Gets sub element of given index.  The index follows order in which sub elements were added.

        @param nIndex index of sub element
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > SAL_CALL getSubElement( sal_Int32 nIndex )
        SAL_THROW( () );

    /** Adds an attribute to elements.

        @param rAttrName qname of attribute
        @param rValue value string of element
    */
    void SAL_CALL addAttribute( ::rtl::OUString const & rAttrName, ::rtl::OUString const & rValue )
        SAL_THROW( () );

    /** Gets the tag name (qname) of element.

        @return
                qname of element
    */
    inline ::rtl::OUString SAL_CALL getName() SAL_THROW( () )
        { return _name; }

    /** Dumps out element (and all sub elements).

        @param xOut document handler to be written to
    */
    void SAL_CALL dump(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > const & xOut );
    /** Dumps out sub elements (and all further sub elements).

        @param xOut document handler to be written to
    */
    void SAL_CALL dumpSubElements(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > const & xOut );

    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName( ::rtl::OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName( ::rtl::OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    ::rtl::OUString _name;

    ::std::vector< ::rtl::OUString > _attrNames;
    ::std::vector< ::rtl::OUString > _attrValues;

    ::std::vector< ::com::sun::star::uno::Reference<
                   ::com::sun::star::xml::sax::XAttributeList > > _subElems;
};


/*##################################################################################################

    STREAMING

##################################################################################################*/

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
SAL_CALL createInputStream(
    ::rtl::ByteSequence const & rInData )
    SAL_THROW( () );

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
SAL_CALL createOutputStream(
    ::rtl::ByteSequence * pOutData )
    SAL_THROW( () );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
