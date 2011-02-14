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

#ifndef _SVX_GALOBJ_HXX_
#define _SVX_GALOBJ_HXX_

#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>
#include "svx/galmisc.hxx"

// -----------
// - Defines -
// -----------

#define S_THUMB 80

// -----------------------------------------------------------------------------

#define SGA_FORMAT_NONE                 0x00000000L
#define SGA_FORMAT_STRING               0x00000001L
#define SGA_FORMAT_GRAPHIC              0x00000010L
#define SGA_FORMAT_SOUND                0x00000100L
#define SGA_FORMAT_OLE                  0x00001000L
#define SGA_FORMAT_SVDRAW               0x00010000L
#define SGA_FORMAT_ALL                  0xFFFFFFFFL

// ----------------
// - GalSoundType -
// ----------------

enum GalSoundType
{
    SOUND_STANDARD = 0,
    SOUND_COMPUTER = 1,
    SOUND_MISC = 2,
    SOUND_MUSIC = 3,
    SOUND_NATURE = 4,
    SOUND_SPEECH = 5,
    SOUND_TECHNIC = 6,
    SOUND_ANIMAL = 7
};

// -------------
// - SgaObject -
// -------------

class SVX_DLLPUBLIC SgaObject
{
    friend class GalleryTheme;

private:

    void                    ImplUpdateURL( const INetURLObject& rNewURL ) { aURL = rNewURL; }

protected:

    Bitmap                  aThumbBmp;
    GDIMetaFile             aThumbMtf;
    INetURLObject           aURL;
    String                  aUserName;
    String                  aTitle;
    sal_Bool                    bIsValid;
    sal_Bool                    bIsThumbBmp;

    virtual void            WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void            ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    sal_Bool                    CreateThumb( const Graphic& rGraphic );

public:

                            SgaObject();
    virtual                 ~SgaObject() {};

    virtual SgaObjKind      GetObjKind() const = 0;
    virtual sal_uInt16          GetVersion() const = 0;

    virtual Bitmap          GetThumbBmp() const { return aThumbBmp; }
    const GDIMetaFile&      GetThumbMtf() const { return aThumbMtf; }
    const INetURLObject&    GetURL() const { return aURL; }
    sal_Bool                    IsValid() const { return bIsValid; }
    sal_Bool                    IsThumbBitmap() const { return bIsThumbBmp; }

    const String            GetTitle() const;
    void                    SetTitle( const String& rTitle );

    friend SvStream&        operator<<( SvStream& rOut, const SgaObject& rObj );
    friend SvStream&        operator>>( SvStream& rIn, SgaObject& rObj );
};

// ------------------
// - SgaObjectSound -
// ------------------

class SgaObjectSound : public SgaObject
{
private:

    GalSoundType        eSoundType;

    virtual void        WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 6; }

public:

                        SgaObjectSound();
                        SgaObjectSound( const INetURLObject& rURL );
    virtual            ~SgaObjectSound();

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_SOUND; }
    virtual Bitmap      GetThumbBmp() const;
    GalSoundType        GetSoundType() const { return eSoundType; }
};

// -------------------
// - SgaObjectSvDraw -
// -------------------

class FmFormModel;

class SgaObjectSvDraw : public SgaObject
{
    using SgaObject::CreateThumb;

private:

    sal_Bool                CreateThumb( const FmFormModel& rModel );

    virtual void        WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 5; }

public:

                        SgaObjectSvDraw();
                        SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL );
                        SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL );
    virtual            ~SgaObjectSvDraw() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_SVDRAW; }

public:

    static sal_Bool         DrawCentered( OutputDevice* pOut, const FmFormModel& rModel );
};

// ----------------
// - SgaObjectBmp -
// ----------------

class SgaObjectBmp: public SgaObject
{
private:

    void                Init( const Graphic& rGraphic, const INetURLObject& rURL );

    virtual void        WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 5; }

public:

                        SgaObjectBmp();
                        SgaObjectBmp( const INetURLObject& rURL );
                        SgaObjectBmp( const Graphic& rGraphic, const INetURLObject& rURL, const String& rFormat );
    virtual             ~SgaObjectBmp() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_BMP; }
};

// -----------------
// - SgaObjectAnim -
// -----------------

class SgaObjectAnim : public SgaObjectBmp
{
private:

                        SgaObjectAnim( const INetURLObject& ) {};

public:

                        SgaObjectAnim();
                        SgaObjectAnim( const Graphic& rGraphic, const INetURLObject& rURL, const String& rFormatName );

    virtual            ~SgaObjectAnim() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_ANIM; }
};

// -----------------
// - SgaObjectINet -
// -----------------

class SgaObjectINet : public SgaObjectAnim
{
private:

                        SgaObjectINet( const INetURLObject& ) {};

public:

                        SgaObjectINet();
                        SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const String& rFormatName );

    virtual            ~SgaObjectINet() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_INET; }
};
#endif
