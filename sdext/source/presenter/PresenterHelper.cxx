/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
