/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CLIPOPTIONS_HXX
#define INCLUDED_SC_INC_CLIPOPTIONS_HXX

#include <com/sun/star/document/XDocumentProperties.hpp>

/// Stores options which are only relevant for clipboard documents.
class ScClipOptions
{
public:
    /// Document properties.
    css::uno::Reference<css::document::XDocumentProperties> m_xDocumentProperties;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
