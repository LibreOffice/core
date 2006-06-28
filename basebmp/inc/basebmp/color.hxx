/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-28 16:50:19 $
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

#ifndef INCLUDED_BASEBMP_COLOR_HXX
#define INCLUDED_BASEBMP_COLOR_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <basebmp/accessoradapters.hxx>
#include <vigra/mathutil.hxx>
#include <math.h>

namespace basebmp
{

class Color
{
private:
    sal_uInt32          mnColor;

public:
    Color() : mnColor(0) {}
    explicit Color( sal_uInt32 nVal ) : mnColor(nVal) {}
    Color( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue ) :
        mnColor( ((sal_uInt32)nRed << 16) | ((sal_uInt32)nGreen << 8) | nBlue )
    {}

    void setRed( sal_uInt8 nRed ) { mnColor &= ~0x00FF0000UL; mnColor |= (sal_uInt32)nRed << 16; }
    void setGreen( sal_uInt8 nGreen ) { mnColor &= ~0x0000FF00UL; mnColor |= (sal_uInt32)nGreen << 8; }
    void setBlue( sal_uInt8 nBlue ) { mnColor &= ~0x000000FFUL; mnColor |= nBlue; }

    void setGray( sal_uInt8 nGrayVal ) { mnColor = (sal_uInt32)nGrayVal << 16 | (sal_uInt32)nGrayVal << 8 | nGrayVal; }

    sal_uInt8 getRed() const   { return 0xFF & (mnColor >> 16); }
    sal_uInt8 getGreen() const { return 0xFF & (mnColor >> 8); }
    sal_uInt8 getBlue() const  { return 0xFF & mnColor; }

    sal_uInt8 getGrayscale() const { return (sal_uInt8)((getBlue()*28UL +
                                                         getGreen()*151 +
                                                         getRed()*77) / 256); }

    sal_uInt32 toInt32() const { return mnColor; }

    Color operator&( sal_uInt32 nMask ) const { return Color(mnColor & nMask); }
    Color operator^( Color col ) const { return Color(col.getRed()^getRed(),
                                                      col.getGreen()^getGreen(),
                                                      col.getBlue()^getBlue()); }
    Color operator-( Color col ) const { return Color(vigra::abs((int)getRed()-col.getRed()),
                                                      vigra::abs((int)getGreen()-col.getGreen()),
                                                      vigra::abs((int)getBlue()-col.getBlue())); }
    Color operator+( Color col ) const { return Color(getRed()+col.getRed(),
                                                      getGreen()+col.getGreen(),
                                                      getBlue()+col.getBlue()); }
    Color operator*( Color col ) const { return Color((sal_uInt32)col.getRed()*getRed()/SAL_MAX_UINT8,
                                                      (sal_uInt32)col.getGreen()*getGreen()/SAL_MAX_UINT8,
                                                      (sal_uInt32)col.getBlue()*getBlue()/SAL_MAX_UINT8); }
    Color operator*( sal_uInt8 n ) const { return Color((sal_uInt32)n*getRed()/SAL_MAX_UINT8,
                                                        (sal_uInt32)n*getGreen()/SAL_MAX_UINT8,
                                                        (sal_uInt32)n*getBlue()/SAL_MAX_UINT8); }
    Color operator*( double n ) const { return Color((sal_uInt8)(n*getRed()+.5),
                                                     (sal_uInt8)(n*getGreen()+.5),
                                                     (sal_uInt8)(n*getBlue()+.5)); }
    bool operator==( const Color& rhs ) const { return (getRed()==rhs.getRed() &&
                                                        getGreen()==rhs.getGreen() &&
                                                        getBlue()==rhs.getBlue()); }
    bool operator!=( const Color& rhs ) const { return !(*this==rhs); }
    double magnitude() const { return sqrt(getRed()*getRed()
                                           + getGreen()*getGreen()
                                           + getBlue()*getBlue()); }
};

struct ColorBitmaskOutputMaskFunctor
{
    Color operator()( Color v1, sal_uInt8 m, Color v2 ) const
    {
#if 0 //#####
        return Color(v1.toInt32()*(sal_uInt8)(1-m) + v2.toInt32()*m);
#else //#####
        static const int nAlphaDiv = 255; // 256 would be much faster and good enough...
        int nR = v1.getRed();
        int nS = v2.getRed();
        nR = nS + (((nR - nS) * m) / nAlphaDiv);

        int nG = v1.getGreen();
        nS = v2.getGreen();
        nG = nS + (((nG - nS) * m) / nAlphaDiv);

        int nB = v1.getBlue();
        nS = v2.getBlue();
        nB = nS + (((nB - nS) * m) / nAlphaDiv);

        return Color( nR, nG, nB );
#endif //#####
    }
};

/// Specialized output mask functor for Color value type
template<> struct outputMaskFunctorSelector< Color, sal_uInt8, FastMask >
{
    typedef ColorBitmaskOutputMaskFunctor type;
};

struct ColorBlendFunctor
{
    Color operator()( sal_uInt8 alpha,
                      Color     v1,
                      Color     v2 ) const
    {
        const sal_uInt8 invAlpha(0xFF-alpha);
        return Color(((sal_uInt32)invAlpha*v1.getRed() + alpha*v2.getRed())/0xFF,
                     ((sal_uInt32)invAlpha*v1.getGreen() + alpha*v2.getGreen())/0xFF,
                     ((sal_uInt32)invAlpha*v1.getBlue() + alpha*v2.getBlue())/0xFF);
    }
};

/// Specialized metafunction to select blend functor for Color value types
template<> struct blendFunctorSelector<Color, sal_uInt8>
{
    typedef ColorBlendFunctor type;
};

} // namespace basebmp

namespace vigra
{

template<>
struct NumericTraits<basebmp::Color>
{
    typedef basebmp::Color Type;
    typedef basebmp::Color Promote;
    typedef basebmp::Color RealPromote;
    typedef std::complex<basebmp::Color> ComplexPromote;
    typedef sal_uInt8 ValueType;

    typedef VigraTrueType  isIntegral;
    typedef VigraFalseType isScalar;
    typedef VigraTrueType  isSigned;
    typedef VigraTrueType  isOrdered;
    typedef VigraFalseType isComplex;

    static Type zero() { return Type(); }
    static Type one() { return Type(0x01010101); }
    static Type nonZero() { return Type(0x01010101); }

    static Promote toPromote(const Type& v) { return v; }
    static RealPromote toRealPromote(const Type& v) { return v; }
    static Type fromPromote(const Promote& v) { return v; }
    static Type fromRealPromote(const RealPromote& v) { return v; }
};

} // namespace vigra

#endif /* INCLUDED_BASEBMP_COLOR_HXX */
