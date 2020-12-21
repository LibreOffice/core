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

#include <drawinglayer/drawinglayerdllapi.h>

#include <basegfx/numeric/ftools.hxx> // for F_PI180
#include <basegfx/vector/b2enums.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <o3tl/cow_wrapper.hxx>

namespace basegfx
{
class BColor;
}

namespace drawinglayer::attribute
{
class ImpLineAttribute;
}

namespace drawinglayer::attribute
{
class DRAWINGLAYER_DLLPUBLIC LineAttribute
{
public:
    typedef o3tl::cow_wrapper<ImpLineAttribute> ImplType;

private:
    ImplType mpLineAttribute;

public:
    /// constructors/assignmentoperator/destructor
    explicit LineAttribute(const basegfx::BColor& rColor, double fWidth = 0.0,
                           basegfx::B2DLineJoin aB2DLineJoin = basegfx::B2DLineJoin::Round,
                           css::drawing::LineCap aLineCap = css::drawing::LineCap_BUTT,
                           double fMiterMinimumAngle = basegfx::deg2rad(15.0),
                           css::drawing::LineDash aLineDash = css::drawing::LineDash());
    LineAttribute();
    LineAttribute(const LineAttribute&);
    LineAttribute& operator=(const LineAttribute&);
    ~LineAttribute();

    // checks if the incarnation is default constructed
    bool isDefault() const;

    // compare operator
    bool operator==(const LineAttribute& rCandidate) const;

    // data read access
    const basegfx::BColor& getColor() const;
    double getWidth() const;
    basegfx::B2DLineJoin getLineJoin() const;
    css::drawing::LineCap getLineCap() const;
    css::drawing::LineDash getLineDash() const;
    double getMiterMinimumAngle() const;
};
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
