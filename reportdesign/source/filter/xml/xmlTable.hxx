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
#ifndef RPT_XMLTABLE_HXX
#define RPT_XMLTABLE_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XSection.hpp>
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLTable : public SvXMLImportContext
    {
    public:
        struct TCell
        {
            sal_Int32 nWidth;
            sal_Int32 nHeight;
            sal_Int32 nColSpan;
            sal_Int32 nRowSpan;
            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> > xElements;
            TCell() : nWidth(0),nHeight(0),nColSpan(1),nRowSpan(1){}
        };
    private:
        ::std::vector< ::std::vector<TCell> >                                               m_aGrid;
        ::std::vector<sal_Int32>                                                            m_aHeight;
        ::std::vector<sal_Int32>                                                            m_aWidth;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >              m_xSection;
        ::rtl::OUString                                                                     m_sStyleName;
        sal_Int32                                                                           m_nColSpan;
        sal_Int32                                                                           m_nRowSpan;
        sal_Int32                                                                           m_nRowIndex;
        sal_Int32                                                                           m_nColumnIndex;
        ORptFilter& GetOwnImport();

        OXMLTable(const OXMLTable&);
        void operator =(const OXMLTable&);
    public:

        OXMLTable( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                    );
        virtual ~OXMLTable();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        inline void addHeight(sal_Int32 _nHeight)   { m_aHeight.push_back( _nHeight ); }
        inline void addWidth(sal_Int32 _nWidth)     { m_aWidth.push_back( _nWidth ); }

        inline void setColumnSpanned(sal_Int32 _nColSpan)     { m_nColSpan = _nColSpan; }
        inline void setRowSpanned(   sal_Int32 _nRowSpan)     { m_nRowSpan = _nRowSpan; }

        void incrementRowIndex();
        inline void incrementColumnIndex()  { ++m_nColumnIndex; }

        inline sal_Int32 getRowIndex() const { return m_nRowIndex; }
        inline sal_Int32 getColumnIndex() const { return m_nColumnIndex; }

        void addCell(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xElement);

        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const { return m_xSection; }
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
