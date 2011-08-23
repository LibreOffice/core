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

#ifndef _XMLOFF_ATTRLIST_HXX
#define _XMLOFF_ATTRLIST_HXX

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase3.hxx>
namespace binfilter {

struct SvXMLAttributeList_Impl;

class SvXMLAttributeList : public ::cppu::WeakImplHelper3<
        ::com::sun::star::xml::sax::XAttributeList,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::lang::XUnoTunnel>
{
    SvXMLAttributeList_Impl *m_pImpl;

public:
    SvXMLAttributeList();
    SvXMLAttributeList( const SvXMLAttributeList& );
    SvXMLAttributeList( const ::com::sun::star::uno::Reference< 
        ::com::sun::star::xml::sax::XAttributeList> & rAttrList );
    ~SvXMLAttributeList();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    static SvXMLAttributeList* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ) throw();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);			 

    // ::com::sun::star::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::util::XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()	throw( ::com::sun::star::uno::RuntimeException );

    // methods that are not contained in any interface
    void AddAttribute( const ::rtl::OUString &sName , const ::rtl::OUString &sValue );
    void Clear();
    void RemoveAttribute( const ::rtl::OUString sName );
    void AppendAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & );

 private:
    const ::rtl::OUString sType; // "CDATA"
};

}//end of namespace binfilter
#endif	//  _XMLOFF_ATTRLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
