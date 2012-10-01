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
#ifndef RPT_XMLREPORTELEMENTBASE_HXX
#define RPT_XMLREPORTELEMENTBASE_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XReportComponent.hpp>
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLTable;

    class SAL_NO_VTABLE IMasterDetailFieds
    {
    public:
        virtual void addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair) = 0;

    protected:
        ~IMasterDetailFieds() {}
    };

    class OXMLReportElementBase : public SvXMLImportContext
    {
        OXMLReportElementBase(const OXMLReportElementBase&);
        void operator =(const OXMLReportElementBase&);
    protected:
        ORptFilter&   m_rImport;
        OXMLTable*    m_pContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >      m_xComponent;

        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    public:

        OXMLReportElementBase( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent
                    ,OXMLTable* _pContainer);
        virtual ~OXMLReportElementBase();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLREPORTELEMENTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
