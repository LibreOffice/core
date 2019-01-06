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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWRESTARTER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWRESTARTER_HXX

#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <memory>

namespace sd { class SlideShow; }
namespace sd { class ViewShellBase; }
class SfxDispatcher;
struct ImplSVEvent;

namespace sd {

/** This class is used when a display is removed or added to restart the
    slide show.  This is necessary at least with DirectX because
    deactivating a display invalidates DirectX resources.  Accessing those
    leads to a crash.

    During a restart a possibly installed presenter extension is given the
    opportunity to show or hide depending on the number of available displays.
*/
class SlideShowRestarter
    : public std::enable_shared_from_this<SlideShowRestarter>
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
    virtual ~SlideShowRestarter();

    /** Restarting the slide show is an asynchronous multi step process
        which is started by calling this method.
        @param bForce
           Used to force a re-start, even if the display count is unchanged.
    */
    void Restart (bool bForce);

private:
    ImplSVEvent * mnEventId;
    ::rtl::Reference<SlideShow> mpSlideShow;
    ViewShellBase* mpViewShellBase;
    ::std::shared_ptr<SlideShowRestarter> mpSelf;
    sal_Int32 mnDisplayCount;
    SfxDispatcher* mpDispatcher;
    sal_Int32 mnCurrentSlideNumber;

    DECL_LINK(EndPresentation, void*, void);

    /** Restart the presentation on the slide last shown before the restart
        was initiated.
    */
    void StartPresentation();
};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWRESTARTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
