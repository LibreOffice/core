/*************************************************************************
 *
 *  $RCSfile: svdomedia.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:05:34 $
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

#include "svdomedia.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRMEDIAOBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#endif


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

FASTBOOL SdrMediaObj::HasTextEdit() const
{
    return FALSE;
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

UINT16 SdrMediaObj::GetObjIdentifier() const
{
    return UINT16( OBJ_MEDIA );
}

// ------------------------------------------------------------------------------

sal_Bool SdrMediaObj::DoPaintObject(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    return true;
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
        setMediaProperties( static_cast< const SdrMediaObj& >( rObj ).getMediaProperties() );
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

void SdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewProperties )
{
    const sal_uInt32 nMaskSet = rNewProperties.getMaskSet();

    // use only a subset of MediaItem properties for own own properties
    if( AVMEDIA_SETMASK_URL & nMaskSet )
        maMediaProperties.setURL( rNewProperties.getURL() );

    if( AVMEDIA_SETMASK_LOOP & nMaskSet )
        maMediaProperties.setLoop( rNewProperties.isLoop() );

    if( AVMEDIA_SETMASK_MUTE & nMaskSet )
        maMediaProperties.setMute( rNewProperties.isMute() );

    if( AVMEDIA_SETMASK_VOLUMEDB & nMaskSet )
        maMediaProperties.setVolumeDB( rNewProperties.getVolumeDB() );

    if( AVMEDIA_SETMASK_ZOOM & nMaskSet )
        maMediaProperties.setZoom( rNewProperties.getZoom() );
}
