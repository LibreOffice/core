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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace com::sun::star::awt
{
class XControl;
}
namespace com::sun::star::awt
{
class XControlModel;
}

namespace drawinglayer::primitive2d
{
/** ControlPrimitive2D class

    Base class for ControlPrimitive handling. It decomposes to a
    graphical representation (Bitmap data) of the control. This
    representation is limited to a quadratic pixel maximum defined
    in the application settings.
 */
class DRAWINGLAYER_DLLPUBLIC ControlPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    /// object's base data
    basegfx::B2DHomMatrix maTransform;
    css::uno::Reference<css::awt::XControlModel> mxControlModel;

    /// the created and cached awt::XControl
    css::uno::Reference<css::awt::XControl> mxXControl;

    /// the last used scaling, used from getDecomposition for buffering
    basegfx::B2DVector maLastViewScaling;

    /// yet another special snowflake way to generate PDF Alt text
    OUString m_AltText;

    /// anchor structure element (Writer)
    void const* const m_pAnchorStructureElementKey;

    /** used from getXControl() to create a local awt::XControl which is remembered in mxXControl
                and from thereon always used and returned by getXControl()
             */
    void createXControl();

    /// single local decompositions, used from create2DDecomposition()
    Primitive2DReference
    createBitmapDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
    Primitive2DReference createPlaceholderDecomposition() const;

    /// local decomposition
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /** constructor with an optional XControl as parameter to allow to hand it over at incarnation time
        if it exists. This will avoid to create a 2nd one on demand in createXControl()
        and thus double the XControls.
     */
    ControlPrimitive2D(basegfx::B2DHomMatrix aTransform,
                       css::uno::Reference<css::awt::XControlModel> xControlModel,
                       css::uno::Reference<css::awt::XControl> xXControl,
                       ::std::u16string_view rTitle, ::std::u16string_view rDescription,
                       void const* pAnchorKey);

    /// data read access
    const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
    const css::uno::Reference<css::awt::XControlModel>& getControlModel() const
    {
        return mxControlModel;
    }

    /// check if this control is visible as ChildWindow
    bool isVisibleAsChildWindow() const;

    /** mxControl access. This will on demand create the awt::XControl using createXControl()
        if it does not exist. It may already have been created or even handed over at
        incarnation
     */
    const css::uno::Reference<css::awt::XControl>& getXControl() const;

    OUString const& GetAltText() const { return m_AltText; }

    void const* GetAnchorStructureElementKey() const { return m_pAnchorStructureElementKey; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;

    /// Override standard getDecomposition to be view-dependent here
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
