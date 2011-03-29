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

#ifndef _SV_SALFONT_H
#define _SV_SALFONT_H

// -=-= exports =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalFontCache;
struct  SalFontDimension;
class   SalFontFamily;
class   SalFontFamilyList;
class   SalFontStruct;
class   SalFontStructList;
class   SalFonts;

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <salstd.hxx>
#include <vcl/outfont.hxx>

// -=-= forwards =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef ULONG XFP_FLAGS;

class   SalDisplay;
class   SalFontCacheItem;

// -=-= SalFontCache -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
DECLARE_LIST( SalFontCache, SalFontCacheItem* )

// -=-= SalFontDimension -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct SalFontDimension
{
            USHORT      nHeight_;               // [pixel]
            USHORT      nPtHeight_;             // [point/10]
            USHORT      nAverage_;              // [pixel/10]
            USHORT      nXRes_;                 // [dpi]
            USHORT      nYRes_;                 // [dpi]
            USHORT      nSlant_;                // [pixel]
//          size_t      nUnderlineThickness_;   // [pixel]
//          size_t      nUnderlinePosition_;    // [pixel]
//          size_t      nStrikeoutAscent_;      // [pixel]
//          size_t      nStrikeoutDescent_;     // [pixel]
//          Subscript, Superscript, Capital, Space ...

    inline              SalFontDimension( USHORT nA = 0, USHORT nH = 0 );

    inline  BOOL        IsScalable() const;
    inline  USHORT      GetWidth() const { return (nAverage_ + 5) / 10; }
    inline  Size        GetSize() const;
    inline  void        SetSize( const Size & rSize );
    inline  BOOL        operator == ( const SalFontDimension &r ) const;
    inline  BOOL        operator != ( const SalFontDimension &r ) const;
    inline  BOOL        operator >= ( const SalFontDimension &r ) const;
};

inline SalFontDimension::SalFontDimension( USHORT nA, USHORT nH )
    : nHeight_( nH ), nAverage_( nA )
{ nPtHeight_ = nXRes_ = nYRes_ = nSlant_ = 0; }

inline BOOL SalFontDimension::IsScalable() const
{ return !nHeight_ && !nPtHeight_ && !nAverage_; }

inline Size SalFontDimension::GetSize() const
{ return Size( (nAverage_ + 5) / 10, nHeight_ ); }

inline void SalFontDimension::SetSize( const Size & rSize )
{ nAverage_ = (USHORT)rSize.Width() * 10; nHeight_ = (USHORT)rSize.Height(); }

inline BOOL SalFontDimension::operator == ( const SalFontDimension &r ) const
{ return nHeight_ == r.nHeight_ && (!r.nAverage_ || nAverage_ == r.nAverage_); }

inline BOOL SalFontDimension::operator != ( const SalFontDimension &r ) const
{ return !(*this == r); }

inline BOOL SalFontDimension::operator >= ( const SalFontDimension &r ) const
{ return nHeight_ > r.nHeight_
         || (nHeight_ == r.nHeight_ && nAverage_ >= r.nAverage_); }

// -=-= SalFontStruct =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalFontStruct : public ImplFontMetricData
{
    friend class SalDisplay;
    friend class SalGraphicsData;

            SalFontCacheItem*pCache_;

#if (OSL_DEBUG_LEVEL > 1) || defined  DBG_UTIL
            ByteString      aFontName_;
#endif
            USHORT          nHeightCount_;      // Anzahl der Hoehen-Eintraege
            SalFontDimension*pDimensions_;      // Hoehen-Array
            USHORT          nWeight_;

            USHORT          nFoundry_;          // properties indexies
            USHORT          nFamily_;
            USHORT          nWeightName_;
            USHORT          nSlant_;
            USHORT          nSetWidthName_;
            ByteString      aAddStyleName_;
            USHORT          nSpacing_;
            USHORT          nCharSet_;
            USHORT          nFaceName_;
            BOOL            mbValidFontDescription; // valid xlfd entries

            void            Init();
            BOOL            Init( SalDisplay*       pDisp,
                                  const char*       pFontName,
                                  SalFontDimension& rDim );

            ByteString      GetXFontName( const SalFontDimension& );

    inline  void            SetFoundry( USHORT n )
        { nFoundry_ = n; }
    inline  void            SetFamily( USHORT n )
        { meFamily = sal_FamilyToSal( nFamily_ = n ); }
    inline  void            SetWeightName( USHORT n )
        { meWeight = sal_WeightToSal( nWeightName_ = n ); }
    inline  void            SetSlant( USHORT n )
        { meItalic = sal_ItalicToSal( nSlant_ = n ); }
    inline  void            SetSetWidthName( USHORT n )
        { nSetWidthName_ = n; }
    inline  void            SetAddStyleName( const ByteString& rAddStyle )
        { aAddStyleName_ = rAddStyle; aAddStyleName_.ToLowerAscii(); }
    inline  void            SetSpacing( USHORT n )
        { mePitch = sal_PitchToSal( nSpacing_ = n ); }
    inline  void            SetAverage( long n )
        { mnWidth = (n + 5) / 10; }
    void                    SetCharSet( USHORT n );

                            SalFontStruct( const SalFontStruct& rFont );
public:
                            SalFontStruct( SalDisplay*          pDisp,
                                           const char*          pFontName,
                                           SalFontDimension&    rDim );

                            ~SalFontStruct();

    inline  void            Cache( SalFontCacheItem *p ) { pCache_ = p; }
    inline  SalFontCacheItem*IsCache() const { return pCache_; }
    inline  BOOL            IsScalable() const { return TYPE_SCALABLE==meType; }
    inline  SalFontDimension*GetDim() const { return pDimensions_; }
    inline  BOOL            IsValid() const { return mbValidFontDescription; }
#ifdef DBG_UTIL
            const ByteString&   GetName() const { return aFontName_; }
#endif

            ImplFontData   *GetDevFontData();
            SalFontCacheItem*Load( SalDisplay *pDisp, const SalFontDimension &rDim );
            CharSet GetCharSet() { return meCharSet; }

};

// -=-= SalFontStructList =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
DECLARE_LIST( SalFontStructList, SalFontStruct* )

#endif // _SV_SALFONT_H

