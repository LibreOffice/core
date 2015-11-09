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

#ifndef INCLUDED_XMLOFF_XMLTEXTMASTERSTYLESCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLTEXTMASTERSTYLESCONTEXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmlstyle.hxx>

class XMLOFF_DLLPUBLIC XMLTextMasterStylesContext : public SvXMLStylesContext
{
protected:
    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const override;

public:

    XMLTextMasterStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList);

    virtual ~XMLTextMasterStylesContext();
};

#endif  //  _XMLOFF_XMLTEXTMASTERSTYLECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
