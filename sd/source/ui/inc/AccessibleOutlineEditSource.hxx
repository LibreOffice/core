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
#if 1

#include <memory>
#include <vcl/svapp.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/editdata.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>

class OutlinerView;
class SdrOutliner;
class SdrView;
class Window;

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
            const ::Window& rViewWindow );
        virtual ~AccessibleOutlineEditSource();

        /// This method is disabled and always returns NULL
        virtual SvxEditSource*          Clone() const;
        virtual SvxTextForwarder*       GetTextForwarder();
        virtual SvxViewForwarder*       GetViewForwarder();
        virtual SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );
        virtual void                    UpdateData();
        virtual SfxBroadcaster&         GetBroadcaster() const;

        // the view forwarder
        virtual sal_Bool        IsValid() const;
        virtual Rectangle   GetVisArea() const;
        virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
        virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    private:

        // declared, but not defined
        AccessibleOutlineEditSource( const AccessibleOutlineEditSource& );
        AccessibleOutlineEditSource& operator=( const AccessibleOutlineEditSource& );

        DECL_LINK( NotifyHdl, EENotify* );

        SdrView&                        mrView;
        const ::Window& mrWindow;
        SdrOutliner*                    mpOutliner;
        OutlinerView* mpOutlinerView;

        SvxOutlinerForwarder            mTextForwarder;
        SvxDrawOutlinerViewForwarder    mViewForwarder;

    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
