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

#ifndef _SVX_GALOBJ_HXX_
#define _SVX_GALOBJ_HXX_

#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>
#include "svx/galmisc.hxx"

#define S_THUMB 80


#define SGA_FORMAT_NONE                 0x00000000L
#define SGA_FORMAT_STRING               0x00000001L
#define SGA_FORMAT_GRAPHIC              0x00000010L
#define SGA_FORMAT_SOUND                0x00000100L
#define SGA_FORMAT_OLE                  0x00001000L
#define SGA_FORMAT_SVDRAW               0x00010000L
#define SGA_FORMAT_ALL                  0xFFFFFFFFL

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

class SVX_DLLPUBLIC SgaObject
{
    friend class GalleryTheme;

private:

    void                    ImplUpdateURL( const INetURLObject& rNewURL ) { aURL = rNewURL; }

protected:

    BitmapEx                aThumbBmp; // Allow transparence to survive
    GDIMetaFile             aThumbMtf;
    INetURLObject           aURL;
    OUString                aUserName;
    OUString                aTitle;
    sal_Bool                    bIsValid;
    sal_Bool                    bIsThumbBmp;

    virtual void            WriteData( SvStream& rOut, const OUString& rDestDir ) const;
    virtual void            ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    sal_Bool                    CreateThumb( const Graphic& rGraphic );

public:
                            SgaObject();
    virtual                 ~SgaObject() {};

    virtual SgaObjKind      GetObjKind() const = 0;
    virtual sal_uInt16      GetVersion() const = 0;

    virtual BitmapEx        GetThumbBmp() const { return aThumbBmp; }
    const GDIMetaFile&      GetThumbMtf() const { return aThumbMtf; }
    const INetURLObject&    GetURL() const { return aURL; }
    sal_Bool                IsValid() const { return bIsValid; }
    sal_Bool                IsThumbBitmap() const { return bIsThumbBmp; }

    const OUString          GetTitle() const;
    void                    SetTitle( const OUString& rTitle );

    friend SvStream&        operator<<( SvStream& rOut, const SgaObject& rObj );
    friend SvStream&        operator>>( SvStream& rIn, SgaObject& rObj );

    BitmapEx createPreviewBitmapEx(const Size& rSizePixel) const;
};

class SgaObjectSound : public SgaObject
{
private:

    GalSoundType        eSoundType;

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 6; }

public:

                        SgaObjectSound();
                        SgaObjectSound( const INetURLObject& rURL );
    virtual            ~SgaObjectSound();

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_SOUND; }
    virtual BitmapEx    GetThumbBmp() const;
    GalSoundType        GetSoundType() const { return eSoundType; }
};

class FmFormModel;

class SgaObjectSvDraw : public SgaObject
{
    using SgaObject::CreateThumb;

private:

    sal_Bool                CreateThumb( const FmFormModel& rModel );

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 5; }

public:

                        SgaObjectSvDraw();
                        SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL );
                        SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL );
    virtual            ~SgaObjectSvDraw() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_SVDRAW; }
};

class SgaObjectBmp: public SgaObject
{
private:

    void                Init( const Graphic& rGraphic, const INetURLObject& rURL );

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    virtual sal_uInt16      GetVersion() const { return 5; }

public:

                        SgaObjectBmp();
                        SgaObjectBmp( const INetURLObject& rURL );
                        SgaObjectBmp( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormat );
    virtual             ~SgaObjectBmp() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_BMP; }
};

class SgaObjectAnim : public SgaObjectBmp
{
private:

                        SgaObjectAnim( const INetURLObject& ) {};

public:

                        SgaObjectAnim();
                        SgaObjectAnim( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormatName );

    virtual            ~SgaObjectAnim() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_ANIM; }
};

class SgaObjectINet : public SgaObjectAnim
{
private:

                        SgaObjectINet( const INetURLObject& ) {};

public:

                        SgaObjectINet();
                        SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormatName );

    virtual            ~SgaObjectINet() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_INET; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
