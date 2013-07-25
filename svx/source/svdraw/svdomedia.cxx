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


#include <svx/svdomedia.hxx>

#include <rtl/ustring.hxx>
#include <osl/file.hxx>

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#include <vcl/svapp.hxx>

#include <svx/svdmodel.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediawindow.hxx>



using namespace ::com::sun::star;

// ---------------
// - SdrMediaObj -
// ---------------

// Note: the temp file is read only, until it is deleted!
// It may be shared between multiple documents in case of copy/paste,
// hence the shared_ptr.
struct MediaTempFile
{
    OUString const m_TempFileURL;
    MediaTempFile(OUString const& rURL) : m_TempFileURL(rURL) {}
    ~MediaTempFile()
    {
        ::osl::File::remove(m_TempFileURL);
    }
};

struct SdrMediaObj::Impl
{
    ::avmedia::MediaItem                  m_MediaProperties;
    ::boost::shared_ptr< MediaTempFile >  m_pTempFile;
    uno::Reference< graphic::XGraphic >   m_xCachedSnapshot;
};

TYPEINIT1( SdrMediaObj, SdrRectObj );

// ------------------------------------------------------------------------------

SdrMediaObj::SdrMediaObj()
    : SdrRectObj()
    , m_pImpl( new Impl() )
{
}

// ------------------------------------------------------------------------------

SdrMediaObj::SdrMediaObj( const Rectangle& rRect )
    : SdrRectObj( rRect )
    , m_pImpl( new Impl() )
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

OUString SdrMediaObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulMEDIA));

    OUString aName(GetName());

    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

// ------------------------------------------------------------------------------

void SdrMediaObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralMEDIA);
}

// ------------------------------------------------------------------------------

SdrMediaObj* SdrMediaObj::Clone() const
{
    return CloneHelper< SdrMediaObj >();
}

SdrMediaObj& SdrMediaObj::operator=(const SdrMediaObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator=( rObj );

    m_pImpl->m_pTempFile = rObj.m_pImpl->m_pTempFile; // before props
    setMediaProperties( rObj.getMediaProperties() );
    m_pImpl->m_xCachedSnapshot = rObj.m_pImpl->m_xCachedSnapshot;
    return *this;
}

uno::Reference< graphic::XGraphic > SdrMediaObj::getSnapshot()
{
    if( !m_pImpl->m_xCachedSnapshot.is() )
    {
        OUString aRealURL = m_pImpl->m_MediaProperties.getTempURL();
        if( aRealURL.isEmpty() )
            aRealURL = m_pImpl->m_MediaProperties.getURL();
        m_pImpl->m_xCachedSnapshot = avmedia::MediaWindow::grabFrame( aRealURL, true );
    }
    return m_pImpl->m_xCachedSnapshot;
}

// ------------------------------------------------------------------------------

