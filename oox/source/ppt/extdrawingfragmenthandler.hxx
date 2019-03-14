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
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <oox/ppt/pptshapegroupcontext.hxx>
#include <oox/ppt/pptshape.hxx>

namespace oox { namespace ppt {

class ExtDrawingFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    ExtDrawingFragmentHandler( oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
        const oox::ppt::SlidePersistPtr& rSlidePersistPtr,
        const oox::ppt::ShapeLocation eShapeLocation,
        oox::drawingml::ShapePtr const & pGroupShapePtr,
        oox::drawingml::ShapePtr const & pShapePtr );
    virtual ~ExtDrawingFragmentHandler() throw() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const AttributeList& rAttribs ) override;

private:
        const oox::ppt::SlidePersistPtr mpSlidePersistPtr;
        const oox::ppt::ShapeLocation   meShapeLocation;
        oox::drawingml::ShapePtr const  mpGroupShapePtr;
        oox::drawingml::ShapePtr        mpShapePtr;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
