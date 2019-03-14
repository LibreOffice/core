/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "extdrawingfragmenthandler.hxx"

#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace oox { namespace ppt {

ExtDrawingFragmentHandler::ExtDrawingFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
        const oox::ppt::SlidePersistPtr& rSlidePersistPtr,
        const oox::ppt::ShapeLocation   eShapeLocation,
        oox::drawingml::ShapePtr const & pGroupShapePtr,
        oox::drawingml::ShapePtr const & pShapePtr)
    : FragmentHandler2( rFilter, rFragmentPath ),
     mpSlidePersistPtr (rSlidePersistPtr ),
     meShapeLocation( eShapeLocation ),
     mpGroupShapePtr( pGroupShapePtr ),
     mpShapePtr( pShapePtr )
{
}

ExtDrawingFragmentHandler::~ExtDrawingFragmentHandler( ) throw ()
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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
