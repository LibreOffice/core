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

#include <memory>
#include <editeng/unoedhlp.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdview.hxx>

#include <AccessibleOutlineEditSource.hxx>

namespace accessibility
{

    AccessibleOutlineEditSource::AccessibleOutlineEditSource(
        SdrOutliner&    rOutliner,
        SdrView&        rView,
        OutlinerView& rOutlView,
        const vcl::Window& rViewWindow )
        : mrView( rView ),
          mrWindow( rViewWindow ),
          mpOutliner( &rOutliner ),
          mpOutlinerView( &rOutlView ),
          mTextForwarder( rOutliner, false ),
          mViewForwarder( rOutlView )
    {
        // register as listener - need to broadcast state change messages
        // Moved to ::GetTextForwarder()
        //rOutliner.SetNotifyHdl( LINK(this, AccessibleOutlineEditSource, NotifyHdl) );
        StartListening(rOutliner);
    }

    AccessibleOutlineEditSource::~AccessibleOutlineEditSource()
    {
        if( mpOutliner )
            mpOutliner->SetNotifyHdl( Link<EENotify&,void>() );
        Broadcast( TextHint( SfxHintId::Dying ) );
    }

    std::unique_ptr<SvxEditSource> AccessibleOutlineEditSource::Clone() const
    {
        return std::unique_ptr<SvxEditSource>(new AccessibleOutlineEditSource(*mpOutliner, mrView, *mpOutlinerView, mrWindow));
    }

    SvxTextForwarder* AccessibleOutlineEditSource::GetTextForwarder()
    {
        // TODO: maybe suboptimal
        if( IsValid() )
        {
            // Moved here to make sure that
            // the NotifyHandler was set on the current object.
            mpOutliner->SetNotifyHdl( LINK(this, AccessibleOutlineEditSource, NotifyHdl) );
            return &mTextForwarder;
        }
        else
            return nullptr;
    }

    SvxViewForwarder* AccessibleOutlineEditSource::GetViewForwarder()
    {
        // TODO: maybe suboptimal
        if( IsValid() )
            return this;
        else
            return nullptr;
    }

    SvxEditViewForwarder* AccessibleOutlineEditSource::GetEditViewForwarder( bool )
    {
        // TODO: maybe suboptimal
        if( IsValid() )
        {
            // ignore parameter, we're always in edit mode here
            return &mViewForwarder;
        }
        else
            return nullptr;
    }

    void AccessibleOutlineEditSource::UpdateData()
    {
        // NOOP, since we're always working on the 'real' outliner,
        // i.e. changes are immediately reflected on the screen
    }

    SfxBroadcaster& AccessibleOutlineEditSource::GetBroadcaster() const
    {
        return * const_cast< AccessibleOutlineEditSource* > (this);
    }

    bool AccessibleOutlineEditSource::IsValid() const
    {
        if( mpOutliner && mpOutlinerView )
        {
            // Our view still on outliner?
            sal_uLong nCurrView, nViews;

            for( nCurrView=0, nViews=mpOutliner->GetViewCount(); nCurrView<nViews; ++nCurrView )
            {
                if( mpOutliner->GetView(nCurrView) == mpOutlinerView )
                    return true;
            }
        }

        return false;
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
            if( rHint.GetId() == SfxHintId::Dying )
            {
                bDispose = true;
                mpOutliner = nullptr;
            }
        }
        else
        {
            const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );

            if( pSdrHint && ( pSdrHint->GetKind() == SdrHintKind::ModelCleared ) )
            {
                // model is dying under us, going defunc
                bDispose = true;
            }
        }

        if( bDispose )
        {
            if( mpOutliner )
                mpOutliner->SetNotifyHdl( Link<EENotify&,void>() );
            mpOutliner = nullptr;
            mpOutlinerView = nullptr;
            Broadcast( TextHint( SfxHintId::Dying ) );
        }
    }

    IMPL_LINK(AccessibleOutlineEditSource, NotifyHdl, EENotify&, rNotify, void)
    {
        ::std::unique_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( &rNotify) );

        if (aHint)
        {
            Broadcast(*aHint);
         }
    }

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
