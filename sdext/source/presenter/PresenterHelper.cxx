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

const OUString PresenterHelper::msPaneURLPrefix( "private:resource/pane/");
const OUString PresenterHelper::msCenterPaneURL( msPaneURLPrefix + OUString("CenterPane"));
const OUString PresenterHelper::msFullScreenPaneURL( msPaneURLPrefix + OUString("FullScreenPane"));

const OUString PresenterHelper::msViewURLPrefix( "private:resource/view/");
const OUString PresenterHelper::msPresenterScreenURL( msViewURLPrefix + OUString("PresenterScreen"));
const OUString PresenterHelper::msSlideSorterURL( msViewURLPrefix + OUString("SlideSorter"));

const OUString PresenterHelper::msResourceActivationEvent( "ResourceActivation");
const OUString PresenterHelper::msResourceDeactivationEvent( "ResourceDeactivation");

const OUString PresenterHelper::msDefaultPaneStyle ( "DefaultPaneStyle");
const OUString PresenterHelper::msDefaultViewStyle ( "DefaultViewStyle");

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
