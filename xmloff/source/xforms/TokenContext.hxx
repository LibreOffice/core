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

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltkmap.hxx>

namespace com::sun::star {
    namespace xml::sax { class XFastAttributeList; }
    namespace uno { template<typename T> class Reference; }
}

class SvXMLImport;

/** handle attributes through an SvXMLTokenMap */
class TokenContext : public SvXMLImportContext
{
public:
    TokenContext( SvXMLImport& rImport );

    // implement SvXMLImportContext methods:

    /** call HandleAttribute for each attribute in the token map;
     * create a warning for all others. Classes that wish to override
     * StartElement need to call the parent method. */
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    /** call HandleChild for each child element in the token map;
     * create a warning for all others. Classes that wish to override
     * CreateChildContext may want to call the parent method for
     * handling of defaults. */
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(
        const OUString& Namespace, const OUString& Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;

    /** Create a warning for all non-namespace character
     * content. Classes that wish to deal with character content have
     * to override this method anyway, and will thus get rid of the
     * warnings. */
    virtual void SAL_CALL characters( const OUString& rChars ) override;

protected:
    /** will be called for each attribute */
    virtual void HandleAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter ) = 0;

    /** will be called for each child element */
    virtual SvXMLImportContext* HandleChild(
        sal_Int32 nElementToken,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
