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

#ifndef _SAX_FASTATTRIBS_HXX_
#define _SAX_FASTATTRIBS_HXX_

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>

#include <cppuhelper/implbase1.hxx>
#include "sax/dllapi.h"

#include <map>
#include <vector>

namespace sax_fastparser
{

struct UnknownAttribute
{
    ::rtl::OUString maNamespaceURL;
    ::rtl::OString maName;
    ::rtl::OString maValue;

    UnknownAttribute( const ::rtl::OUString& rNamespaceURL, const ::rtl::OString& rName, const ::rtl::OString& rValue );

    UnknownAttribute( const ::rtl::OString& rName, const ::rtl::OString& rValue );

    void FillAttribute( ::com::sun::star::xml::Attribute* pAttrib ) const;
};

typedef std::map< sal_Int32, ::rtl::OString > FastAttributeMap;
typedef std::vector< UnknownAttribute > UnknownAttributeList;

class SAX_DLLPUBLIC FastAttributeList : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastAttributeList >
{
public:
    FastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xTokenHandler );
    virtual ~FastAttributeList();

    void clear();
    void add( sal_Int32 nToken, const ::rtl::OString& rValue );
    void addUnknown( const ::rtl::OUString& rNamespaceURL, const ::rtl::OString& rName, const ::rtl::OString& rValue );
    void addUnknown( const ::rtl::OString& rName, const ::rtl::OString& rValue );

    // XFastAttributeList
    virtual ::sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValue( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
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
