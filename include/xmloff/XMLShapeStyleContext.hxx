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

#ifndef INCLUDED_XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#define INCLUDED_XMLOFF_XMLSHAPESTYLECONTEXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/prstylei.hxx>

class SvXMLImport;


// style:style context

class XMLOFF_DLLPUBLIC XMLShapeStyleContext: public XMLPropStyleContext
{
    OUString     m_sControlDataStyleName;
    bool         m_bIsNumRuleAlreadyConverted;

protected:
    OUString     m_sListStyleName;

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;
public:

    XMLShapeStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily);
    virtual ~XMLShapeStyleContext() override;

    SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void Finish( bool bOverwrite ) override;

    virtual void FillPropertySet(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;
};

#endif // INCLUDED_XMLOFF_XMLSHAPESTYLECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
