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

#ifndef INCLUDED_XMLOFF_INC_ANIMATIONIMPORT_HXX
#define INCLUDED_XMLOFF_INC_ANIMATIONIMPORT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>

// presentations:animations

namespace xmloff
{
class AnimationsImportHelperImpl;

class AnimationNodeContext : public SvXMLImportContext
{
    AnimationsImportHelperImpl* mpHelper;
    bool mbRootContext;
    css::uno::Reference< css::animations::XAnimationNode > mxNode;

    void init_node( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

public:

    AnimationNodeContext(
        const css::uno::Reference< css::animations::XAnimationNode >& xParentNode,
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        AnimationsImportHelperImpl* mpImpl = nullptr );
    virtual ~AnimationNodeContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    static void postProcessRootNode( SvXMLImport& rImport, const css::uno::Reference< css::animations::XAnimationNode >& xNode, css::uno::Reference< css::beans::XPropertySet >& xPageProps );
};

}

#endif // INCLUDED_XMLOFF_INC_ANIMATIONIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
