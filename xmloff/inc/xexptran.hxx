/*************************************************************************
 *
 *  $RCSfile: xexptran.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-24 16:46:35 $
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

#ifndef _XEXPTRANSFORM_HXX
#define _XEXPTRANSFORM_HXX

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
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

//////////////////////////////////////////////////////////////////////////////

struct ImpSdXMLExpTransObj2DBase;
struct ImpSdXMLExpTransObj3DBase;
class SvXMLUnitConverter;
class Vector3D;
class Matrix3D;
class Matrix4D;

//////////////////////////////////////////////////////////////////////////////

DECLARE_LIST(ImpSdXMLExpTransObj2DBaseList, ImpSdXMLExpTransObj2DBase*);
DECLARE_LIST(ImpSdXMLExpTransObj3DBaseList, ImpSdXMLExpTransObj3DBase*);

//////////////////////////////////////////////////////////////////////////////

class SdXMLImExTransform2D
{
    ImpSdXMLExpTransObj2DBaseList   maList;
    rtl::OUString                   msString;

    void EmptyList();
    ImpSdXMLExpTransObj2DBase* FindObject(sal_uInt16 nType, sal_uInt32 nInd = 0L);

public:
    SdXMLImExTransform2D() {}
    SdXMLImExTransform2D(const rtl::OUString& rNew, const SvXMLUnitConverter& rConv);
    ~SdXMLImExTransform2D() { EmptyList(); }

    void AddRotate(double fNew);
    void AddScale(const Vector2D& rNew);
    void AddTranslate(const Vector2D& rNew);
    void AddSkewX(double fNew);
    void AddSkewY(double fNew);
    void AddMatrix(const Matrix3D& rNew);

    sal_Bool FindRotate(double& rNew, sal_uInt32 nInd = 0L);

    sal_Bool NeedsAction() const { return (sal_Bool)(maList.Count() > 0L); }

    const rtl::OUString& GetExportString(const SvXMLUnitConverter& rConv);
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
    void AddScale(const Vector3D& rNew);
    void AddTranslate(const Vector3D& rNew);
    void AddMatrix(const Matrix3D& rNew);

    void AddHomogenMatrix(const com::sun::star::drawing::HomogenMatrix& xHomMat);

    sal_Bool NeedsAction() const { return (sal_Bool)(maList.Count() > 0L); }
    void GetFullTransform(Matrix4D& rFullTrans);
    BOOL GetFullHomogenTransform(com::sun::star::drawing::HomogenMatrix& xHomMat);
    const rtl::OUString& GetExportString(const SvXMLUnitConverter& rConv);
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
    const rtl::OUString& GetExportString(const SvXMLUnitConverter& rConv);
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
        const SvXMLUnitConverter& rConv);
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
    sal_Bool                        mbIsClosed;
    sal_Bool                        mbIsCurve;

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
        const SvXMLUnitConverter& rConv,
        sal_Bool bClosed = FALSE, sal_Bool bRelative = TRUE);

    const rtl::OUString& GetExportString() const { return msString; }
    sal_Bool IsClosed() const { return mbIsClosed; }
    sal_Bool IsCurve() const { return mbIsCurve; }
    const com::sun::star::drawing::PointSequenceSequence& GetPointSequenceSequence() const { return maPoly; }
    const com::sun::star::drawing::FlagSequenceSequence& GetFlagSequenceSequence() const { return maFlag; }
};


#endif  //  _XEXPTRANSFORM_HXX
