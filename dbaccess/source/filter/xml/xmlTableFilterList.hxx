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
#ifndef DBA_XMLTABLEFILTERLIST_HXX
#define DBA_XMLTABLEFILTERLIST_HXX

#include <xmloff/xmlictxt.hxx>
#include <vector>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTableFilterList : public SvXMLImportContext
    {
        ::std::vector< ::rtl::OUString> m_aPatterns;
        ::std::vector< ::rtl::OUString> m_aTypes;

        ODBFilter& GetOwnImport();
    public:

        OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName);

        virtual ~OXMLTableFilterList();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        /** pushes a new TableFilterPattern to the list of patterns
            @param  _sTableFilterPattern
                The new filter pattern.
        */
        inline void pushTableFilterPattern(const ::rtl::OUString& _sTableFilterPattern)
        {
            m_aPatterns.push_back(_sTableFilterPattern);
        }

        /** pushes a new TableTypeFilter to the list of patterns
            @param  _sTypeFilter
                The new type filter.
        */
        inline void pushTableTypeFilter(const ::rtl::OUString& _sTypeFilter)
        {
            m_aTypes.push_back(_sTypeFilter);
        }
    };
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLEFILTERLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
