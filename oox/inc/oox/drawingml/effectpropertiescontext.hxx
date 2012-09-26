/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX
#define OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX

#include "oox/core/contexthandler.hxx"
#include "oox/dllapi.h"

namespace oox { namespace drawingml {

// ---------------------------------------------------------------------

struct EffectProperties;

class OOX_DLLPUBLIC EffectPropertiesContext : public ::oox::core::ContextHandler
{
public:
    EffectPropertiesContext( ::oox::core::ContextHandler& rParent,
            EffectProperties& rEffectProperties ) throw();
    ~EffectPropertiesContext();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
            throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    EffectProperties& mrEffectProperties;
};

} }

#endif // OOX_DRAWINGML_EFFECTPROPERTIESCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
