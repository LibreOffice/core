/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <oox/core/contexthandler2.hxx>

namespace oox::drawingml {

struct EffectProperties;
struct Effect;

class EffectPropertiesContext final : public ::oox::core::ContextHandler2
{
public:
    EffectPropertiesContext( ::oox::core::ContextHandler2Helper const & rParent,
            EffectProperties& rEffectProperties ) noexcept;
    virtual ~EffectPropertiesContext() override;

    virtual ::oox::core::ContextHandlerRef
        onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) override;

private:
    static void saveUnsupportedAttribs( Effect& rEffect, const AttributeList& rAttribs );

    EffectProperties& mrEffectProperties;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
