/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "extdrawingfragmenthandler.hxx"

#include <oox/ppt/pptshapegroupcontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <utility>

using namespace ::oox::core;

namespace oox::ppt {

ExtDrawingFragmentHandler::ExtDrawingFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
        oox::ppt::SlidePersistPtr pSlidePersistPtr,
        const oox::ppt::ShapeLocation   eShapeLocation,
        oox::drawingml::ShapePtr pGroupShapePtr,
        oox::drawingml::ShapePtr pShapePtr)
    : FragmentHandler2( rFilter, rFragmentPath ),
     mpSlidePersistPtr (std::move(pSlidePersistPtr )),
     meShapeLocation( eShapeLocation ),
     mpGroupShapePtr(std::move( pGroupShapePtr )),
     mpShapePtr(std::move( pShapePtr ))
{
}

ExtDrawingFragmentHandler::~ExtDrawingFragmentHandler( ) noexcept
{
    // Empty DrawingML fallback, need to warn the user at the end.
    if (mpShapePtr && mpShapePtr->getChildren().empty())
        getFilter().setMissingExtDrawing();
}

ContextHandlerRef
ExtDrawingFragmentHandler::onCreateContext( ::sal_Int32 aElement,
                                            const AttributeList& )
{
    switch( aElement )
    {
    case DSP_TOKEN( drawing ):
        break;
    case DSP_TOKEN( spTree ):
        return new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpGroupShapePtr,
                mpShapePtr );
    default:
        break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
