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

#ifndef INCLUDED_SVX_ENHANCEDCUSTOMSHAPE2D_HXX
#define INCLUDED_SVX_ENHANCEDCUSTOMSHAPE2D_HXX

#include <svx/msdffdef.hxx>
#include <svx/sdasitm.hxx>
#include <svx/svdotext.hxx>
#include <tools/degree.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>

#include <svx/EnhancedCustomShapeFunctionParser.hxx>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include <memory>
#include <vector>

namespace com::sun::star::awt { struct Point; }

class Color;
class SdrObject;
class SdrPathObj;
class SdrObjCustomShape;

enum class HandleFlags
{
    NONE                   = 0x0000,
    MIRRORED_X             = 0x0001,
    MIRRORED_Y             = 0x0002,
    SWITCHED               = 0x0004,
    POLAR                  = 0x0008,
    RANGE_X_MINIMUM        = 0x0020,
    RANGE_X_MAXIMUM        = 0x0040,
    RANGE_Y_MINIMUM        = 0x0080,
    RANGE_Y_MAXIMUM        = 0x0100,
    RADIUS_RANGE_MINIMUM   = 0x0200,
    RADIUS_RANGE_MAXIMUM   = 0x0400,
    REFX                   = 0x0800,
    REFY                   = 0x1000,
    REFANGLE               = 0x2000,
    REFR                   = 0x4000,
};
namespace o3tl
{
    template<> struct typed_flags<HandleFlags> : is_typed_flags<HandleFlags, 0x7fef> {};
}

// MSDFF_HANDLE_FLAGS_RANGE_Y seems to be not defined in
// escher, but we are using it internally in to differentiate
// between X_RANGE and Y_RANGE

class SVXCORE_DLLPUBLIC EnhancedCustomShape2d : public SfxItemSet
{
        SdrObjCustomShape&          mrSdrObjCustomShape;
        MSO_SPT                     eSpType;

        sal_Int32                   nCoordLeft;
        sal_Int32                   nCoordTop;
        sal_Int32                   nCoordWidthG;
        sal_Int32                   nCoordHeightG;
        sal_Int32                   nCoordWidth;
        sal_Int32                   nCoordHeight;
        tools::Rectangle                   aLogicRect;

        double                      fXScale;
        double                      fYScale;
        double                      fXRatio;
        double                      fYRatio;

        bool                        bOOXMLShape;

        sal_Int32                   nXRef;
        sal_Int32                   nYRef;
        sal_uInt32                  nColorData;

        /*

        */
        struct SAL_DLLPRIVATE EquationResult {
            bool bReady;
            double fValue;
        };
        css::uno::Sequence< OUString >                                                       seqEquations;
        std::vector< std::shared_ptr< EnhancedCustomShape::ExpressionNode > >                           vNodesSharedPtr;
        std::vector< EquationResult >                                                                   vEquationResults;

        css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment >            seqSegments;
        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair>       seqCoordinates;
        css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame >          seqTextFrames;
        css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair>       seqGluePoints;
        css::uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue >    seqAdjustmentValues;
        css::uno::Sequence< css::beans::PropertyValues >                          seqHandles;
        css::uno::Sequence< css::awt::Size >                                      seqSubViewSize;

        bool                    bFilled         : 1;
        bool                    bStroked        : 1;

        bool                    bFlipH;
        bool                    bFlipV;
        Degree100               nRotateAngle;

        SAL_DLLPRIVATE bool     SetAdjustValueAsDouble( const double& rValue, const sal_Int32 nIndex );
        SAL_DLLPRIVATE sal_Int32 GetLuminanceChange( sal_uInt32 nIndex ) const;
        SAL_DLLPRIVATE Color    GetColorData( const Color& rFillColor, sal_uInt32 nIndex, double dBrightness ) const;
        SAL_DLLPRIVATE void AdaptObjColor(
            SdrPathObj& rObj,
            double dBrightness,
            const SfxItemSet& rCustomShapeSet,
            sal_uInt32& nColorIndex,
            sal_uInt32 nColorCount);
        SAL_DLLPRIVATE Point    GetPoint( const css::drawing::EnhancedCustomShapeParameterPair&,
                                                    const bool bScale = true, const bool bReplaceGeoSize = false ) const;
        SAL_DLLPRIVATE basegfx::B2DPoint GetPointAsB2DPoint(const css::drawing::EnhancedCustomShapeParameterPair&,
                                                    const bool bScale = true, const bool bReplaceGeoSize = false ) const;

