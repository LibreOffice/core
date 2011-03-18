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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vcl/svapp.hxx>

#include <svx/svdomedia.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediawindow.hxx>

// ---------------
// - SdrMediaObj -
// ---------------

TYPEINIT1( SdrMediaObj, SdrRectObj );

// ------------------------------------------------------------------------------

SdrMediaObj::SdrMediaObj()
{
}

// ------------------------------------------------------------------------------

SdrMediaObj::SdrMediaObj( const Rectangle& rRect ) :
    SdrRectObj( rRect )
{
}

// ------------------------------------------------------------------------------

SdrMediaObj::~SdrMediaObj()
{
}

// ------------------------------------------------------------------------------

bool SdrMediaObj::HasTextEdit() const
{
    return false;
}

// ------------------------------------------------------------------------------

sdr::contact::ViewContact* SdrMediaObj::CreateObjectSpecificViewContact()
{
    return new ::sdr::contact::ViewContactOfSdrMediaObj( *this );
}

// ------------------------------------------------------------------------------

void SdrMediaObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.bSelectAllowed = true;
    rInfo.bMoveAllowed = true;
    rInfo.bResizeFreeAllowed = true;
    rInfo.bResizePropAllowed = true;
    rInfo.bRotateFreeAllowed = false;
    rInfo.bRotate90Allowed = false;
    rInfo.bMirrorFreeAllowed = false;
    rInfo.bMirror45Allowed = false;
    rInfo.bMirror90Allowed = false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bGradientAllowed = false;
    rInfo.bShearAllowed = false;
    rInfo.bEdgeRadiusAllowed = false;
    rInfo.bNoOrthoDesired = false;
    rInfo.bNoContortion = false;
    rInfo.bCanConvToPath = false;
    rInfo.bCanConvToPoly = false;
    rInfo.bCanConvToContour = false;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
}

// ------------------------------------------------------------------------------

sal_uInt16 SdrMediaObj::GetObjIdentifier() const
{
    return sal_uInt16( OBJ_MEDIA );
}

// ------------------------------------------------------------------------------

void SdrMediaObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulMEDIA);

    String aName( GetName() );

    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// ------------------------------------------------------------------------------

void SdrMediaObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralMEDIA);
}

// ------------------------------------------------------------------------------

void SdrMediaObj::operator=(const SdrObject& rObj)
{
    SdrRectObj::operator=( rObj );

    if( rObj.ISA( SdrMediaObj ) )
    {
        const SdrMediaObj& rMediaObj = static_cast< const SdrMediaObj& >( rObj );

        setMediaProperties( rMediaObj.getMediaProperties() );
        setGraphic( rMediaObj.mapGraphic.get() );
    }
}

// ------------------------------------------------------------------------------

void SdrMediaObj::AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly /* = false */ )
{
    Size aSize( Application::GetDefaultDevice()->PixelToLogic( getPreferredSize(), MAP_100TH_MM ) );
    Size aMaxSize( rMaxRect.GetSize() );

    if( aSize.Height() != 0 && aSize.Width() != 0 )
    {
        Point aPos( rMaxRect.TopLeft() );

        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ( (!bShrinkOnly                          ||
             ( aSize.Height() > aMaxSize.Height() ) ||
             ( aSize.Width()  > aMaxSize.Width()  ) )&&
             aSize.Height() && aMaxSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aMaxSize.Width() /
                            (float)aMaxSize.Height();

            // Grafik an Pagesize anpassen (skaliert)
            if ( fGrfWH < fWinWH )
            {
                aSize.Width() = (long)(aMaxSize.Height() * fGrfWH);
                aSize.Height()= aMaxSize.Height();
            }
            else if ( fGrfWH > 0.F )
            {
                aSize.Width() = aMaxSize.Width();
                aSize.Height()= (long)(aMaxSize.Width() / fGrfWH);
            }

            aPos = rMaxRect.Center();
        }

        if( bShrinkOnly )
            aPos = aRect.TopLeft();

        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        SetLogicRect( Rectangle( aPos, aSize ) );
    }
}

// ------------------------------------------------------------------------------

void SdrMediaObj::setURL( const ::rtl::OUString& rURL )
{
    ::avmedia::MediaItem aURLItem;

    aURLItem.setURL( rURL );
    setMediaProperties( aURLItem );
}

// ------------------------------------------------------------------------------

const ::rtl::OUString& SdrMediaObj::getURL() const
{
    return getMediaProperties().getURL();
}

// ------------------------------------------------------------------------------

void SdrMediaObj::setMediaProperties( const ::avmedia::MediaItem& rState )
{
    mediaPropertiesChanged( rState );
    static_cast< ::sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).executeMediaItem( getMediaProperties() );
}

// ------------------------------------------------------------------------------

const ::avmedia::MediaItem& SdrMediaObj::getMediaProperties() const
{
    return maMediaProperties;
}

// ------------------------------------------------------------------------------

bool SdrMediaObj::hasPreferredSize() const
{
    return static_cast< ::sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).hasPreferredSize();
}

// ------------------------------------------------------------------------------

Size SdrMediaObj::getPreferredSize() const
{
    return static_cast< ::sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).getPreferredSize();
}

// ------------------------------------------------------------------------------

const Graphic& SdrMediaObj::getGraphic() const
{
    if( !mapGraphic.get() )
        const_cast< SdrMediaObj* >( this )->mapGraphic.reset( new Graphic( ::avmedia::MediaWindow::grabFrame( getURL(), true ) ) );

    return *mapGraphic;
}

// ------------------------------------------------------------------------------

void SdrMediaObj::setGraphic( const Graphic* pGraphic )
{
    mapGraphic.reset( pGraphic ? new Graphic( *pGraphic ) : NULL );
}

// ------------------------------------------------------------------------------

void SdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewProperties )
{
    const sal_uInt32 nMaskSet = rNewProperties.getMaskSet();

    // use only a subset of MediaItem properties for own own properties
    if( ( AVMEDIA_SETMASK_URL & nMaskSet ) &&
        ( rNewProperties.getURL() != getURL() ) )
    {
        setGraphic();
        maMediaProperties.setURL( rNewProperties.getURL() );
    }

    if( AVMEDIA_SETMASK_LOOP & nMaskSet )
        maMediaProperties.setLoop( rNewProperties.isLoop() );

    if( AVMEDIA_SETMASK_MUTE & nMaskSet )
        maMediaProperties.setMute( rNewProperties.isMute() );

    if( AVMEDIA_SETMASK_VOLUMEDB & nMaskSet )
        maMediaProperties.setVolumeDB( rNewProperties.getVolumeDB() );

    if( AVMEDIA_SETMASK_ZOOM & nMaskSet )
        maMediaProperties.setZoom( rNewProperties.getZoom() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
