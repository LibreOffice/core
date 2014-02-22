/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#include <avmedia/mediawindow.hxx>

namespace sdr { namespace contact {





ViewContactOfSdrMediaObj::ViewContactOfSdrMediaObj( SdrMediaObj& rMediaObj ) :
    ViewContactOfSdrObj( rMediaObj )
{
}



ViewContactOfSdrMediaObj::~ViewContactOfSdrMediaObj()
{
}



ViewObjectContact& ViewContactOfSdrMediaObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
    return *( new ViewObjectContactOfSdrMediaObj( rObjectContact, *this, static_cast< SdrMediaObj& >( GetSdrObject() ).getMediaProperties() ) );
}



Size ViewContactOfSdrMediaObj::getPreferredSize() const
{
    
    
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);
        Size aSize(pCandidate ? static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->getPreferredSize() : Size());

        if(0 != aSize.getWidth() || 0 != aSize.getHeight())
        {
            return aSize;
        }
    }

    return Size();
}



void ViewContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
    
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);

        if(pCandidate)
        {
            static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->updateMediaItem(rItem);
        }
    }
}



void ViewContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
    const sal_uInt32 nCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);

        if(pCandidate)
        {
            static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->executeMediaItem(rItem);
        }
    }
}



void ViewContactOfSdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState )
{
    static_cast< SdrMediaObj& >(GetSdrObject()).mediaPropertiesChanged(rNewState);
}

}} 

namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrMediaObj::createViewIndependentPrimitive2DSequence() const
        {
            
            
            Rectangle aRectangle(GetSdrMediaObj().GetGeoRect());
            
            
            
            aRectangle += GetSdrMediaObj().GetGridOffset();
            const basegfx::B2DRange aRange(
                aRectangle.Left(), aRectangle.Top(),
                aRectangle.Right(), aRectangle.Bottom());

            
            basegfx::B2DHomMatrix aTransform;
            aTransform.set(0, 0, aRange.getWidth());
            aTransform.set(1, 1, aRange.getHeight());
            aTransform.set(0, 2, aRange.getMinX());
            aTransform.set(1, 2, aRange.getMinY());

            
            
            
            const basegfx::BColor aBackgroundColor(67.0 / 255.0, 67.0 / 255.0, 67.0 / 255.0);
            const OUString& rURL(GetSdrMediaObj().getURL());
            const sal_uInt32 nPixelBorder(4L);
            const drawinglayer::primitive2d::Primitive2DReference xRetval(
                new drawinglayer::primitive2d::MediaPrimitive2D(
                    aTransform, rURL, aBackgroundColor, nPixelBorder,
                    GetSdrMediaObj().getSnapshot()));

            return drawinglayer::primitive2d::Primitive2DSequence(&xRetval, 1);
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
