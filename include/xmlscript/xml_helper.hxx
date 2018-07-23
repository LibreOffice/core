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
#ifndef INCLUDED_XMLSCRIPT_XML_HELPER_HXX
#define INCLUDED_XMLSCRIPT_XML_HELPER_HXX

#include <vector>
#include <rtl/byteseq.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <xmlscript/xmlscriptdllapi.h>

namespace xmlscript
{

/*##################################################################################################

    EXPORTING

##################################################################################################*/


class XMLSCRIPT_DLLPUBLIC XMLElement
    : public cppu::WeakImplHelper< css::xml::sax::XAttributeList >
{
public:
    XMLElement( OUString const & name )
        : _name( name )
        {}

    /** Adds a sub element of element.

        @param xElem element reference
    */
    void addSubElement(
        css::uno::Reference< css::xml::sax::XAttributeList > const & xElem );

    /** Gets sub element of given index.  The index follows order in which sub elements were added.

        @param nIndex index of sub element
    */
    css::uno::Reference< css::xml::sax::XAttributeList > const & getSubElement( sal_Int32 nIndex );

    /** Adds an attribute to elements.

        @param rAttrName qname of attribute
        @param rValue value string of element
    */
    void addAttribute( OUString const & rAttrName, OUString const & rValue );

    /** Dumps out element (and all sub elements).

        @param xOut document handler to be written to
    */
    void dump(
        css::uno::Reference< css::xml::sax::XDocumentHandler > const & xOut );
    /** Dumps out sub elements (and all further sub elements).

        @param xOut document handler to be written to
    */
    void dumpSubElements(
        css::uno::Reference< css::xml::sax::XDocumentHandler > const & xOut );

    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength() override;
    virtual OUString SAL_CALL getNameByIndex( sal_Int16 nPos ) override;
    virtual OUString SAL_CALL getTypeByIndex( sal_Int16 nPos ) override;
    virtual OUString SAL_CALL getTypeByName( OUString const & rName ) override;
    virtual OUString SAL_CALL getValueByIndex( sal_Int16 nPos ) override;
    virtual OUString SAL_CALL getValueByName( OUString const & rName ) override;

protected:
    ::std::vector< css::uno::Reference<
                      css::xml::sax::XAttributeList > > _subElems;
private:
    OUString const _name;
    ::std::vector< OUString > _attrNames;
    ::std::vector< OUString > _attrValues;

};


/*##################################################################################################

    STREAMING

##################################################################################################*/

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::io::XInputStream >
createInputStream(
    std::vector<sal_Int8> const & rInData );

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::io::XInputStream >
createInputStream(
    const sal_Int8* pData, int len );

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::io::XOutputStream >
createOutputStream(
    std::vector<sal_Int8> * pOutData );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
