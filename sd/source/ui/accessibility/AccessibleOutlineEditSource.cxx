/*************************************************************************
 *
 *  $RCSfile: AccessibleOutlineEditSource.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2002-06-13 18:51:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_UNOEDHLP_HXX
#include <svx/unoedhlp.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#include <AccessibleOutlineEditSource.hxx>
#include <outlview.hxx>

namespace accessibility
{

    AccessibleOutlineEditSource::AccessibleOutlineEditSource( SdrOutliner&  rOutliner,
                                                              SdrView&      rView,
                                                              OutlinerView& rOutlView,
                                                              const Window& rViewWindow ) :
        mrView( rView ),
        mrWindow( rViewWindow ),
        mpOutliner( &rOutliner ),
        mpOutlinerView( &rOutlView ),
        mTextForwarder( rOutliner ),
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
            Rectangle aVisArea = mrView.GetVisibleArea( mrView.FindWin( const_cast< Window* > (&mrWindow) ) );

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
            ::std::auto_ptr< SfxHint > aHint( SvxEditSourceHintTranslator::EENotification2Hint( aNotify) );

            if( aHint.get() )
                Broadcast( *aHint.get() );
        }

        return 0;
    }

} // end of namespace accessibility
