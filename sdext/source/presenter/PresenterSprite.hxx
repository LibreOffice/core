/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
