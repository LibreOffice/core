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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDITSOURCE_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDITSOURCE_HXX

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
