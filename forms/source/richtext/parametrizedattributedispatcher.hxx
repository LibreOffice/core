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

#include "attributedispatcher.hxx"

class SfxPoolItem;

namespace frm
{

    class OParametrizedAttributeDispatcher  :public OAttributeDispatcher
    {
    public:
        OParametrizedAttributeDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const css::util::URL&               _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        virtual ~OParametrizedAttributeDispatcher() override;

        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

        // OAttributeDispatcher
        virtual void    fillFeatureEventFromAttributeState( css::frame::FeatureStateEvent& _rEvent, const AttributeState& _rState ) const override;

    protected:
        // own overridables
        /** convert the arguments as got in a XDispatch::dispatch call into an SfxPoolItem, which can
            be used with a IMultiAttributeDispatcher::executeAttribute
        */
        virtual const SfxPoolItem* convertDispatchArgsToItem(
            const css::uno::Sequence< css::beans::PropertyValue >& _rArguments );
    };


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
