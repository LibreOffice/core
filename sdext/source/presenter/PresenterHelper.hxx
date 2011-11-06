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



#ifndef SDEXT_PRESENTER_VIEW_HELPER_HXX
#define SDEXT_PRESENTER_VIEW_HELPER_HXX

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** Collection of helper functions that do not fit in anywhere else.
    Provide access to frequently used strings of the drawing framework.
*/
class PresenterHelper
    : ::boost::noncopyable
{
public:
    static const ::rtl::OUString msPaneURLPrefix;
    static const ::rtl::OUString msCenterPaneURL;
    static const ::rtl::OUString msFullScreenPaneURL;

    static const ::rtl::OUString msViewURLPrefix;
    static const ::rtl::OUString msPresenterScreenURL;
    static const ::rtl::OUString msSlideSorterURL;

    static const ::rtl::OUString msResourceActivationEvent;
    static const ::rtl::OUString msResourceDeactivationEvent;

    static const ::rtl::OUString msDefaultPaneStyle;
    static const ::rtl::OUString msDefaultViewStyle;

    /** Return the slide show controller of a running presentation that has
        the same document as the given framework controller.
        @return
            When no presentation is running this method returns an empty reference.
    */
    static css::uno::Reference<css::presentation::XSlideShowController> GetSlideShowController (
        const css::uno::Reference<css::frame::XController>& rxController);

private:
    PresenterHelper (void);
    ~PresenterHelper (void);
};

} } // end of namespace presenter

#endif
