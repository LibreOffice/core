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



#ifndef _SV_XRENDER_PEER_HXX
#define _SV_XRENDER_PEER_HXX

#include <tools/prex.h>
struct _XTrap; // on some older systems this is not declared within Xrender.h
#include <X11/extensions/Xrender.h>
#include <tools/postx.h>

#include <vcl/salgtype.hxx>
#include <osl/module.h>

typedef Glyph XRenderGlyph;

class XRenderPeer
{
public:
    static XRenderPeer& GetInstance();
    int                 GetVersion() const;

    sal_uInt32          InitRenderText();

protected:
                        XRenderPeer();
                        ~XRenderPeer();
    void                InitRenderLib();

    Display*            mpDisplay;
    XRenderPictFormat*  mpStandardFormatA8;
    int                 mnRenderVersion;
    oslModule           mpRenderLib;

public:
    XRenderPictFormat* GetStandardFormatA8() const;
    XRenderPictFormat* FindStandardFormat(int nFormat) const;

    // the methods below are thin wrappers for the XRENDER API
    XRenderPictFormat* FindVisualFormat( Visual* ) const;
    XRenderPictFormat* FindPictureFormat( unsigned long nMask,
        const XRenderPictFormat& ) const;
    Picture     CreatePicture( Drawable, const XRenderPictFormat*,
                    unsigned long nDrawable, const XRenderPictureAttributes* ) const;
    void        ChangePicture( Picture, unsigned long nValueMask,
                    const XRenderPictureAttributes* ) const;
    void        SetPictureClipRegion( Picture, XLIB_Region ) const;
    void        CompositePicture( int nOp, Picture aSrc, Picture aMask, Picture aDst,
                    int nXSrc, int nYSrc, int nXMask, int nYMask,
                    int nXDst, int nYDst, unsigned nWidth, unsigned nHeight ) const;
    void        FreePicture( Picture ) const;

    GlyphSet    CreateGlyphSet() const;
    void        FreeGlyphSet( GlyphSet ) const;
    void        AddGlyph( GlyphSet, XRenderGlyph nXRGlyph, const XGlyphInfo&,
                    const char* pBuffer, int nBufSize ) const;
    void        FreeGlyph( GlyphSet, XRenderGlyph nXRGlyphId ) const;
    void        CompositeString32( Picture aSrc, Picture aDst, GlyphSet,
                    int nDstX, int nDstY, const unsigned* pText, int nTextLen ) const;
    void        FillRectangle( int nOp, Picture aDst, const XRenderColor*,
                               int nX, int nY, unsigned nW, unsigned nH ) const;
    void        CompositeTrapezoids( int nOp, Picture aSrc, Picture aDst,
                    const XRenderPictFormat*, int nXSrc, int nYSrc,
                    const XTrapezoid*, int nCount ) const;
    bool        AddTraps( Picture aDst, int nXOfs, int nYOfs,
                    const _XTrap*, int nCount ) const;

    bool        AreTrapezoidsSupported() const
#ifdef XRENDER_LINK
                    { return true; }
#else
                    { return mpXRenderCompositeTrapezoids!=NULL; }

private:
    XRenderPictFormat* (*mpXRenderFindFormat)(Display*,unsigned long,
        const XRenderPictFormat*,int);
    XRenderPictFormat* (*mpXRenderFindVisualFormat)(Display*,Visual*);
    XRenderPictFormat* (*mpXRenderFindStandardFormat)(Display*,int);
    Bool        (*mpXRenderQueryExtension)(Display*,int*,int*);
    void        (*mpXRenderQueryVersion)(Display*,int*,int*);

