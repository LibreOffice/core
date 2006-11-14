/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleOutlineEditSource.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:23:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVX_UNOEDHLP_HXX
#include <svx/unoedhlp.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_EDIT_SOURCE_HXX
#include <AccessibleOutlineEditSource.hxx>
#endif
#ifndef SD_OUTLINE_VIEW_HXX
#include "OutlineView.hxx"
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <svx/sdrpaintwindow.hxx>
#endif

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
          mTextForwarder( rOutliner, NULL ),
          mViewForwarder( rOutlView )
    {
        // register as listener - need to broadcast state change messages
        rOutliner.SetNotifyHdl( LINK(this, AccessibleOutlineEditSource, NotifyHdl) );
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

    BOOL AccessibleOutlineEditSource::IsValid() const
    {
        if( mpOutliner && mpOutlinerView )
        {
            // Our view still on outliner?
            ULONG nCurrView, nViews;

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

    void AccessibleOutlineEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

        if( pSdrHint )
        {
            switch( pSdrHint->GetKind() )
            {
                case HINT_MODELCLEARED:
                    // model is dying under us, going defunc
                    if( mpOutliner )
                        mpOutliner->SetNotifyHdl( Link() );
                    mpOutliner = NULL;
                    mpOutlinerView = NULL;
                    Broadcast( TextHint( SFX_HINT_DYING ) );
                    break;
            }
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
