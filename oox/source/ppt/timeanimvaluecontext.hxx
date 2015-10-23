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

#ifndef INCLUDED_OOX_SOURCE_PPT_TIMEANIMVALUECONTEXT_HXX
#define INCLUDED_OOX_SOURCE_PPT_TIMEANIMVALUECONTEXT_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/ppt/animationspersist.hxx"

namespace oox { namespace ppt {

    /** CT_TLTimeAnimateValueList */
    class TimeAnimValueListContext
        : public ::oox::core::FragmentHandler2
    {
    public:
        TimeAnimValueListContext( ::oox::core::FragmentHandler2& rParent,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttribs,
            TimeAnimationValueList & aTavList );

        virtual ~TimeAnimValueListContext( );

        virtual void onEndElement() override;

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override;

    private:
        TimeAnimationValueList & maTavList;
        bool                     mbInValue;
    };

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
