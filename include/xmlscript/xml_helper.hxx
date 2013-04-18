/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#define _XMLSCRIPT_XML_HELPER_HXX_

#include <vector>
#include <rtl/byteseq.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include "xmlscript/xmlscriptdllapi.h"

namespace xmlscript
{

/*##################################################################################################

    EXPORTING

##################################################################################################*/

//==================================================================================================
class XMLSCRIPT_DLLPUBLIC XMLElement
    : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XAttributeList >
{
public:
    inline XMLElement( OUString const & name )
        SAL_THROW(())
        : _name( name )
        {}

    /** Adds a sub element of element.

        @param xElem element reference
    */
    void SAL_CALL addSubElement(
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > const & xElem )
        SAL_THROW(());

    /** Gets sub element of given index.  The index follows order in which sub elements were added.

        @param nIndex index of sub element
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > SAL_CALL getSubElement( sal_Int32 nIndex )
        SAL_THROW(());

    /** Adds an attribute to elements.

        @param rAttrName qname of attribute
        @param rValue value string of element
    */
    void SAL_CALL addAttribute( OUString const & rAttrName, OUString const & rValue )
        SAL_THROW(());

    /** Gets the tag name (qname) of element.

        @return
                qname of element
    */
    inline OUString SAL_CALL getName() SAL_THROW(())
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
    virtual OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTypeByName( OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getValueByName( OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    OUString _name;

    ::std::vector< OUString > _attrNames;
    ::std::vector< OUString > _attrValues;

    ::std::vector< ::com::sun::star::uno::Reference<
                   ::com::sun::star::xml::sax::XAttributeList > > _subElems;
};


/*##################################################################################################

    STREAMING

##################################################################################################*/

XMLSCRIPT_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
SAL_CALL createInputStream(
    ::rtl::ByteSequence const & rInData )
    SAL_THROW(());

XMLSCRIPT_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
SAL_CALL createOutputStream(
    ::rtl::ByteSequence * pOutData )
    SAL_THROW(());

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
