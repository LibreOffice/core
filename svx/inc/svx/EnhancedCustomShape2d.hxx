/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _ENHANCEDCUSTOMSHAPE2D_HXX
#define _ENHANCEDCUSTOMSHAPE2D_HXX

#include <svx/msdffdef.hxx>
#include <svx/sdasitm.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <svx/EnhancedCustomShapeFunctionParser.hxx>

#include <boost/shared_ptr.hpp>
#include <vector>

#define DFF_CUSTOMSHAPE_FLIP_V      1
#define DFF_CUSTOMSHAPE_FLIP_H      2
#define DFF_CUSTOMSHAPE_EXCH            4

class SdrModel;
class SdrObject;
class SfxItemSet;
struct SvxMSDffVertPair;
struct SvxMSDffCalculationData;
struct SvxMSDffTextRectangles;

#define HANDLE_FLAGS_MIRRORED_X             0x0001
#define HANDLE_FLAGS_MIRRORED_Y             0x0002
#define HANDLE_FLAGS_SWITCHED               0x0004
#define HANDLE_FLAGS_POLAR                  0x0008
#define HANDLE_FLAGS_MAP                    0x0010
#define HANDLE_FLAGS_RANGE_X_MINIMUM        0x0020
#define HANDLE_FLAGS_RANGE_X_MAXIMUM        0x0040
#define HANDLE_FLAGS_RANGE_Y_MINIMUM        0x0080
#define HANDLE_FLAGS_RANGE_Y_MAXIMUM        0x0100
#define HANDLE_FLAGS_RADIUS_RANGE_MINIMUM   0x0200
#define HANDLE_FLAGS_RADIUS_RANGE_MAXIMUM   0x0400
#define HANDLE_FLAGS_REFX                   0x0800
#define HANDLE_FLAGS_REFY                   0x1000
#define HANDLE_FLAGS_REFANGLE               0x2000
#define HANDLE_FLAGS_REFR                   0x4000

// MSDFF_HANDLE_FLAGS_RANGE_Y seems to be not defined in
// escher, but we are using it internally in to differentiate
// between X_RANGE and Y_RANGE

class SdrPathObj;

class EnhancedCustomShape2d : public SfxItemSet
{
        SdrObject*                  pCustomShapeObj;
        MSO_SPT                     eSpType;

        sal_Int32                   nCoordLeft;
        sal_Int32                   nCoordTop;
        sal_Int32                   nCoordWidthG;
        sal_Int32                   nCoordHeightG;
        sal_Int32                   nCoordWidth;
        sal_Int32                   nCoordHeight;
        Rectangle                   aLogicRect;

        double                      fXScale;
        double                      fYScale;
        double                      fXRatio;
        double                      fYRatio;

        sal_Bool                    bOOXMLShape;

        sal_Int32                   nXRef;
        sal_Int32                   nYRef;
        sal_uInt32                  nFlags;
        sal_uInt32                  nColorData;

        /*

        */
        struct EquationResult {
            sal_Bool bReady;
            double fValue;
        };
        com::sun::star::uno::Sequence< rtl::OUString >                                                  seqEquations;
        std::vector< ::boost::shared_ptr< EnhancedCustomShape::ExpressionNode > >                       vNodesSharedPtr;
        std::vector< EquationResult >                                                                   vEquationResults;

        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment >            seqSegments;
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair>       seqCoordinates;
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame >          seqTextFrames;
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair>       seqGluePoints;
        com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue >    seqAdjustmentValues;
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues >                          seqHandles;
        com::sun::star::uno::Sequence< com::sun::star::awt::Size >                                      seqSubViewSize;

        sal_Bool                    bTextFlow       : 1;
        sal_Bool                    bFilled         : 1;
        sal_Bool                    bStroked        : 1;

        sal_Bool                    bFlipH;
        sal_Bool                    bFlipV;
        sal_Int32                   nRotateAngle;

