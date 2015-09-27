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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_SAX_SAXATTRLIST_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_SAX_SAXATTRLIST_HXX

#include <rtl/ustring.hxx>
#include <unordered_map>
#include <vector>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace pdfi
{
    class SaxAttrList : public ::cppu::WeakImplHelper<
            css::xml::sax::XAttributeList,
            css::util::XCloneable
            >
    {
        struct AttrEntry
        {
            OUString m_aName;
            OUString m_aValue;

            AttrEntry( const OUString& i_rName, const OUString& i_rValue )
            : m_aName( i_rName ), m_aValue( i_rValue ) {}
        };
        std::vector< AttrEntry >                                    m_aAttributes;
        std::unordered_map< OUString, size_t, OUStringHash >   m_aIndexMap;

    public:
        SaxAttrList() {}
        explicit SaxAttrList( const std::unordered_map< OUString, OUString, OUStringHash >& );
        SaxAttrList( const SaxAttrList& );
        virtual ~SaxAttrList();

        // css::xml::sax::XAttributeList
        virtual sal_Int16 SAL_CALL getLength() throw(std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw(std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw(std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw(std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw(std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getValueByName(const OUString& aName) throw(std::exception) SAL_OVERRIDE;

        // css::util::XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() throw(std::exception) SAL_OVERRIDE;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
