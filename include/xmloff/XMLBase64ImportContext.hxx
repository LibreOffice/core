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

#ifndef INCLUDED_XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLBASE64IMPORTCONTEXT_HXX

#include <sal/config.h>
#include <rtl/ustrbuf.hxx>
#include <xmloff/dllapi.h>
#include <xmloff/xmlictxt.hxx>

namespace com::sun::star::xml::sax { class XAttributeList; }
namespace com::sun::star::io { class XOutputStream; }

class XMLOFF_DLLPUBLIC XMLBase64ImportContext final : public SvXMLImportContext
{
    css::uno::Reference< css::io::XOutputStream > xOut;
    OUStringBuffer maCharBuffer;

public:

    XMLBase64ImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                            const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
        const css::uno::Reference< css::io::XOutputStream >& rOut );

    XMLBase64ImportContext( SvXMLImport& rImport,
        const css::uno::Reference< css::io::XOutputStream >& rOut );

    virtual ~XMLBase64ImportContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32) override;
    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual void EndElement() override;
    virtual void Characters( const OUString& rChars ) override;

};

#endif // INCLUDED_XMLOFF_XMLBASE64IMPORTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
