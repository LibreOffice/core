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
    oportunity to show or hide depending on the number of available displays.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
