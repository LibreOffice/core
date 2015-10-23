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

#ifndef INCLUDED_OOX_SOURCE_PPT_ANIMVARIANTCONTEXT_HXX
#define INCLUDED_OOX_SOURCE_PPT_ANIMVARIANTCONTEXT_HXX

#include <com/sun/star/uno/Any.hxx>

#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/color.hxx"

namespace oox { namespace ppt {

    /** context CT_TLAnimVariant */
    class AnimVariantContext
        : public ::oox::core::FragmentHandler2
    {
    public:
        AnimVariantContext( ::oox::core::FragmentHandler2& rParent, ::sal_Int32 aElement, css::uno::Any & aValue );
        virtual ~AnimVariantContext( ) throw( );
        virtual void onEndElement() override;
        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override;

    private:
        ::sal_Int32                 mnElement;
        css::uno::Any&              maValue;
        ::oox::drawingml::Color     maColor;
    };

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
