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

#ifndef _SAX_FASTATTRIBS_HXX_
#define _SAX_FASTATTRIBS_HXX_

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>

#include <cppuhelper/implbase1.hxx>
#include "sax/saxdllapi.h"

#include <map>
#include <vector>

namespace sax_fastparser
{

struct UnknownAttribute
{
    OUString maNamespaceURL;
    OString maName;
    OString maValue;

    UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const OString& rValue );

    UnknownAttribute( const OString& rName, const OString& rValue );

    void FillAttribute( ::com::sun::star::xml::Attribute* pAttrib ) const;
};

typedef std::map< sal_Int32, OString > FastAttributeMap;
typedef std::vector< UnknownAttribute > UnknownAttributeList;

class SAX_DLLPUBLIC FastAttributeList : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastAttributeList >
{
public:
    FastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xTokenHandler );
    virtual ~FastAttributeList();

    void clear();
    void add( sal_Int32 nToken, const OString& rValue );
    void addUnknown( const OUString& rNamespaceURL, const OString& rName, const OString& rValue );
    void addUnknown( const OString& rName, const OString& rValue );

    // XFastAttributeList
    virtual ::sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getValue( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::xml::Attribute > SAL_CALL getUnknownAttributes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::xml::FastAttribute > SAL_CALL getFastAttributes() throw (::com::sun::star::uno::RuntimeException);

private:
    FastAttributeMap maAttributes;
    UnknownAttributeList maUnknownAttributes;
    FastAttributeMap::iterator maLastIter;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler > mxTokenHandler;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
