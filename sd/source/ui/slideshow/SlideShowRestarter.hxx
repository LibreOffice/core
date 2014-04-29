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



#include "precompiled_sd.hxx"

#include "slideshow.hxx"
#include "ViewShellBase.hxx"
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <boost/enable_shared_from_this.hpp>

namespace sd {

/** This class is used when a display is removed or added to restart the
    slide show.  This is necessary at least with DirectX because
    deactivating a display invalidates DirectX resources.  Accessing those
    leads to a crash.

    During a restart a possibly installed presenter extension is given the
    opportunity to show or hide depending on the number of available displays.
*/
class SlideShowRestarter
    : public boost::enable_shared_from_this<SlideShowRestarter>
{
public:
    /** Create a new SlideShowRestarter object.
        @param rpSlideShow
            The slide show is used to determine the current slide, which is
            restored after the restart, and of course to stop and start the
            slide show.
        @param pViewShellBase
            Used to get access to a slot dispatcher.
    */
    SlideShowRestarter (
        const ::rtl::Reference<SlideShow>& rpSlideShow,
        ViewShellBase* pViewShellBase);
    virtual ~SlideShowRestarter (void);

    /** Restarting the slide show is an asynchronous multi step process
        which is started by calling this method.
    */
    void Restart (void);

private:
    sal_Int32 mnEventId;
    ::rtl::Reference<SlideShow> mpSlideShow;
    ViewShellBase* mpViewShellBase;
    ::boost::shared_ptr<SlideShowRestarter> mpSelf;
    sal_Int32 mnDisplayCount;
    SfxDispatcher* mpDispatcher;
    sal_Int32 mnCurrentSlideNumber;

    /** The display count is used to determine whether the number of
        displays has changed and thus whether restarting the slide show is
        really necessary.
    */
    sal_Int32 GetDisplayCount (void);

    DECL_LINK(EndPresentation, void*);

    /** Restart the presentation on the slide last shown before the restart
        was initiated.
    */
    void StartPresentation (void);
};

} // end of namespace sd
