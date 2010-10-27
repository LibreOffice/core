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

#ifndef _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX
#define _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX

#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>

#include <memory>
#include <editeng/unoedsrc.hxx>

class SdrObject;
class SdrView;
class Window;

namespace accessibility
{
    /** Proxy edit source for shapes without text

        Extracted from old SvxDummyEditSource
     */
    class AccessibleEmptyEditSource : public SvxEditSource, public SfxListener, public SfxBroadcaster
    {
    public:
        /** Create proxy edit source for shapes without text

            Since the views don't broadcast their dying, make sure that
            this object gets destroyed if the view becomes invalid

            The window is necessary, since our views can display on multiple windows

            Make sure you only create such an object if the shape _really_
            does not contain text.
        */
        AccessibleEmptyEditSource(  SdrObject& rObj, SdrView& rView, const Window& rViewWindow );
        ~AccessibleEmptyEditSource();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder();
        SvxViewForwarder*       GetViewForwarder();

        SvxEditSource*          Clone() const;

        // this method internally switches from empty to proxy mode,
        // creating an SvxTextEditSource for the functionality.
        SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );

        void                    UpdateData();
        SfxBroadcaster&         GetBroadcaster() const;

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    private:
        void Switch2ProxyEditSource();

        /** Pointer to edit source implementation. This is switched on
            a GetEditViewForwarder( true ) call, to actually create a
            SvxTextEditSource.

            @dyn
         */
        std::auto_ptr< SvxEditSource >  mpEditSource;

        SdrObject&                      mrObj;
        SdrView&                        mrView;
        const Window&                   mrViewWindow;

        bool                            mbEditSourceEmpty;
    };

} // namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
