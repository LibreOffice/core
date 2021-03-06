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

#pragma once

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com::sun::star::lang { class XComponent; }

class XMLEmbeddedObjectImportContext final : public SvXMLImportContext
{
    css::uno::Reference<css::xml::sax::XFastDocumentHandler > mxFastHandler;
    css::uno::Reference<css::lang::XComponent > xComp;

    OUString sFilterService;
    OUString sCLSID;

public:

    const OUString& GetFilterServiceName() const { return sFilterService; }
    const OUString& GetFilterCLSID() const { return sCLSID; }

    XMLEmbeddedObjectImportContext( SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );

    virtual ~XMLEmbeddedObjectImportContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL startFastElement(
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;

    void SetComponent( css::uno::Reference< css::lang::XComponent > const & rComp );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
