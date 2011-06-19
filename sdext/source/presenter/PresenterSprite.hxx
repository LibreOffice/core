/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SDEXT_PRESENTER_PRESENTER_SPRITE_HXX
#define SDEXT_PRESENTER_PRESENTER_SPRITE_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** A wrapper around a com::sun::star::rendering::XCustomSprite that allows
    not only setting values like size, location, and transformation but also
    provides read access to them.
    It also handles the showing and hiding of a sprite.  This includes not
    to show the sprite when its size is not yet defined (results in a crash)
    and hiding a sprite before disposing it (results in zombie sprites.)
*/
class PresenterSprite
    : private ::boost::noncopyable
{
public:
    PresenterSprite (void);
    virtual ~PresenterSprite (void);

    /** The given sprite canvas is used as factory to create the sprite that
        is wrapped by objects of this class.
        It is also used to call updateScreen() at (wrapped by the Update() method).
    */
    void SetFactory (
        const ::css::uno::Reference<css::rendering::XSpriteCanvas>& rxSpriteFactory);

    ::css::uno::Reference<css::rendering::XCanvas> GetCanvas (void);

    void Show (void);
    void Hide (void);
    bool IsVisible (void) const;

    void SetPriority (const double nPriority);
    double GetPriority (void) const;

    void Resize (const css::geometry::RealSize2D& rSize);
    css::geometry::RealSize2D GetSize (void) const;

    void MoveTo (const css::geometry::RealPoint2D& rLocation);
    css::geometry::RealPoint2D GetLocation (void) const;

    void Transform (const css::geometry::AffineMatrix2D& rTransform);
    css::geometry::AffineMatrix2D GetTransform (void) const;

    void SetAlpha (const double nAlpha);
    double GetAlpha (void) const;

    void Update (void);

private:
    ::css::uno::Reference<css::rendering::XSpriteCanvas> mxSpriteFactory;
    ::css::uno::Reference<css::rendering::XCustomSprite> mxSprite;
    css::geometry::RealSize2D maSize;
    css::geometry::RealPoint2D maLocation;
    css::geometry::AffineMatrix2D maTransform;
    bool mbIsVisible;
    double mnPriority;
    double mnAlpha;

    void ProvideSprite (void);
    void DisposeSprite (void);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
