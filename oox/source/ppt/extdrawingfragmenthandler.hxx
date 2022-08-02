/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_SOURCE_PPT_EXTDRAWINGFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_SOURCE_PPT_EXTDRAWINGFRAGMENTHANDLER_HXX

#include <oox/core/fragmenthandler2.hxx>
#include <oox/ppt/slidepersist.hxx>

namespace oox::ppt {

class ExtDrawingFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    ExtDrawingFragmentHandler( oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
        oox::ppt::SlidePersistPtr pSlidePersistPtr,
        const oox::ppt::ShapeLocation eShapeLocation,
        oox::drawingml::ShapePtr pGroupShapePtr,
        oox::drawingml::ShapePtr pShapePtr );
    virtual ~ExtDrawingFragmentHandler() noexcept override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const AttributeList& rAttribs ) override;

private:
        const oox::ppt::SlidePersistPtr mpSlidePersistPtr;
        const oox::ppt::ShapeLocation   meShapeLocation;
        oox::drawingml::ShapePtr        mpGroupShapePtr;
        oox::drawingml::ShapePtr        mpShapePtr;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
