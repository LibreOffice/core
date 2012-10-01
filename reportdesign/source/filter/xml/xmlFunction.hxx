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
#ifndef RPT_XMLFUNCTION_HXX
#define RPT_XMLFUNCTION_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/report/XFunctionsSupplier.hpp>
#include <com/sun/star/report/XFunctions.hpp>


namespace rptxml
{
    class ORptFilter;
    class OXMLFunction : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions >    m_xFunctions;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction >     m_xFunction;
        bool                                                                        m_bAddToReport;

        ORptFilter& GetOwnImport();

        OXMLFunction(const OXMLFunction&);
        OXMLFunction operator =(const OXMLFunction&);
    public:

        OXMLFunction( ORptFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctionsSupplier >&    _xFunctions
                    ,bool _bAddToReport = false
                    );
        virtual ~OXMLFunction();

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFunction_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
