/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *                       Novell, Inc.
 *
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Muthu Subramanian <sumuthu@suse.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "extdrawingfragmenthandler.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace oox { namespace ppt {

ExtDrawingFragmentHandler::ExtDrawingFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
        const oox::ppt::SlidePersistPtr pSlidePersistPtr,
        const oox::ppt::ShapeLocation   eShapeLocation,
        oox::drawingml::ShapePtr        pMasterShapePtr,
        oox::drawingml::ShapePtr        pGroupShapePtr,
        oox::drawingml::ShapePtr        pShapePtr)
    throw( )
    : FragmentHandler( rFilter, rFragmentPath ),
     mpSlidePersistPtr (pSlidePersistPtr ),
     meShapeLocation( eShapeLocation ),
     mpMasterShapePtr( pMasterShapePtr ),
     mpGroupShapePtr( pGroupShapePtr ),
     mpOrgShapePtr( pShapePtr )
{
}

ExtDrawingFragmentHandler::~ExtDrawingFragmentHandler( ) throw ()
{

}

Reference< XFastContextHandler > SAL_CALL
ExtDrawingFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                    const Reference< XFastAttributeList >& )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DSP_TOKEN( drawing ):
        break;
    case DSP_TOKEN( spTree ):
        mpShapePtr = oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) );
        xRet.set( new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpSlidePersistPtr->getShapes(),
                mpShapePtr ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}
void SAL_CALL ExtDrawingFragmentHandler::endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    mpShapePtr->moveAllToPosition( mpOrgShapePtr->getPosition() );
    mpShapePtr->setName( mpOrgShapePtr->getName() );
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
