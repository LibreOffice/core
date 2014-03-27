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

#ifndef OOX_PPT_SLIDETRANSITIONCONTEXT
#define OOX_PPT_SLIDETRANSITIONCONTEXT

#include <oox/core/fragmenthandler2.hxx>
#include <oox/ppt/slidetransition.hxx>

namespace oox { class PropertyMap; }

namespace oox { namespace ppt {

    class SlideTransitionContext : public ::oox::core::FragmentHandler2
    {
    public:
        SlideTransitionContext( ::oox::core::FragmentHandler2& rParent,
            const AttributeList& rAttributes,
            PropertyMap & aProperties ) throw();
        virtual ~SlideTransitionContext() throw();

    virtual void onEndElement() SAL_OVERRIDE;
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) SAL_OVERRIDE;


    private:
        PropertyMap&                    maSlideProperties;
        ::sal_Bool                      mbHasTransition;
        SlideTransition                 maTransition;
    };

} }

#endif // OOX_PPT_SLIDEFRAGMENTHANDLER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
