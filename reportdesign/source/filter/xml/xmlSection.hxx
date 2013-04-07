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
#ifndef RPT_XMLSECTION_HXX
#define RPT_XMLSECTION_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XSection.hpp>
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLSection : public SvXMLImportContext
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >              m_xSection;
        sal_Bool                                                                            m_bPageHeader;
        ORptFilter& GetOwnImport();

        OXMLSection(const OXMLSection&);
        void operator =(const OXMLSection&);
    public:

        OXMLSection( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                    ,sal_Bool _bPageHeader = sal_True);
        virtual ~OXMLSection();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
