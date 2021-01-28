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

#ifndef INCLUDED_OOX_DRAWINGML_GRAPHICPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_GRAPHICPROPERTIES_HXX

#include <sal/config.h>

#include <com/sun/star/io/XInputStream.hpp>

#include <drawingml/fillproperties.hxx>

namespace oox {
    class GraphicHelper;
    class PropertyMap;
}

namespace oox::drawingml {

struct GraphicProperties
{
    BlipFillProperties      maBlipProps;            ///< Properties for the graphic.
    OUString                m_sMediaPackageURL;     ///< Audio/Video URL.
    bool                    mbIsCustomShape = false;
    css::uno::Reference<css::io::XInputStream> m_xMediaStream; ///< Audio/Video input stream.

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper,
                            bool bFlipH = false,
                            bool bFlipV = false) const;
};

} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
