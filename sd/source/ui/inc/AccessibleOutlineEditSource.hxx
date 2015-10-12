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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEOUTLINEEDITSOURCE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEOUTLINEEDITSOURCE_HXX

#include <vcl/svapp.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/editdata.hxx>
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
        virtual ~AccessibleOutlineEditSource();

        /// This method is disabled and always returns NULL
        virtual SvxEditSource*          Clone() const SAL_OVERRIDE;
        virtual SvxTextForwarder*       GetTextForwarder() SAL_OVERRIDE;
        virtual SvxViewForwarder*       GetViewForwarder() SAL_OVERRIDE;
        virtual SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) SAL_OVERRIDE;
        virtual void                    UpdateData() SAL_OVERRIDE;
        virtual SfxBroadcaster&         GetBroadcaster() const SAL_OVERRIDE;

        // the view forwarder
        virtual bool        IsValid() const SAL_OVERRIDE;
        virtual Rectangle   GetVisArea() const SAL_OVERRIDE;
        virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const SAL_OVERRIDE;
        virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const SAL_OVERRIDE;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    private:
        AccessibleOutlineEditSource( const AccessibleOutlineEditSource& ) = delete;
        AccessibleOutlineEditSource& operator=( const AccessibleOutlineEditSource& ) = delete;

        DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

        SdrView&                        mrView;
        const vcl::Window& mrWindow;
        SdrOutliner*                    mpOutliner;
        OutlinerView* mpOutlinerView;

        SvxOutlinerForwarder            mTextForwarder;
        SvxDrawOutlinerViewForwarder    mViewForwarder;

    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
