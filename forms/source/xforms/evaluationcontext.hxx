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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_EVALUATIONCONTEXT_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_EVALUATIONCONTEXT_HXX

#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel.hpp>

namespace xforms
{


/** define the context for the evaluation of an XPath expression */
class EvaluationContext
{
public:
    EvaluationContext()
        : mxContextNode(),
          mxModel(),
          mxNamespaces(),
          mnContextPosition( 0 ),
          mnContextSize( 0 )
    { }

    EvaluationContext(
        const css::uno::Reference<css::xml::dom::XNode>& xContextNode,
        const css::uno::Reference<css::xforms::XModel>& xModel,
        const css::uno::Reference<css::container::XNameContainer>& xNamespaces,
        sal_Int32 nPosition,
        sal_Int32 nSize )
        : mxContextNode( xContextNode ),
          mxModel( xModel ),
          mxNamespaces( xNamespaces ),
          mnContextPosition( nPosition ),
          mnContextSize( nSize )
    { }

    css::uno::Reference<css::xml::dom::XNode> mxContextNode;
    css::uno::Reference<css::xforms::XModel> mxModel;
    css::uno::Reference<css::container::XNameContainer> mxNamespaces;

    sal_Int32 mnContextPosition;
    sal_Int32 mnContextSize;
};


} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
