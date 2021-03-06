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

#include "parametrizedattributedispatcher.hxx"


namespace frm
{

    class OSelectAllDispatcher : public ORichTextFeatureDispatcher
    {
    public:
        OSelectAllDispatcher( EditView& _rView, const css::util::URL&  _rURL );

    protected:
        virtual ~OSelectAllDispatcher() override;

        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

        // ORichTextFeatureDispatcher
        virtual css::frame::FeatureStateEvent  buildStatusEvent() const override;
    };

    class OParagraphDirectionDispatcher : public OAttributeDispatcher
    {
    public:
        OParagraphDirectionDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const css::util::URL&               _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        // ORichTextFeatureDispatcher
        virtual css::frame::FeatureStateEvent  buildStatusEvent() const override;
    };

    class OTextDirectionDispatcher : public ORichTextFeatureDispatcher
    {
    public:
        OTextDirectionDispatcher( EditView& _rView, const css::util::URL&  _rURL );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const css::util::URL& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

        // ORichTextFeatureDispatcher
        virtual css::frame::FeatureStateEvent  buildStatusEvent() const override;
    };

    class OAsianFontLayoutDispatcher : public OParametrizedAttributeDispatcher
    {
    public:
        OAsianFontLayoutDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const css::util::URL&                _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

    protected:
        // OParametrizedAttributeDispatcher
        virtual const SfxPoolItem* convertDispatchArgsToItem(
            const css::uno::Sequence< css::beans::PropertyValue >& _rArguments ) override;
    };


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
