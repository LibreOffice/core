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

#ifndef INCLUDED_PDFI_SAXATTRLIST_HXX
#define INCLUDED_PDFI_SAXATTRLIST_HXX

#include <rtl/ustring.hxx>
#include <vector>
#include <boost/unordered_map.hpp>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace pdfi
{
    class SaxAttrList : public ::cppu::WeakImplHelper2<
            com::sun::star::xml::sax::XAttributeList,
            com::sun::star::util::XCloneable
            >
    {
        struct AttrEntry
        {
            rtl::OUString m_aName;
            rtl::OUString m_aValue;

            AttrEntry( const rtl::OUString& i_rName, const rtl::OUString& i_rValue )
            : m_aName( i_rName ), m_aValue( i_rValue ) {}
        };
        std::vector< AttrEntry >                                    m_aAttributes;
        boost::unordered_map< rtl::OUString, size_t, rtl::OUStringHash >   m_aIndexMap;

    public:
        SaxAttrList() {}
        SaxAttrList( const boost::unordered_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >& );
        SaxAttrList( const SaxAttrList& );
        virtual ~SaxAttrList();

        // ::com::sun::star::xml::sax::XAttributeList
        virtual sal_Int16 SAL_CALL getLength() throw();
        virtual rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName) throw();
        virtual rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i) throw();
        virtual rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName) throw();

        // ::com::sun::star::util::XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone() throw();
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
