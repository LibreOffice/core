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
#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLTABLEFILTERLIST_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLTABLEFILTERLIST_HXX

#include <xmloff/xmlictxt.hxx>
#include <vector>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTableFilterList : public SvXMLImportContext
    {
        ::std::vector< OUString> m_aPatterns;
        ::std::vector< OUString> m_aTypes;

        ODBFilter& GetOwnImport();
    public:

        OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx,
                    const OUString& rLName);

        virtual ~OXMLTableFilterList();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

        virtual void EndElement() override;

        /** pushes a new TableFilterPattern to the list of patterns
            @param  _sTableFilterPattern
                The new filter pattern.
        */
        inline void pushTableFilterPattern(const OUString& _sTableFilterPattern)
        {
            m_aPatterns.push_back(_sTableFilterPattern);
        }

        /** pushes a new TableTypeFilter to the list of patterns
            @param  _sTypeFilter
                The new type filter.
        */
        inline void pushTableTypeFilter(const OUString& _sTypeFilter)
        {
            m_aTypes.push_back(_sTypeFilter);
        }
    };
} // namespace dbaxml

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLTABLEFILTERLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
