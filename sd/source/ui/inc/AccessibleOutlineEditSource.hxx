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

#pragma once

#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>

class OutlinerView;
class SdrOutliner;
class SdrView;
namespace vcl { class Window; }

namespace accessibility
{
    /** Implementation of the SvxEditSource interface in the SdOutlineView

        This class connects the SdOutlineView and its EditEngine
        outliner with the AccessibleTextHelper, which provides all
        necessary functionality to make the outliner text accessible

        @see SvxEditSource
        @see SvxViewForwarder
    */
    class AccessibleOutlineEditSource : public SvxEditSource, public SvxViewForwarder, public SfxBroadcaster, public SfxListener
    {
    public:
        /// Create an SvxEditSource interface for the given Outliner
        AccessibleOutlineEditSource(
            SdrOutliner& rOutliner,
            SdrView& rView,
            OutlinerView& rOutlView,
            const vcl::Window& rViewWindow );
        virtual ~AccessibleOutlineEditSource() override;

        /// This method is disabled and always returns NULL
        virtual std::unique_ptr<SvxEditSource> Clone() const override;
        virtual SvxTextForwarder*       GetTextForwarder() override;
        virtual SvxViewForwarder*       GetViewForwarder() override;
        virtual SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) override;
        virtual void                    UpdateData() override;
        virtual SfxBroadcaster&         GetBroadcaster() const override;

        // the view forwarder
        virtual bool        IsValid() const override;
        virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
        virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    private:
        AccessibleOutlineEditSource( const AccessibleOutlineEditSource& ) = delete;
        AccessibleOutlineEditSource& operator=( const AccessibleOutlineEditSource& ) = delete;

        DECL_LINK( NotifyHdl, EENotify&, void );

        SdrView&                        mrView;
        const vcl::Window& mrWindow;
        SdrOutliner*                    mpOutliner;
        OutlinerView* mpOutlinerView;

        SvxOutlinerForwarder            mTextForwarder;
        SvxDrawOutlinerViewForwarder    mViewForwarder;

    };

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
