/*************************************************************************
 *
 *  $RCSfile: xlfd_extd.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:39:50 $
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
#ifndef XLFD_EXTENDED_HXX
#define XLFD_EXTENDED_HXX

#ifndef _SALUNX_H
#include <salunx.h>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <enum.hxx>
#endif
#ifndef _VCL_OUTFONT_HXX
#include <outfont.hxx>
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

    static bool             CheckFontData( const ImplFontData& r ) { return r.CheckMagic( X11IFD_MAGIC ); }
};

#endif /* XLFD_EXTENDED_HXX */
