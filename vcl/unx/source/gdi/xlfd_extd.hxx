/*************************************************************************
 *
 *  $RCSfile: xlfd_extd.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-23 16:24:12 $
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

class Xlfd;
class AttributeProvider;
class ImplFontData;
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

class VirtualXlfd;
class XlfdStorage;

class ExtendedXlfd {

    friend VirtualXlfd;
    friend XlfdStorage;

    public:
                             ExtendedXlfd();
        virtual             ~ExtendedXlfd();
        virtual Bool        AddEncoding( const Xlfd *pXlfd );
        Bool                HasEncoding( rtl_TextEncoding nEncoding ) const;
        int                 GetEncodingIdx( rtl_TextEncoding nEncoding ) const;
        unsigned short      NumEncodings() const
                                    { return mnEncodings; }
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                       rtl_TextEncoding nEncoding ) const ;
        virtual void        ToString( ByteString &rString,
                                      unsigned short nPixelSize,
                                     char* pMatricsString,
                                       rtl_TextEncoding nEncoding  ) const;

        virtual void        ToImplFontData( ImplFontData *pFontData ) const ;
        virtual FontType    GetFontType() const
                                    { return TYPE_DONTKNOW; }
        FontFamily          GetFamily() const;
        FontWeight          GetWeight() const;
        FontItalic          GetItalic() const;
        FontWidth           GetWidth() const;
        virtual FontPitch   GetSpacing() const;
        virtual FontPitch   GetSpacing( rtl_TextEncoding nEnc ) const;
        rtl_TextEncoding    GetAsciiEncoding( int *pAsciiRange = NULL ) const;
        rtl_TextEncoding    GetEncoding() const;
        rtl_TextEncoding    GetEncoding( int i ) const;

        #ifdef DEBUG
        void                Dump() const;
        #endif

    protected:

        AttributeProvider*  mpFactory;

    #ifdef GCC
    public:
    #endif

        unsigned short      mnFoundry;
        unsigned short      mnFamily;
        unsigned short      mnWeight;
        unsigned short      mnSlant;
        unsigned short      mnSetwidth;

    #ifdef GCC
    protected:
    #endif

        unsigned short      mnEncodings;
        struct EncodingInfo {
            unsigned char       mcSpacing;
            unsigned short      mnResolutionX;
            unsigned short      mnResolutionY;
            unsigned short      mnAddstyle;
            unsigned short      mnCharset;

            rtl_TextEncoding    mnEncoding;

            EncodingInfo&       operator= ( const Xlfd *pXlfd );
            EncodingInfo&       operator= ( const EncodingInfo& rInfo );
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

        virtual void        ToImplFontData( ImplFontData *pFontData ) const;
        virtual FontType    GetFontType() const
                                    { return TYPE_SCALABLE; }
};

// class to handle true bitmap fonts

class ScalableXlfd;

class BitmapXlfd : public ExtendedXlfd {

    public:
                            BitmapXlfd();
                            ~BitmapXlfd();
        Bool                AddEncoding( const Xlfd *pXlfd );
        Bool                AddEncoding( const ScalableXlfd *pXlfd );
        unsigned short      GetPixelSize() const
                                    { return mnPixelSize; }
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    rtl_TextEncoding nEncoding ) const;
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                    char* pMatricsString,
                                    rtl_TextEncoding nEncoding ) const;
        virtual void        ToImplFontData( ImplFontData *pFontData ) const ;
        virtual FontType    GetFontType() const
                                    { return TYPE_RASTER; }
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
        virtual void        ToImplFontData( ImplFontData *pFontData ) const ;
        virtual FontType    GetFontType() const
                                    { return TYPE_SCALABLE; }
};

// class to maintain a list of fonts ( bitmap and scalable )

class XlfdStorage {

    public:
                            XlfdStorage();
                            ~XlfdStorage();

        void                Dispose();
        void                Reset();

        void                Add( const ExtendedXlfd *pXlfd );
        void                Add( const XlfdStorage *pXlfd );
        unsigned short      GetCount() const
                                    { return mnCount; }
        const ExtendedXlfd* Get(int nIdx) const;
        #ifdef DEBUG
        void                Dump() const ;
        #endif

    protected:

        void                Enlarge();

        unsigned short      mnCount;
        unsigned short      mnSize;
        const ExtendedXlfd**
                            mpList;
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
        virtual Bool        AddEncoding( const Xlfd *pXlfd );
        void                FilterInterfaceFont (const Xlfd *pXlfd);
        virtual void        ToString( ByteString &rString,
                                    unsigned short nPixelSize,
                                       rtl_TextEncoding nEncoding ) const ;
        virtual void        ToString( ByteString &rString,
                                      unsigned short nPixelSize,
                                     char* pMatricsString,
                                       rtl_TextEncoding nEncoding  ) const;

        virtual void        ToImplFontData( ImplFontData *pFontData ) const ;
        virtual FontType    GetFontType() const
                                    { return TYPE_SCALABLE; }
    protected:

        struct ExtEncodingInfo {

            unsigned short      mnFoundry;
            unsigned short      mnFamily;
            unsigned short      mnWeight;
            unsigned short      mnSlant;
            unsigned short      mnSetwidth;

            ExtEncodingInfo&    operator= ( const Xlfd *pXlfd );
            ExtEncodingInfo&    operator= ( const ExtEncodingInfo& rInfo );
        } *mpExtEncodingInfo;

    friend class ExtEncodingInfo;
};

#endif /* XLFD_EXTENDED_HXX */

