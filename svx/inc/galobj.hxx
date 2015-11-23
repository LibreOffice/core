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

#ifndef INCLUDED_SVX_INC_GALOBJ_HXX
#define INCLUDED_SVX_INC_GALOBJ_HXX

#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>
#include "svx/galmisc.hxx"

#define S_THUMB 80



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
    OUString                aTitle;
    bool                    bIsValid;
    bool                    bIsThumbBmp;

    virtual void            WriteData( SvStream& rOut, const OUString& rDestDir ) const;
    virtual void            ReadData( SvStream& rIn, sal_uInt16& rReadVersion );

    bool                    CreateThumb( const Graphic& rGraphic );

public:
                            SgaObject();
    virtual                 ~SgaObject() {};

    virtual SgaObjKind      GetObjKind() const = 0;
    virtual sal_uInt16      GetVersion() const = 0;

    virtual BitmapEx        GetThumbBmp() const { return aThumbBmp; }
    const GDIMetaFile&      GetThumbMtf() const { return aThumbMtf; }
    const INetURLObject&    GetURL() const { return aURL; }
    bool                IsValid() const { return bIsValid; }
    bool                IsThumbBitmap() const { return bIsThumbBmp; }

    const OUString          GetTitle() const;
    void                    SetTitle( const OUString& rTitle );

    friend SvStream&        WriteSgaObject( SvStream& rOut, const SgaObject& rObj );
    friend SvStream&        ReadSgaObject( SvStream& rIn, SgaObject& rObj );

    BitmapEx createPreviewBitmapEx(const Size& rSizePixel) const;
};

class SgaObjectSound : public SgaObject
{
private:

    GalSoundType        eSoundType;

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const override;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion ) override;

    virtual sal_uInt16      GetVersion() const override { return 6; }

public:

                        SgaObjectSound();
                        SgaObjectSound( const INetURLObject& rURL );
    virtual            ~SgaObjectSound();

    virtual SgaObjKind  GetObjKind() const override { return SGA_OBJ_SOUND; }
    virtual BitmapEx    GetThumbBmp() const override;
};

class FmFormModel;

class SgaObjectSvDraw : public SgaObject
{
    using SgaObject::CreateThumb;

private:

    bool                CreateThumb( const FmFormModel& rModel );

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const override;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion ) override;

    virtual sal_uInt16      GetVersion() const override { return 5; }

public:

                        SgaObjectSvDraw();
                        SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL );
                        SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL );
    virtual            ~SgaObjectSvDraw() {};

    virtual SgaObjKind  GetObjKind() const override { return SGA_OBJ_SVDRAW; }
};

class SgaObjectBmp: public SgaObject
{
private:

    void                Init( const Graphic& rGraphic, const INetURLObject& rURL );

    virtual void        WriteData( SvStream& rOut, const OUString& rDestDir ) const override;
    virtual void        ReadData( SvStream& rIn, sal_uInt16& rReadVersion ) override;

    virtual sal_uInt16      GetVersion() const override { return 5; }

public:

                        SgaObjectBmp();
                        SgaObjectBmp( const INetURLObject& rURL );
                        SgaObjectBmp( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormat );
    virtual             ~SgaObjectBmp() {};

    virtual SgaObjKind  GetObjKind() const override { return SGA_OBJ_BMP; }
};

class SgaObjectAnim : public SgaObjectBmp
{
private:

                        SgaObjectAnim( const INetURLObject& ) {};

public:

                        SgaObjectAnim();
                        SgaObjectAnim( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormatName );

    virtual            ~SgaObjectAnim() {};

    virtual SgaObjKind  GetObjKind() const override { return SGA_OBJ_ANIM; }
};

class SgaObjectINet : public SgaObjectAnim
{
private:

                        SgaObjectINet( const INetURLObject& ) {};

public:

                        SgaObjectINet();
                        SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormatName );

    virtual            ~SgaObjectINet() {};

    virtual SgaObjKind  GetObjKind() const override { return SGA_OBJ_INET; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
