/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vcl/svapp.hxx>

#include <svx/svdomedia.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/svdlegacy.hxx>

// ---------------
// - SdrMediaObj -
// ---------------

SdrMediaObj::SdrMediaObj(
    SdrModel& rSdrModel,
    const basegfx::B2DHomMatrix& rTransform)
:   SdrRectObj(
        rSdrModel,
        rTransform)
{
}

// ------------------------------------------------------------------------------

SdrMediaObj::~SdrMediaObj()
{
}

void SdrMediaObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrMediaObj* pSource = dynamic_cast< const SdrMediaObj* >(&rSource);

        if(pSource)
{
            // call parent
            SdrRectObj::copyDataFromSdrObject(rSource);

            // copy local data
            setMediaProperties(pSource->getMediaProperties());
            setGraphic(pSource->mapGraphic.get());
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrMediaObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrMediaObj* pClone = new SdrMediaObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
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
    rInfo.mbSelectAllowed = true;
    rInfo.mbMoveAllowed = true;
    rInfo.mbResizeFreeAllowed = true;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = false;
    rInfo.mbRotate90Allowed = false;
    rInfo.mbMirrorFreeAllowed = false;
    rInfo.mbMirror45Allowed = false;
    rInfo.mbMirror90Allowed = false;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = false;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbNoOrthoDesired = false;
    rInfo.mbNoContortion = false;
    rInfo.mbCanConvToPath = false;
    rInfo.mbCanConvToPoly = false;
    rInfo.mbCanConvToContour = false;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
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

void SdrMediaObj::AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool bShrinkOnly /* = false */ )
{
    const Size aLogicSize(Application::GetDefaultDevice()->PixelToLogic(getPreferredSize(), MAP_100TH_MM));
    basegfx::B2DVector aSize(aLogicSize.Width(), aLogicSize.Height());
    const basegfx::B2DVector aMaxSize(rMaxRange.getRange());

    if(!aSize.equalZero())
    {
        basegfx::B2DPoint aPos(rMaxRange.getMinimum());

        if(!bShrinkOnly
            || basegfx::fTools::more(aSize.getY(), aMaxSize.getY())
            || basegfx::fTools::more(aSize.getX(), aMaxSize.getX()))
        {
             if(!basegfx::fTools::equalZero(aSize.getX()) && !basegfx::fTools::equalZero(aMaxSize.getY()))
            {
                const double fScaleGraphic(aSize.getX() / aSize.getY());
                const double fScaleLimit(aMaxSize.getX() / aMaxSize.getY());

                if(basegfx::fTools::less(fScaleGraphic, fScaleLimit))
                {
                    aSize.setX(aMaxSize.getY() * fScaleGraphic);
                    aSize.setY(aMaxSize.getY());
                }
                else if(basegfx::fTools::more(fScaleGraphic, 0.0))
                {
                    aSize.setX(aMaxSize.getX());
                    aSize.setY(aMaxSize.getX() / fScaleGraphic);
                }

                aPos = rMaxRange.getCenter();
            }
        }

        if( bShrinkOnly )
        {
            aPos = getSdrObjectTranslate();
        }

        aPos -= aSize * 0.5;

        sdr::legacy::SetLogicRange(*this, basegfx::B2DRange(aPos, aPos + aSize));
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

// eof
