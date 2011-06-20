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
#ifndef RPT_XMLSUBDOCUMENT_HXX
#define RPT_XMLSUBDOCUMENT_HXX

#include "xmlReportElementBase.hxx"
#include <com/sun/star/report/XReportComponent.hpp>
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLCell;
    class OXMLSubDocument : public OXMLReportElementBase, public IMasterDetailFieds
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>   m_xComponent;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>   m_xFake;
        ::std::vector< ::rtl::OUString> m_aMasterFields;
        ::std::vector< ::rtl::OUString> m_aDetailFields;
        OXMLCell*       m_pCellParent;
        sal_Int32       m_nCurrentCount;
        bool            m_bContainsShape;

        OXMLSubDocument(const OXMLSubDocument&);
        void operator =(const OXMLSubDocument&);

        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    public:

        OXMLSubDocument( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent
                    ,OXMLTable* _pContainer
                    ,OXMLCell* _pCellParent);
        virtual ~OXMLSubDocument();

        virtual void EndElement();
        virtual void addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair);
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLSubDocument_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
