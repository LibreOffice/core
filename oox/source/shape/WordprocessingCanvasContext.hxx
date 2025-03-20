/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <oox/core/fragmenthandler2.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

namespace oox::shape
{
/// Handles CT_WordprocessingCanvas, used for wpc element, which is a drawing canvas for Word.
class WordprocessingCanvasContext final : public oox::core::FragmentHandler2
{
public:
    // mpShapePtr points to the root of the group. rSize is the size of the background shape.
    explicit WordprocessingCanvasContext(oox::core::FragmentHandler2 const& rParent,
                                         const css::awt::Size& rSize);
    ~WordprocessingCanvasContext() override;

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElementToken,
                                                 const ::oox::AttributeList& rAttribs) override;

    const oox::drawingml::ShapePtr& getShape() { return mpShapePtr; }
    const bool& isFullWPGSupport() const { return m_bFullWPGSupport; };
    void setFullWPGSupport(bool bUse) { m_bFullWPGSupport = bUse; };

private:
    oox::drawingml::ShapePtr mpShapePtr;
    bool m_bFullWPGSupport;
};
} // end namespace oox::shape

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
