/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galobj.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:07:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_GALOBJ_HXX_
#define _SVX_GALOBJ_HXX_

#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>

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

// --------------
// - SgaObjKind -
// --------------

enum SgaObjKind
{
    SGA_OBJ_NONE = 0,   // Abstraktes Objekt
    SGA_OBJ_BMP = 1,    // Bitmap-Objekt
    SGA_OBJ_SOUND = 2,  // Sound-Objekt
    SGA_OBJ_VIDEO = 3,  // Video-Objekt
    SGA_OBJ_ANIM = 4,   // Animations-Objekt
    SGA_OBJ_SVDRAW = 5, // Svdraw-Objekt
    SGA_OBJ_INET = 6    // Grafik aus dem Internet
};

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

class SgaObject
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
    BOOL                    bIsValid;
    BOOL                    bIsThumbBmp;

    virtual void            WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void            ReadData( SvStream& rIn, UINT16& rReadVersion );

    BOOL                    CreateThumb( const Graphic& rGraphic );

public:

                            SgaObject();
    virtual                 ~SgaObject() {};

    virtual SgaObjKind      GetObjKind() const = 0;
    virtual UINT16          GetVersion() const = 0;

    virtual Bitmap          GetThumbBmp() const { return aThumbBmp; }
    const GDIMetaFile&      GetThumbMtf() const { return aThumbMtf; }
    const INetURLObject&    GetURL() const { return aURL; }
    BOOL                    IsValid() const { return bIsValid; }
    BOOL                    IsThumbBitmap() const { return bIsThumbBmp; }

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
    virtual void        ReadData( SvStream& rIn, UINT16& rReadVersion );

    virtual UINT16      GetVersion() const { return 6; }

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
private:

    BOOL                CreateThumb( const FmFormModel& rModel );

    virtual void        WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, UINT16& rReadVersion );

    virtual UINT16      GetVersion() const { return 5; }

public:

                        SgaObjectSvDraw();
                        SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL );
                        SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL );
    virtual            ~SgaObjectSvDraw() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_SVDRAW; }

public:

    static BOOL         DrawCentered( OutputDevice* pOut, const FmFormModel& rModel );
};

// ----------------
// - SgaObjectBmp -
// ----------------

class SgaObjectBmp: public SgaObject
{
private:

    void                Init( const Graphic& rGraphic, const INetURLObject& rURL );

    virtual void        WriteData( SvStream& rOut, const String& rDestDir ) const;
    virtual void        ReadData( SvStream& rIn, UINT16& rReadVersion );

    virtual UINT16      GetVersion() const { return 5; }

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

                        SgaObjectAnim( const INetURLObject& rURL ) {};

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

                        SgaObjectINet( const INetURLObject& rURL ) {};

public:

                        SgaObjectINet();
                        SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const String& rFormatName );

    virtual            ~SgaObjectINet() {};

    virtual SgaObjKind  GetObjKind() const { return SGA_OBJ_INET; }
};
#endif
