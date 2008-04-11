/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlfd_extd.hxx,v $
 * $Revision: 1.17 $
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
#ifndef XLFD_EXTENDED_HXX
#define XLFD_EXTENDED_HXX

#include <salunx.h>
#ifndef _VCL_VCLENUM_HXX
#include <vcl/enum.hxx>
#endif
#ifndef _VCL_OUTFONT_HXX
#include <vcl/outfont.hxx>
#endif

#include <vector>

class Xlfd;
class AttributeProvider;
class ImplDevFontList;
class ByteString;

// --------------------------------------------------------------------------
//
//  classes for Xlfd handling that contain more than a single encoding.
//  Members that may vary through different encodings are stored in
//  a mpEncodingInfo member. There are three different classes:
//  true scalable fonts (truetype and type1) scalable bitmap fonts
//  (the ugly ones) and bitmap fonts. The ExtendedXlfd stores all the members
//  that are specific to a font outline
//  ( e.g. adobe-times-roman-medium-r-normal- * -p- * )
//  and specifies the interface.
//
// --------------------------------------------------------------------------

// base class

class ExtendedXlfd : public ImplDevFontAttributes
{
    public:
                             ExtendedXlfd( bool bScalable );
        virtual             ~ExtendedXlfd();
        virtual bool        AddEncoding( const Xlfd* );
        bool                HasEncoding( rtl_TextEncoding ) const;
        int                 GetEncodingIdx( rtl_TextEncoding nEncoding ) const;
        unsigned short      NumEncodings() const
                                    { return mnEncodings; }
        virtual int             GetPixelSize() const
                                    { return 0; }
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                       rtl_TextEncoding nEncoding ) const ;
        virtual void        ToString( ByteString &rString,
                                      unsigned short nPixelSize,
                                     char* pMatricsString,
                                       rtl_TextEncoding nEncoding  ) const;

        virtual ImplFontData* GetImplFontData() const = 0;
        bool                  IsScalable() const { return mbScalable; }
        virtual FontFamily    GetFamilyType() const;
        virtual FontWeight    GetWeight() const;
        virtual FontItalic    GetSlant() const;
        virtual FontWidth     GetWidthType() const;
        virtual FontPitch     GetPitch() const;
        virtual FontPitch     GetPitch( rtl_TextEncoding ) const;
        rtl_TextEncoding      GetAsciiEncoding( int *pAsciiRange = NULL ) const;
        rtl_TextEncoding      GetEncoding() const;
        rtl_TextEncoding      GetEncoding( int i ) const;

        int                 GetFontCodeRanges( sal_uInt32* pCodePairs ) const;

    protected:
        AttributeProvider*  mpFactory;

    public:
        unsigned short      mnFoundry;
        unsigned short      mnFamily;
        unsigned short      mnWeight;
        unsigned short      mnSlant;
        unsigned short      mnSetwidth;
        bool                mbScalable;

    protected:
        unsigned short      mnEncodings;
        unsigned short      mnEncCapacity;
        struct EncodingInfo {
            unsigned char       mcSpacing;
            unsigned short      mnResolutionX;
            unsigned short      mnResolutionY;
            unsigned short      mnAddstyle;
            unsigned short      mnCharset;
            rtl_TextEncoding    mnEncoding;

            EncodingInfo&       operator= ( const Xlfd *pXlfd );
        } *mpEncodingInfo;
};

// class to handle scalable bitmap fonts

class ScalableBitmapXlfd : public ExtendedXlfd {

    public:
                            ScalableBitmapXlfd();
        virtual             ~ScalableBitmapXlfd();
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    rtl_TextEncoding nEncoding ) const;
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    char* pMatricsString,
                                    rtl_TextEncoding nEncoding ) const;

        virtual ImplFontData* GetImplFontData() const ;
};

// class to handle true bitmap fonts

class ScalableXlfd;

class BitmapXlfd : public ExtendedXlfd {

    public:
                            BitmapXlfd();
                            ~BitmapXlfd();
        bool                AddEncoding( const Xlfd* );
        virtual int     GetPixelSize() const
                                    { return mnPixelSize; }
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    rtl_TextEncoding nEncoding ) const;
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    char* pMatricsString,
                                    rtl_TextEncoding nEncoding ) const;
        virtual ImplFontData* GetImplFontData() const ;
    protected:

        unsigned short      mnPixelSize;
        unsigned short      mnPointSize;
        unsigned short      mnAverageWidth;
};

// class to handle true scalable fonts

class ScalableXlfd : public ExtendedXlfd {

    friend class BitmapXlfd;

    public:
                            ScalableXlfd();
        virtual             ~ScalableXlfd();
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    rtl_TextEncoding nEncoding ) const;

        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    char* pMatricsString,
                                    rtl_TextEncoding  nEncoding ) const;
        virtual ImplFontData* GetImplFontData() const ;
};

// class to maintain a list of fonts ( bitmap and scalable )

class XlfdStorage {

    public:
                            XlfdStorage();

        void                Dispose();
        void                Reset();

        void                Add( const ExtendedXlfd *pXlfd );
        void                Add( const XlfdStorage *pXlfd );
        void                AnnounceFonts( ImplDevFontList* ) const;

    protected:

    typedef ::std::vector<const ExtendedXlfd*> XlfdList;
    XlfdList                maXlfdList;
};

// list of fonts specific for bitmap fonts

class BitmapXlfdStorage : public XlfdStorage {

    public:

        void                AddBitmapFont( const Xlfd *pXlfd );
};


/* Virtual font for User Interface */

class VirtualXlfd : public ExtendedXlfd
{
    private:

        int                 GetFontQuality (unsigned short nFamily);

    public:
                             VirtualXlfd();
        virtual             ~VirtualXlfd();
        virtual bool        AddEncoding( const Xlfd* );
        void                FilterInterfaceFont (const Xlfd *pXlfd);
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                       rtl_TextEncoding nEncoding ) const ;
        virtual void        ToString( ByteString &rString,
                                      unsigned short nPixelSize,
                                     char* pMatricsString,
                                       rtl_TextEncoding nEncoding  ) const;

        virtual ImplFontData* GetImplFontData() const ;
    protected:

        unsigned short      mnExtCapacity;
        struct ExtEncodingInfo {
            unsigned short      mnFoundry;
            unsigned short      mnFamily;
            unsigned short      mnWeight;
            unsigned short      mnSlant;
            unsigned short      mnSetwidth;

            ExtEncodingInfo&    operator= ( const Xlfd *pXlfd );
        } *mpExtEncodingInfo;

    friend class ExtEncodingInfo;
};


// class to describe a X11 physically available font face

class ImplX11FontData : public ImplFontData
{
private:
    enum { X11IFD_MAGIC = 0x111FDA1C };
    const ExtendedXlfd&   mrXlfd;

public:
                            ImplX11FontData( const ExtendedXlfd&, int nHeight );
    const ExtendedXlfd&     GetExtendedXlfd() const { return mrXlfd; }
    virtual ImplFontData*   Clone() const           { return new ImplX11FontData( *this ); }
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
    virtual sal_IntPtr      GetFontId() const;

    static bool             CheckFontData( const ImplFontData& r ) { return r.CheckMagic( X11IFD_MAGIC ); }
};

#endif /* XLFD_EXTENDED_HXX */
