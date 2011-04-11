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
    RTL_CONSTASCII_USTRINGPARAM("private:resource/pane/"));
const OUString PresenterHelper::msCenterPaneURL(
    msPaneURLPrefix + OUString(RTL_CONSTASCII_USTRINGPARAM("CenterPane")));
const OUString PresenterHelper::msFullScreenPaneURL(
    msPaneURLPrefix + OUString(RTL_CONSTASCII_USTRINGPARAM("FullScreenPane")));

const OUString PresenterHelper::msViewURLPrefix(
    RTL_CONSTASCII_USTRINGPARAM("private:resource/view/"));
const OUString PresenterHelper::msPresenterScreenURL(
    msViewURLPrefix + OUString(RTL_CONSTASCII_USTRINGPARAM("PresenterScreen")));
const OUString PresenterHelper::msSlideSorterURL(
    msViewURLPrefix + OUString(RTL_CONSTASCII_USTRINGPARAM("SlideSorter")));

const OUString PresenterHelper::msResourceActivationEvent(
    RTL_CONSTASCII_USTRINGPARAM("ResourceActivation"));
const OUString PresenterHelper::msResourceDeactivationEvent(
    RTL_CONSTASCII_USTRINGPARAM("ResourceDeactivation"));

const OUString PresenterHelper::msDefaultPaneStyle (
    RTL_CONSTASCII_USTRINGPARAM("DefaultPaneStyle"));
const OUString PresenterHelper::msDefaultViewStyle (
    RTL_CONSTASCII_USTRINGPARAM("DefaultViewStyle"));


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