        sal_Bool                    SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex );
        sal_Int32                   GetLuminanceChange( sal_uInt32 nIndex ) const;
        Color                       GetColorData( const Color& rFillColor, sal_uInt32 nIndex, double dBrightness ) const;
        void                        AdaptObjColor(SdrPathObj& rObj, const SfxItemSet& rCustomShapeSet,
                                                  sal_uInt32& nColorIndex, sal_uInt32 nColorCount);
        sal_Bool                    GetParameter( double& rParameterReturnValue,  const com::sun::star::drawing::EnhancedCustomShapeParameter&,
                                                  const sal_Bool bReplaceGeoWidth, const sal_Bool bReplaceGeoHeight ) const;
        Point                       GetPoint( const com::sun::star::drawing::EnhancedCustomShapeParameterPair&,
                                                    const sal_Bool bScale = sal_True, const sal_Bool bReplaceGeoSize = sal_False ) const;

        void                        CreateSubPath( sal_uInt16& rSrcPt, sal_uInt16& rSegmentInd, std::vector< SdrPathObj* >& rObjectList,
                                                   sal_Bool bLineGeometryNeededOnly, sal_Bool bSortFilledObjectsToBack,
                                                   sal_Int32 nIndex );
        SdrObject*                  CreatePathObj( sal_Bool bLineGeometryNeededOnly );
        const sal_Int32*            ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rItem );

        void                        SetPathSize( sal_Int32 nIndex = 0 );

    public :

        struct Handle
        {
            sal_uInt32  nFlags;

            sal_Bool    bMirroredX;
            sal_Bool    bMirroredY;
            sal_Bool    bSwitched;

            com::sun::star::drawing::EnhancedCustomShapeParameterPair   aPosition;
            com::sun::star::drawing::EnhancedCustomShapeParameterPair   aPolar;

            sal_Int32   nRefX;
            sal_Int32   nRefY;
            sal_Int32   nRefAngle;
            sal_Int32   nRefR;

            com::sun::star::drawing::EnhancedCustomShapeParameter       aRadiusRangeMinimum;
            com::sun::star::drawing::EnhancedCustomShapeParameter       aRadiusRangeMaximum;
            com::sun::star::drawing::EnhancedCustomShapeParameter       aXRangeMinimum;
            com::sun::star::drawing::EnhancedCustomShapeParameter       aXRangeMaximum;
            com::sun::star::drawing::EnhancedCustomShapeParameter       aYRangeMinimum;
            com::sun::star::drawing::EnhancedCustomShapeParameter       aYRangeMaximum;

            Handle() : bMirroredX ( sal_False ), bMirroredY ( sal_False ), bSwitched( sal_False ),
                nRefX( -1 ), nRefY( -1 ), nRefAngle( -1 ), nRefR( -1 ) {};
        };

        sal_Bool                    IsFlipVert() { return bFlipV; };
        sal_Bool                    IsFlipHorz() { return bFlipH; };
        sal_Int32                   GetRotateAngle() { return nRotateAngle; };
        SVX_DLLPUBLIC bool          IsPostRotate() const;

        SVX_DLLPUBLIC SdrObject*                    CreateLineGeometry();
        SVX_DLLPUBLIC SdrObject*                    CreateObject( sal_Bool bLineGeometryNeededOnly );
        SVX_DLLPUBLIC void                      ApplyGluePoints( SdrObject* pObj );
        SVX_DLLPUBLIC Rectangle                 GetTextRect() const;

        SVX_DLLPUBLIC sal_uInt32                    GetHdlCount() const;
        SVX_DLLPUBLIC sal_Bool                  GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const;
        SVX_DLLPUBLIC sal_Bool                  SetHandleControllerPosition( const sal_uInt32 nIndex, const com::sun::star::awt::Point& rPosition );

        SVX_DLLPUBLIC EnhancedCustomShape2d( SdrObject* pSdrObjCustomShape );
        SVX_DLLPUBLIC ~EnhancedCustomShape2d();

        enum EnumFunc
        {
            ENUM_FUNC_PI,
            ENUM_FUNC_LEFT,
            ENUM_FUNC_TOP,
            ENUM_FUNC_RIGHT,
            ENUM_FUNC_BOTTOM,
            ENUM_FUNC_XSTRETCH,
            ENUM_FUNC_YSTRETCH,
            ENUM_FUNC_HASSTROKE,
            ENUM_FUNC_HASFILL,
            ENUM_FUNC_WIDTH,
            ENUM_FUNC_HEIGHT,
            ENUM_FUNC_LOGWIDTH,
            ENUM_FUNC_LOGHEIGHT
        };
        double                      GetEnumFunc( const EnumFunc eVal ) const;

        double                      GetAdjustValueAsDouble( const sal_Int32 nIndex ) const;
        double                      GetEquationValueAsDouble( const sal_Int32 nIndex ) const;
        sal_Int32                   GetAdjustValueAsInteger( const sal_Int32 nIndex, const sal_Int32 nDefault = 0 ) const;

        SVX_DLLPUBLIC static rtl::OUString      GetEquation( const sal_uInt16 nFlags, sal_Int16 nPara1, sal_Int16 nPara2, sal_Int16 nPara3 );

        static void                 AppendEnhancedCustomShapeEquationParameter( rtl::OUString& rParameter, const sal_Int16 nPara, const sal_Bool bIsSpecialValue );

        static void                 SetEnhancedCustomShapeEquationParameter( com::sun::star::drawing::EnhancedCustomShapeParameter&
                                        rParameter, const sal_Int16 nPara, const sal_Bool bIsSpecialValue );
        SVX_DLLPUBLIC static void                   SetEnhancedCustomShapeParameter( com::sun::star::drawing::EnhancedCustomShapeParameter&
                                        rParameter, const sal_Int32 nValue );
        SVX_DLLPUBLIC static void                   SetEnhancedCustomShapeHandleParameter( com::sun::star::drawing::EnhancedCustomShapeParameter&
                                        rParameter, const sal_Int32 nPara, const sal_Bool bIsSpecialValue, sal_Bool bHorz );
        static sal_Bool             ConvertSequenceToEnhancedCustomShape2dHandle( const com::sun::star::beans::PropertyValues& rHandleProperties,
                                        EnhancedCustomShape2d::Handle& rDestinationHandle );
        static void                 SwapStartAndEndArrow( SdrObject* pObj );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
