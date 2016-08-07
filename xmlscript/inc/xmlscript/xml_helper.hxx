/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#define _XMLSCRIPT_XML_HELPER_HXX_

#include <vector>
#include <rtl/byteseq.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

/*##################################################################################################

    EXPORTING

##################################################################################################*/

//==================================================================================================
class XCR_DLLPUBLIC XMLElement
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
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
SAL_CALL createInputStream(
    ::rtl::ByteSequence const & rInData )
    SAL_THROW( () );

//==================================================================================================
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
SAL_CALL createOutputStream(
    ::rtl::ByteSequence * pOutData )
    SAL_THROW( () );

}

#endif