        SAL_DLLPRIVATE void CreateSubPath(
            sal_Int32& rSrcPt,
            sal_Int32& rSegmentInd,
            std::vector< std::pair< SdrPathObjUniquePtr, double> >& rObjectList,
            bool bLineGeometryNeededOnly,
            bool bSortFilledObjectsToBack,
            sal_Int32 nIndex);
        SAL_DLLPRIVATE SdrObjectUniquePtr CreatePathObj( bool bLineGeometryNeededOnly );
        SAL_DLLPRIVATE void     ApplyShapeAttributes( const SdrCustomShapeGeometryItem& rItem );

        SAL_DLLPRIVATE void     SetPathSize( sal_Int32 nIndex = 0 );

    public:

        struct SAL_DLLPRIVATE Handle
        {
            HandleFlags nFlags;

            css::drawing::EnhancedCustomShapeParameterPair   aPosition;
            css::drawing::EnhancedCustomShapeParameterPair   aPolar;

            sal_Int32   nRefX;
            sal_Int32   nRefY;
            sal_Int32   nRefAngle;
            sal_Int32   nRefR;

            css::drawing::EnhancedCustomShapeParameter       aRadiusRangeMinimum;
            css::drawing::EnhancedCustomShapeParameter       aRadiusRangeMaximum;
            css::drawing::EnhancedCustomShapeParameter       aXRangeMinimum;
            css::drawing::EnhancedCustomShapeParameter       aXRangeMaximum;
            css::drawing::EnhancedCustomShapeParameter       aYRangeMinimum;
            css::drawing::EnhancedCustomShapeParameter       aYRangeMaximum;

            Handle()
                : nFlags(HandleFlags::NONE)
                , nRefX( -1 )
                , nRefY( -1 )
                , nRefAngle( -1 )
                , nRefR( -1 )
            {
            }
        };

        SAL_DLLPRIVATE bool     IsFlipVert() const { return bFlipV; };
        SAL_DLLPRIVATE bool     IsFlipHorz() const { return bFlipH; };
        SAL_DLLPRIVATE Degree100 GetRotateAngle() const { return nRotateAngle; };

        SdrObjectUniquePtr      CreateLineGeometry();
        SdrObjectUniquePtr      CreateObject( bool bLineGeometryNeededOnly );
        void                    ApplyGluePoints( SdrObject* pObj );
        tools::Rectangle        GetTextRect() const;
        const tools::Rectangle& GetLogicRect() const { return aLogicRect; }

        sal_uInt32              GetHdlCount() const;
        bool                    GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const;
        bool                    SetHandleControllerPosition( const sal_uInt32 nIndex, const css::awt::Point& rPosition );

        EnhancedCustomShape2d(SdrObjCustomShape& rSdrObjCustomShape);
        virtual ~EnhancedCustomShape2d() override;

        SAL_DLLPRIVATE double   GetEnumFunc( const EnhancedCustomShape::ExpressionFunct eVal ) const;

        void     GetParameter( double& rParameterReturnValue,  const css::drawing::EnhancedCustomShapeParameter&,
                               const bool bReplaceGeoWidth, const bool bReplaceGeoHeight ) const;
        SAL_DLLPRIVATE double   GetAdjustValueAsDouble( const sal_Int32 nIndex ) const;
        SAL_DLLPRIVATE double   GetEquationValueAsDouble( const sal_Int32 nIndex ) const;

        static OUString         GetEquation( const sal_uInt16 nFlags, sal_Int32 nPara1, sal_Int32 nPara2, sal_Int32 nPara3 );

        SAL_DLLPRIVATE static void AppendEnhancedCustomShapeEquationParameter( OUString& rParameter, const sal_Int32 nPara, const bool bIsSpecialValue );

        static void             SetEnhancedCustomShapeParameter( css::drawing::EnhancedCustomShapeParameter&
                                        rParameter, const sal_Int32 nValue );
        static void             SetEnhancedCustomShapeHandleParameter( css::drawing::EnhancedCustomShapeParameter&
                                        rParameter, const sal_Int32 nPara, const bool bIsSpecialValue, bool bHorz );
        SAL_DLLPRIVATE static bool ConvertSequenceToEnhancedCustomShape2dHandle( const css::beans::PropertyValues& rHandleProperties,
                                        EnhancedCustomShape2d::Handle& rDestinationHandle );
        SAL_DLLPRIVATE static void SwapStartAndEndArrow( SdrObject* pObj );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
