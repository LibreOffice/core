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

#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#define _XMLTEXTCOLUMNSCONTEXT_HXX

#include "XMLElementPropertyContext.hxx"


namespace rtl { class OUString; }
class XMLTextColumnsArray_Impl;
class XMLTextColumnSepContext_Impl;
class SvXMLTokenMap;

class XMLTextColumnsContext :public XMLElementPropertyContext
{
    const ::rtl::OUString sSeparatorLineIsOn;
    const ::rtl::OUString sSeparatorLineWidth;
    const ::rtl::OUString sSeparatorLineColor;
    const ::rtl::OUString sSeparatorLineRelativeHeight;
    const ::rtl::OUString sSeparatorLineVerticalAlignment;
    const ::rtl::OUString sIsAutomatic;
    const ::rtl::OUString sAutomaticDistance;
    const ::rtl::OUString sSeparatorLineStyle;


    XMLTextColumnsArray_Impl *pColumns;
    XMLTextColumnSepContext_Impl     *pColumnSep;
    SvXMLTokenMap            *pColumnAttrTokenMap;
    SvXMLTokenMap            *pColumnSepAttrTokenMap;
    sal_Int16                nCount;
    sal_Bool                 bAutomatic;
    sal_Int32                nAutomaticDistance;

public:
    TYPEINFO();

    XMLTextColumnsContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLTextColumnsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
