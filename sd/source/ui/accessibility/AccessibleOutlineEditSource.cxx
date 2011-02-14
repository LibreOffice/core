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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <editeng/unoedhlp.hxx>
#include <svx/svdoutl.hxx>

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDIT_SOURCE_HXX
#include <AccessibleOutlineEditSource.hxx>
#endif
#include "OutlineView.hxx"
#include <svx/sdrpaintwindow.hxx>

namespace accessibility
{

    AccessibleOutlineEditSource::AccessibleOutlineEditSource(
        SdrOutliner&    rOutliner,
        SdrView&        rView,
        OutlinerView& rOutlView,
        const ::Window& rViewWindow )
        : mrView( rView ),
          mrWindow( rViewWindow ),
          mpOutliner( &rOutliner ),
          mpOutlinerView( &rOutlView ),
          mTextForwarder( rOutliner, 0 ),
          mViewForwarder( rOutlView )
    {
        // register as listener - need to broadcast state change messages
        rOutliner.SetNotifyHdl( LINK(this, AccessibleOutlineEditSource, NotifyHdl) );
        StartListening(rOutliner);
    }

    AccessibleOutlineEditSource::~AccessibleOutlineEditSource()
    {
        if( mpOutliner )
            mpOutliner->SetNotifyHdl( Link() );
        Broadcast( TextHint( SFX_HINT_DYING ) );
    }

    SvxEditSource* AccessibleOutlineEditSource::Clone() const
    {
        return NULL;
    }

    SvxTextForwarder* AccessibleOutlineEditSource::GetTextForwarder()
    {
        // TODO: maybe suboptimal
        if( IsValid() )
            return &mTextForwarder;
        else
            return NULL;
    }

    SvxViewForwarder* AccessibleOutlineEditSource::GetViewForwarder()
    {
        // TODO: maybe suboptimal
        if( IsValid() )
            return this;
        else
            return NULL;
    }

    SvxEditViewForwarder* AccessibleOutlineEditSource::GetEditViewForwarder( sal_Bool )
    {
        // TODO: maybe suboptimal
        if( IsValid() )
        {
            // ignore parameter, we're always in edit mode here
            return &mViewForwarder;
        }
        else
            return NULL;
    }

    void AccessibleOutlineEditSource::UpdateData()
    {
        // NOOP, since we're always working on the 'real' outliner,
        // i.e. changes are immediately reflected on the screen
    }

    SfxBroadcaster& AccessibleOutlineEditSource::GetBroadcaster() const
    {
        return *( const_cast< AccessibleOutlineEditSource* > (this) );
    }

    sal_Bool AccessibleOutlineEditSource::IsValid() const
    {
        if( mpOutliner && mpOutlinerView )
        {
            // Our view still on outliner?
            sal_uLong nCurrView, nViews;

            for( nCurrView=0, nViews=mpOutliner->GetViewCount(); nCurrView<nViews; ++nCurrView )
            {
                if( mpOutliner->GetView(nCurrView) == mpOutlinerView )
                    return sal_True;
            }
        }

        return sal_False;
    }

    Rectangle AccessibleOutlineEditSource::GetVisArea() const
    {
        if( IsValid() )
        {
            SdrPaintWindow* pPaintWindow = mrView.FindPaintWindow(mrWindow);
            Rectangle aVisArea;

            if(pPaintWindow)
            {
                aVisArea = pPaintWindow->GetVisibleArea();
            }

            MapMode aMapMode(mrWindow.GetMapMode());
            aMapMode.SetOrigin(Point());
            return mrWindow.LogicToPixel( aVisArea, aMapMode );
        }

        return Rectangle();
    }

    Point AccessibleOutlineEditSource::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
    {
        if( IsValid() && mrView.GetModel() )
        {
            Point aPoint( OutputDevice::LogicToLogic( rPoint, rMapMode,
                                                      MapMode(mrView.GetModel()->GetScaleUnit()) ) );
            MapMode aMapMode(mrWindow.GetMapMode());
            aMapMode.SetOrigin(Point());
            return mrWindow.LogicToPixel( aPoint, aMapMode );
        }

        return Point();
    }

    Point AccessibleOutlineEditSource::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
    {
        if( IsValid() && mrView.GetModel() )
        {
            MapMode aMapMode(mrWindow.GetMapMode());
            aMapMode.SetOrigin(Point());
            Point aPoint( mrWindow.PixelToLogic( rPoint, aMapMode ) );
            return OutputDevice::LogicToLogic( aPoint,
                                               MapMode(mrView.GetModel()->GetScaleUnit()),
                                               rMapMode );
        }

        return Point();
    }

    void AccessibleOutlineEditSource::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& rHint )
    {
        bool bDispose = false;

        if( &rBroadcaster == mpOutliner )
        {
            const SfxSimpleHint* pHint = dynamic_cast< const SfxSimpleHint * >( &rHint );
            if( pHint && (pHint->GetId() == SFX_HINT_DYING) )
            {
                bDispose = true;
                mpOutliner = NULL;
            }
        }
        else
        {
            const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );

            if( pSdrHint && ( pSdrHint->GetKind() == HINT_MODELCLEARED ) )
            {
                // model is dying under us, going defunc
                bDispose = true;
            }
        }

        if( bDispose )
        {
            if( mpOutliner )
                mpOutliner->SetNotifyHdl( Link() );
            mpOutliner = NULL;
            mpOutlinerView = NULL;
            Broadcast( TextHint( SFX_HINT_DYING ) );
        }
    }

    IMPL_LINK(AccessibleOutlineEditSource, NotifyHdl, EENotify*, aNotify)
    {
        if( aNotify )
        {
            ::std::auto_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( aNotify) );

            if( aHint.get() )
                Broadcast( *aHint.get() );
        }

        return 0;
    }

} // end of namespace accessibility