void SdrMediaObj::AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly /* = false */ )
{
    Size aSize( Application::GetDefaultDevice()->PixelToLogic( getPreferredSize(), MAP_100TH_MM ) );
    Size aMaxSize( rMaxRect.GetSize() );

    if( aSize.Height() != 0 && aSize.Width() != 0 )
    {
        Point aPos( rMaxRect.TopLeft() );

        // if graphic is too large, fit it to the page
        if ( (!bShrinkOnly                          ||
             ( aSize.Height() > aMaxSize.Height() ) ||
             ( aSize.Width()  > aMaxSize.Width()  ) )&&
             aSize.Height() && aMaxSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aMaxSize.Width() /
                            (float)aMaxSize.Height();

            // scale graphic to page size
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

void SdrMediaObj::setURL( const OUString& rURL)
{
    ::avmedia::MediaItem aURLItem;

    aURLItem.setURL( rURL, 0 );
    setMediaProperties( aURLItem );
}

// ------------------------------------------------------------------------------

const OUString& SdrMediaObj::getURL() const
{
    return m_pImpl->m_MediaProperties.getURL();
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
    return m_pImpl->m_MediaProperties;
}

// ------------------------------------------------------------------------------

Size SdrMediaObj::getPreferredSize() const
{
    return static_cast< ::sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).getPreferredSize();
}

// ------------------------------------------------------------------------------

uno::Reference<io::XInputStream> SdrMediaObj::GetInputStream()
{
    if (!m_pImpl->m_pTempFile)
    {
        SAL_WARN("svx", "this is only intended for embedded media");
        return 0;
    }
    ucbhelper::Content tempFile(m_pImpl->m_pTempFile->m_TempFileURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());
    return tempFile.openStream();
}

/// copy a stream from XStorage to temp file
static bool lcl_HandlePackageURL(
        OUString const & rURL,
        SdrModel *const pModel,
        OUString & o_rTempFileURL)
{
    if (!pModel)
    {
        SAL_WARN("svx", "no model");
        return false;
    }
    ::comphelper::LifecycleProxy sourceProxy;
    uno::Reference<io::XInputStream> xInStream;
    try {
        xInStream = pModel->GetDocumentStream(rURL, sourceProxy);
    }
    catch (container::NoSuchElementException const&)
    {
        SAL_INFO("svx", "not found: '" << OUString(rURL) << "'");
        return false;
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("svx", "exception: '" << e.Message << "'");
        return false;
    }
    if (!xInStream.is())
    {
        SAL_WARN("svx", "no stream?");
        return false;
    }

    OUString tempFileURL;
    ::osl::FileBase::RC const err =
        ::osl::FileBase::createTempFile(0, 0, & tempFileURL);
    if (::osl::FileBase::E_None != err)
    {
        SAL_INFO("svx", "cannot create temp file");
        return false;
    }

    try
    {
        ::ucbhelper::Content tempContent(tempFileURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());
        tempContent.writeStream(xInStream, true); // copy stream to file
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("svx", "exception: '" << e.Message << "'");
        return false;
    }
    o_rTempFileURL = tempFileURL;
    return true;
}

static char const s_PkgScheme[] = "vnd.sun.star.Package:";

void SdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewProperties )
{
    bool bBroadcastChanged = false;
    const sal_uInt32 nMaskSet = rNewProperties.getMaskSet();

    // use only a subset of MediaItem properties for own own properties
    if( ( AVMEDIA_SETMASK_URL & nMaskSet ) &&
        ( rNewProperties.getURL() != getURL() ))
    {
        m_pImpl->m_xCachedSnapshot.clear();
        OUString const url(rNewProperties.getURL());
        if ((0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
                url.getStr(), url.getLength(),
                s_PkgScheme, SAL_N_ELEMENTS(s_PkgScheme) - 1)))
        {
            if (   !m_pImpl->m_pTempFile
                || (m_pImpl->m_pTempFile->m_TempFileURL !=
                                rNewProperties.getTempURL()))
            {
                OUString tempFileURL;
                bool const bSuccess = lcl_HandlePackageURL(
                    url, GetModel(), tempFileURL);
                if (bSuccess)
                {
                    m_pImpl->m_pTempFile.reset(new MediaTempFile(tempFileURL));
                    m_pImpl->m_MediaProperties.setURL(url, & tempFileURL);
                }
                else // this case is for Clone via operator=
                {
                    m_pImpl->m_pTempFile.reset();
                    m_pImpl->m_MediaProperties.setURL(OUString(), 0);
                }
            }
            else
            {
                m_pImpl->m_MediaProperties.setURL(url,
                        &rNewProperties.getTempURL());
            }
        }
        else
        {
            m_pImpl->m_pTempFile.reset();
            m_pImpl->m_MediaProperties.setURL(url, 0);
        }
        bBroadcastChanged = true;
    }

    if( AVMEDIA_SETMASK_LOOP & nMaskSet )
        m_pImpl->m_MediaProperties.setLoop( rNewProperties.isLoop() );

    if( AVMEDIA_SETMASK_MUTE & nMaskSet )
        m_pImpl->m_MediaProperties.setMute( rNewProperties.isMute() );

    if( AVMEDIA_SETMASK_VOLUMEDB & nMaskSet )
        m_pImpl->m_MediaProperties.setVolumeDB( rNewProperties.getVolumeDB() );

    if( AVMEDIA_SETMASK_ZOOM & nMaskSet )
        m_pImpl->m_MediaProperties.setZoom( rNewProperties.getZoom() );

    if( bBroadcastChanged )
    {
        SetChanged();
        BroadcastObjectChange();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
