/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX

#include <oox/core/contexthandler2.hxx>
#include <oox/dllapi.h>

namespace oox { namespace drawingml {



struct EffectProperties;

class OOX_DLLPUBLIC EffectPropertiesContext : public ::oox::core::ContextHandler2
{
public:
    EffectPropertiesContext( ::oox::core::ContextHandler2Helper& rParent,
            EffectProperties& rEffectProperties ) throw();
    ~EffectPropertiesContext();

    virtual ::oox::core::ContextHandlerRef
        onCreateContext( ::sal_Int32 Element, const ::oox::AttributeList& rAttribs ) SAL_OVERRIDE;

protected:
    EffectProperties& mrEffectProperties;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
