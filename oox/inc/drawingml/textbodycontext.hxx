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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTBODYCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTBODYCONTEXT_HXX

#include <com/sun/star/text/XText.hpp>

#include <drawingml/textbody.hxx>
#include <drawingml/textrun.hxx>
#include <oox/core/contexthandler2.hxx>

namespace oox { namespace drawingml {

class TextBodyContext : public ::oox::core::ContextHandler2
{
public:
    TextBodyContext( ::oox::core::ContextHandler2Helper& rParent, TextBody& rTextBody );

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;

protected:
    TextBody&           mrTextBody;
    css::uno::Reference< css::text::XText > mxText;
};

// CT_RegularTextRun
class RegularTextRunContext : public ::oox::core::ContextHandler2
{
public:
    RegularTextRunContext( ::oox::core::ContextHandler2Helper& rParent, TextRunPtr pRunPtr );

    virtual void onEndElement() override;
    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;
    virtual void onCharacters( const OUString& aChars ) override;

protected:
    TextRunPtr          mpRunPtr;
    bool                mbIsInText;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_TEXTBODYCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
