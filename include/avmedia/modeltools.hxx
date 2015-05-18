/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_MODELTOOLS_HXX
#define INCLUDED_AVMEDIA_MODELTOOLS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <avmedia/avmediadllapi.h>

#include <config_features.h>

namespace avmedia {

#if HAVE_FEATURE_COLLADA
bool KmzDae2Gltf(const OUString& rSourceURL, OUString& o_rOutput);
#endif

bool AVMEDIA_DLLPUBLIC Embed3DModel(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
    const OUString& rSourceURL, OUString& o_rEmbeddedURL);

bool AVMEDIA_DLLPUBLIC IsModel(const OUString& rMimeType);

}

#endif // INCLUDED_AVMEDIA_MODELTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
