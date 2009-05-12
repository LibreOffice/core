/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterHelper.cxx,v $
 *
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterHelper.hxx"

#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using ::rtl::OUString;

namespace sdext { namespace presenter {

const OUString PresenterHelper::msPaneURLPrefix(
    OUString::createFromAscii("private:resource/pane/"));
const OUString PresenterHelper::msCenterPaneURL(
    msPaneURLPrefix + OUString::createFromAscii("CenterPane"));
const OUString PresenterHelper::msFullScreenPaneURL(
    msPaneURLPrefix + OUString::createFromAscii("FullScreenPane"));

const OUString PresenterHelper::msViewURLPrefix(
    OUString::createFromAscii("private:resource/view/"));
const OUString PresenterHelper::msPresenterScreenURL(
    msViewURLPrefix + OUString::createFromAscii("PresenterScreen"));
const OUString PresenterHelper::msSlideSorterURL(
    msViewURLPrefix + OUString::createFromAscii("SlideSorter"));

const OUString PresenterHelper::msResourceActivationEvent(
    OUString::createFromAscii("ResourceActivation"));
const OUString PresenterHelper::msResourceDeactivationEvent(
    OUString::createFromAscii("ResourceDeactivation"));

const OUString PresenterHelper::msDefaultPaneStyle (
    OUString::createFromAscii("DefaultPaneStyle"));
const OUString PresenterHelper::msDefaultViewStyle (
    OUString::createFromAscii("DefaultViewStyle"));


Reference<presentation::XSlideShowController> PresenterHelper::GetSlideShowController (
    const Reference<frame::XController>& rxController)
{
    Reference<presentation::XSlideShowController> xSlideShowController;

    if( rxController.is() ) try
    {
        Reference<XPresentationSupplier> xPS ( rxController->getModel(), UNO_QUERY_THROW);

        Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);

        xSlideShowController = xPresentation->getController();
    }
    catch(RuntimeException&)
    {
    }

    return xSlideShowController;
}




} } // end of namespace ::sdext::presenter