    Picture     (*mpXRenderCreatePicture)(Display*,Drawable, const XRenderPictFormat*,
                    unsigned long,const XRenderPictureAttributes*);
    void        (*mpXRenderChangePicture)(Display*,Picture,
                    unsigned long,const XRenderPictureAttributes*);
    void        (*mpXRenderSetPictureClipRegion)(Display*,Picture,XLIB_Region);
    void        (*mpXRenderFreePicture)(Display*,Picture);
    void        (*mpXRenderComposite)(Display*,int,Picture,Picture,Picture,
                    int,int,int,int,int,int,unsigned,unsigned);

    GlyphSet    (*mpXRenderCreateGlyphSet)(Display*, const XRenderPictFormat*);
    void        (*mpXRenderFreeGlyphSet)(Display*,GlyphSet);
    void        (*mpXRenderAddGlyphs)(Display*,GlyphSet,Glyph*,
                    const XGlyphInfo*,int,const char*,int);
    void        (*mpXRenderFreeGlyphs)(Display*,GlyphSet,Glyph*,int);
    void        (*mpXRenderCompositeString32)(Display*,int,Picture,Picture,
                    const XRenderPictFormat*,GlyphSet,int,int,int,int,const unsigned*,int);
    void        (*mpXRenderFillRectangle)(Display*,int,Picture,
                    const XRenderColor*,int,int,unsigned int,unsigned int);
    void        (*mpXRenderCompositeTrapezoids)(Display*,int,Picture,Picture,
                    const XRenderPictFormat*,int,int,const XTrapezoid*,int);
    void        (*mpXRenderAddTraps)(Display*,Picture,int,int,const _XTrap*,int);
#endif // XRENDER_LINK
};

//=====================================================================

class ScopedPic
{
public:
               ScopedPic( XRenderPeer& rPeer, Picture& rPic );
               ~ScopedPic();
    Picture&   Get();

private:
    XRenderPeer& mrRenderPeer;
    Picture      maPicture;

private: // prevent copy and assignmet
           ScopedPic( const ScopedPic& );
    void   operator=( const ScopedPic& );
};

//=====================================================================

inline int XRenderPeer::GetVersion() const
{
    return mnRenderVersion;
}

inline XRenderPictFormat* XRenderPeer::GetStandardFormatA8() const
{
    return mpStandardFormatA8;
}

inline XRenderPictFormat* XRenderPeer::FindStandardFormat(int nFormat) const
{
#ifdef XRENDER_LINK
    return XRenderFindStandardFormat(mpDisplay, nFormat);
#else
    return (*mpXRenderFindStandardFormat)(mpDisplay, nFormat);
#endif
}

inline XRenderPictFormat* XRenderPeer::FindVisualFormat( Visual* pVisual ) const
{
#ifdef XRENDER_LINK
    return XRenderFindVisualFormat ( mpDisplay, pVisual );
#else
    return (*mpXRenderFindVisualFormat)( mpDisplay, pVisual );
#endif
}

inline XRenderPictFormat* XRenderPeer::FindPictureFormat( unsigned long nFormatMask,
    const XRenderPictFormat& rFormatAttr ) const
{
#ifdef XRENDER_LINK
    return XRenderFindFormat( mpDisplay, nFormatMask, &rFormatAttr, 0 );
#else
    return (*mpXRenderFindFormat)( mpDisplay, nFormatMask, &rFormatAttr, 0 );
#endif
}

inline Picture XRenderPeer::CreatePicture( Drawable aDrawable,
    const XRenderPictFormat* pVisFormat, unsigned long nValueMask,
    const XRenderPictureAttributes* pRenderAttr ) const
{
#ifdef XRENDER_LINK
    return XRenderCreatePicture( mpDisplay, aDrawable, pVisFormat,
                                 nValueMask, pRenderAttr );
#else
    return (*mpXRenderCreatePicture)( mpDisplay, aDrawable, pVisFormat,
        nValueMask, pRenderAttr );
#endif
}

