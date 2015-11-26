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

#ifndef INCLUDED_XMLOFF_SOURCE_XFORMS_TOKENCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_XFORMS_TOKENCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace uno { template<typename T> class Reference; }
} } }

class SvXMLImport;

#define TOKEN_MAP_ENTRY(NAMESPACE,TOKEN) { XML_NAMESPACE_##NAMESPACE, xmloff::token::XML_##TOKEN, xmloff::token::XML_##TOKEN }

extern struct SvXMLTokenMapEntry aEmptyMap[1];

/** handle attributes through an SvXMLTokenMap */
class TokenContext : public SvXMLImportContext
{
protected:
    const SvXMLTokenMapEntry* mpAttributes;    /// static token map
    const SvXMLTokenMapEntry* mpChildren;      /// static token map

public:
    TokenContext( SvXMLImport& rImport,
                  sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const SvXMLTokenMapEntry* pAttributes = nullptr,
                  const SvXMLTokenMapEntry* pChildren = nullptr );

    virtual ~TokenContext();

    // implement SvXMLImportContext methods:

    /** call HandleAttribute for each attribute in the token map;
     * create a warning for all others. Classes that wish to override
     * StartElement need to call the parent method. */
    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    /** call HandleChild for each child element in the token map;
     * create a warning for all others. Classes that wish to override
     * CreateChildCotnenxt may want to call the parent method for
     * handling of defaults. */
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    /** Create a warning for all non-namespace character
     * content. Classes that wish to deal with character content have
     * to override this method anyway, and will thus get rid of the
     * warnings. */
    virtual void Characters( const OUString& rChars ) override;

protected:
    /** will be called for each attribute */
    virtual void HandleAttribute(
        sal_uInt16 nToken,
        const OUString& rValue ) = 0;

    /** will be called for each child element */
    virtual SvXMLImportContext* HandleChild(
        sal_uInt16 nToken,

        // the following attributes are mainly to be used for child
        // context creation
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
