/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xexptran.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:13:14 $
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

#ifndef _XEXPTRANSFORM_HXX
#define _XEXPTRANSFORM_HXX

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequenceSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

struct ImpSdXMLExpTransObj2DBase;
struct ImpSdXMLExpTransObj3DBase;
class SvXMLUnitConverter;

namespace basegfx
{
    class B2DTuple;
    class B2DHomMatrix;
    class B3DTuple;
    class B3DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

typedef ::std::vector< ImpSdXMLExpTransObj2DBase* > ImpSdXMLExpTransObj2DBaseList;
typedef ::std::vector< ImpSdXMLExpTransObj3DBase* > ImpSdXMLExpTransObj3DBaseList;

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExTransform2D
{
    ImpSdXMLExpTransObj2DBaseList   maList;
    rtl::OUString                   msString;

    void EmptyList();

public:
    SdXMLImExTransform2D() {}
    SdXMLImExTransform2D(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);
    ~SdXMLImExTransform2D() { EmptyList(); }

    void AddRotate(double fNew);
    void AddScale(const ::basegfx::B2DTuple& rNew);
    void AddTranslate(const ::basegfx::B2DTuple& rNew);
    void AddSkewX(double fNew);
    void AddSkewY(double fNew);
    void AddMatrix(const ::basegfx::B2DHomMatrix& rNew);

    bool NeedsAction() const { return 0L != maList.size(); }
    void GetFullTransform(::basegfx::B2DHomMatrix& rFullTrans);
    const rtl::OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExTransform3D
{
    ImpSdXMLExpTransObj3DBaseList   maList;
    rtl::OUString                   msString;

    void EmptyList();

public:
    SdXMLImExTransform3D() {}
    SdXMLImExTransform3D(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);
    ~SdXMLImExTransform3D() { EmptyList(); }

    void AddRotateX(double fNew);
    void AddRotateY(double fNew);
    void AddRotateZ(double fNew);
    void AddScale(const ::basegfx::B3DTuple& rNew);
    void AddTranslate(const ::basegfx::B3DTuple& rNew);
    void AddMatrix(const ::basegfx::B3DHomMatrix& rNew);

    void AddHomogenMatrix(const com::sun::star::drawing::HomogenMatrix& xHomMat);
    bool NeedsAction() const { return 0L != maList.size(); }
    void GetFullTransform(::basegfx::B3DHomMatrix& rFullTrans);
    bool GetFullHomogenTransform(com::sun::star::drawing::HomogenMatrix& xHomMat);
    const rtl::OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExViewBox
{
    rtl::OUString               msString;
    sal_Int32                   mnX;
    sal_Int32                   mnY;
    sal_Int32                   mnW;
    sal_Int32                   mnH;

public:
    SdXMLImExViewBox(sal_Int32 nX = 0L, sal_Int32 nY = 0L, sal_Int32 nW = 1000L, sal_Int32 nH = 1000L);
    SdXMLImExViewBox(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);

    sal_Int32 GetX() const { return mnX; }
    sal_Int32 GetY() const { return mnY; }
    sal_Int32 GetWidth() const { return mnW; }
    sal_Int32 GetHeight() const { return mnH; }
    const rtl::OUString& GetExportString();
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExPointsElement
{
    rtl::OUString               msString;
    com::sun::star::drawing::PointSequenceSequence  maPoly;

public:
    SdXMLImExPointsElement(com::sun::star::drawing::PointSequence* pPoints,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        // #96328#
        const bool bClosed = true);
    SdXMLImExPointsElement(const rtl::OUString& rNew,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        const SvXMLUnitConverter& rConv);

    const rtl::OUString& GetExportString() const { return msString; }
    const com::sun::star::drawing::PointSequenceSequence& GetPointSequenceSequence() const { return maPoly; }
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExSvgDElement
{
    rtl::OUString                   msString;
    const SdXMLImExViewBox&         mrViewBox;
    bool                            mbIsClosed;
    bool                            mbIsCurve;

    sal_Int32                       mnLastX;
    sal_Int32                       mnLastY;

    com::sun::star::drawing::PointSequenceSequence      maPoly;
    com::sun::star::drawing::FlagSequenceSequence       maFlag;

public:
    SdXMLImExSvgDElement(const SdXMLImExViewBox& rViewBox);
    SdXMLImExSvgDElement(const rtl::OUString& rNew,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        const SvXMLUnitConverter& rConv);

    void AddPolygon(
        com::sun::star::drawing::PointSequence* pPoints,
        com::sun::star::drawing::FlagSequence* pFlags,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        bool bClosed = false, bool bRelative = true);

    const rtl::OUString& GetExportString() const { return msString; }
    bool IsClosed() const { return mbIsClosed; }
    bool IsCurve() const { return mbIsCurve; }
    const com::sun::star::drawing::PointSequenceSequence& GetPointSequenceSequence() const { return maPoly; }
    const com::sun::star::drawing::FlagSequenceSequence& GetFlagSequenceSequence() const { return maFlag; }
};


#endif  //  _XEXPTRANSFORM_HXX