inline void XRenderPeer::ChangePicture( Picture aPicture,
    unsigned long nValueMask, const XRenderPictureAttributes* pRenderAttr ) const
{
#ifdef XRENDER_LINK
    XRenderChangePicture( mpDisplay, aPicture, nValueMask, pRenderAttr );
#else
    (*mpXRenderChangePicture)( mpDisplay, aPicture, nValueMask, pRenderAttr );
#endif
}

inline void XRenderPeer::SetPictureClipRegion( Picture aPicture,
    XLIB_Region aXlibRegion ) const
{
#ifdef XRENDER_LINK
    XRenderSetPictureClipRegion( mpDisplay, aPicture, aXlibRegion );
#else
    (*mpXRenderSetPictureClipRegion)( mpDisplay, aPicture, aXlibRegion );
#endif
}

inline void XRenderPeer::CompositePicture( int nXRenderOp,
    Picture aSrcPic, Picture aMaskPic, Picture aDstPic,
    int nSrcX, int nSrcY, int nMaskX, int nMaskY, int nDstX, int nDstY,
    unsigned nWidth, unsigned nHeight ) const
{
#ifdef XRENDER_LINK
    XRenderComposite( mpDisplay, nXRenderOp, aSrcPic, aMaskPic, aDstPic,
                      nSrcX, nSrcY, nMaskX, nMaskY, nDstX, nDstY, nWidth, nHeight );
#else
    (*mpXRenderComposite)( mpDisplay, nXRenderOp, aSrcPic, aMaskPic, aDstPic,
        nSrcX, nSrcY, nMaskX, nMaskY, nDstX, nDstY, nWidth, nHeight );
#endif
}

inline void XRenderPeer::FreePicture( Picture aPicture ) const
{
#ifdef XRENDER_LINK
    XRenderFreePicture( mpDisplay, aPicture );
#else
    (*mpXRenderFreePicture)( mpDisplay, aPicture );
#endif
}

inline GlyphSet XRenderPeer::CreateGlyphSet() const
{
#ifdef XRENDER_LINK
    return XRenderCreateGlyphSet( mpDisplay, mpStandardFormatA8 );
#else
    return (*mpXRenderCreateGlyphSet)( mpDisplay, mpStandardFormatA8 );
#endif
}

inline void XRenderPeer::FreeGlyphSet( GlyphSet aGS ) const
{
#ifdef XRENDER_LINK
    XRenderFreeGlyphSet( mpDisplay, aGS );
#else
    (*mpXRenderFreeGlyphSet)( mpDisplay, aGS );
#endif
}

inline void XRenderPeer::AddGlyph( GlyphSet aGS, XRenderGlyph nXRGlyph,
    const XGlyphInfo& rGI, const char* pBuffer, int nBufSize ) const
{
#ifdef XRENDER_LINK
    XRenderAddGlyphs( mpDisplay, aGS, &nXRGlyph, &rGI, 1,
                      const_cast<char*>(pBuffer), nBufSize );
#else
    (*mpXRenderAddGlyphs)( mpDisplay, aGS, &nXRGlyph, &rGI, 1,
        const_cast<char*>(pBuffer), nBufSize );
#endif
}

inline void XRenderPeer::FreeGlyph( GlyphSet aGS, XRenderGlyph nXRGlyph ) const
{
    (void)aGS; (void)nXRGlyph;

    // XRenderFreeGlyphs not implemented yet for version<=0.2
    // #108209# disabled because of crash potential,
    // the glyph leak is not too bad because they will
    // be cleaned up when the glyphset is released
#if 0 // TODO: reenable when it works without problems
    if( mnRenderVersion >= 0x05 )
    {
#ifdef XRENDER_LINK
        XRenderFreeGlyphs( mpDisplay, aGS, &nXRGlyph, 1 );
#else
        (*mpXRenderFreeGlyphs)( mpDisplay, aGS, &nXRGlyph, 1 );
#endif
    }
#endif
}

