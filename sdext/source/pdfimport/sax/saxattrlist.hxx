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
