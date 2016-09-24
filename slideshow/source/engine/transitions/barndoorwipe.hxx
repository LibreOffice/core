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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_BARNDOORWIPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_BARNDOORWIPE_HXX

#include <basegfx/polygon/b2dpolygon.hxx>

#include "parametricpolypolygon.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

/// Generate a barn door wipe or double barn door wipe:
class BarnDoorWipe : public ParametricPolyPolygon
{
public:
    explicit BarnDoorWipe( bool doubled = false )
        : m_unitRect( createUnitRect() ), m_doubled(doubled) {}
    virtual ::basegfx::B2DPolyPolygon operator()( double x ) override;
private:
    const ::basegfx::B2DPolygon m_unitRect;
    const bool m_doubled;
};

}
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_BARNDOORWIPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
