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

#ifndef INCLUDED_SVX_SOURCE_ACCESSIBILITY_ACCESSIBLEEMPTYEDITSOURCE_HXX
#define INCLUDED_SVX_SOURCE_ACCESSIBILITY_ACCESSIBLEEMPTYEDITSOURCE_HXX

#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>

#include <memory>
#include <editeng/unoedsrc.hxx>

class SdrObject;
class SdrView;
namespace vcl { class Window; }

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
        AccessibleEmptyEditSource(  SdrObject& rObj, SdrView& rView, const vcl::Window& rViewWindow );
        virtual ~AccessibleEmptyEditSource();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder() override;
        SvxViewForwarder*       GetViewForwarder() override;

        SvxEditSource*          Clone() const override;

        // this method internally switches from empty to proxy mode,
        // creating an SvxTextEditSource for the functionality.
        SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) override;

        void                    UpdateData() override;
        SfxBroadcaster&         GetBroadcaster() const override;

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    private:
        void Switch2ProxyEditSource();

        /** Pointer to edit source implementation. This is switched on
            a GetEditViewForwarder( true ) call, to actually create a
            SvxTextEditSource.

            @dyn
         */
        std::unique_ptr< SvxEditSource >  mpEditSource;

        SdrObject&                      mrObj;
        SdrView&                        mrView;
        const vcl::Window&                   mrViewWindow;

        bool                            mbEditSourceEmpty;
    };

} // namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