inline void XRenderPeer::CompositeString32( Picture aSrc, Picture aDst,
    GlyphSet aGlyphSet, int nDstX, int nDstY,
    const unsigned* pText, int nTextLen ) const
{
#ifdef XRENDER_LINK
    XRenderCompositeString32( mpDisplay, PictOpOver, aSrc, aDst, NULL,
                              aGlyphSet, 0, 0, nDstX, nDstY, pText, nTextLen );
#else
    (*mpXRenderCompositeString32)( mpDisplay, PictOpOver, aSrc, aDst, NULL,
        aGlyphSet, 0, 0, nDstX, nDstY, pText, nTextLen );
#endif
}

inline void XRenderPeer::FillRectangle( int a, Picture b, const XRenderColor* c,
    int d, int e, unsigned int f, unsigned int g) const
{
#ifdef XRENDER_LINK
    XRenderFillRectangle( mpDisplay, a, b, c, d, e, f, g );
#else
    (*mpXRenderFillRectangle)( mpDisplay, a, b, c, d, e, f, g );
#endif
}


inline void XRenderPeer::CompositeTrapezoids( int nOp,
    Picture aSrc, Picture aDst, const XRenderPictFormat* pXRPF,
    int nXSrc, int nYSrc, const XTrapezoid* pXT, int nCount ) const
{
#ifdef XRENDER_LINK
    XRenderCompositeTrapezoids( mpDisplay, nOp, aSrc, aDst, pXRPF,
        nXSrc, nYSrc, pXT, nCount );
#else
    (*mpXRenderCompositeTrapezoids)( mpDisplay, nOp, aSrc, aDst, pXRPF,
        nXSrc, nYSrc, pXT, nCount );
#endif
}

inline bool XRenderPeer::AddTraps( Picture aDst, int nXOfs, int nYOfs,
    const _XTrap* pTraps, int nCount ) const
{
#ifdef XRENDER_LINK
    XRenderAddTraps( mpDisplay, aDst, nXOfs, nYOfs, pTraps, nCount );
#else
    if( !mpXRenderAddTraps )
        return false;
    (*mpXRenderAddTraps)( mpDisplay, aDst, nXOfs, nYOfs, pTraps, nCount );
#endif
    return true;
}

//=====================================================================

inline ScopedPic::ScopedPic( XRenderPeer& rPeer, Picture& rPic )
:   mrRenderPeer( rPeer)
,   maPicture( rPic )
{}

inline ScopedPic::~ScopedPic()
{
    if( maPicture )
        mrRenderPeer.FreePicture( maPicture );
}

inline Picture& ScopedPic::Get()
{
    return maPicture;
}

//=====================================================================

inline XRenderColor GetXRenderColor( const SalColor& rSalColor, double fTransparency = 0.0 )
{
    XRenderColor aRetVal;
    // convert the SalColor
    aRetVal.red   = SALCOLOR_RED(   rSalColor ); aRetVal.red   |= (aRetVal.red   << 8);
    aRetVal.green = SALCOLOR_GREEN( rSalColor ); aRetVal.green |= (aRetVal.green << 8);
    aRetVal.blue  = SALCOLOR_BLUE(  rSalColor ); aRetVal.blue  |= (aRetVal.blue  << 8);

    // handle transparency
    aRetVal.alpha = 0xFFFF; // default to opaque
    if( fTransparency != 0 )
    {
        const double fAlpha = 1.0 - fTransparency;
        aRetVal.alpha = static_cast<sal_uInt16>(fAlpha * 0xFFFF + 0.5);
        // xrender wants pre-multiplied colors
        aRetVal.red   = static_cast<sal_uInt16>(fAlpha * aRetVal.red + 0.5);
        aRetVal.green = static_cast<sal_uInt16>(fAlpha * aRetVal.green + 0.5);
        aRetVal.blue  = static_cast<sal_uInt16>(fAlpha * aRetVal.blue + 0.5);
    }

    return aRetVal;
}

//=====================================================================

#endif // _SV_XRENDER_PEER_HXX
