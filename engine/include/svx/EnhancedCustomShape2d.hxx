/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>

#include <svx/EnhancedCustomShapeFunctionParser.hxx>
#include <svx/EnhancedCustomShapeFormulas.hxx>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/sdrhandlerails.hxx>

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

class SVXCORE_DLLPUBLIC EnhancedCustomShape2d final : public SfxItemSet,
                                                     public EnhancedCustomShapeFormulas
{
        SdrObjCustomShape&          mrSdrObjCustomShape;

        sal_uInt32                  m_nColorData;

        Degree100               m_nRotateAngle;

        SAL_DLLPRIVATE Color    GetColorData( const Color& rFillColor, sal_uInt32 nIndex, double dBrightness ) const;
        SAL_DLLPRIVATE void AdaptObjColor(
            SdrPathObj& rObj,
            double dBrightness,
            const SfxItemSet& rCustomShapeSet,
            sal_uInt32& nColorIndex,
            sal_uInt32 nColorCount);

        SAL_DLLPRIVATE void CreateSubPath(
            sal_Int32& rSrcPt,
            sal_Int32& rSegmentInd,
            std::vector< std::pair< rtl::Reference<SdrPathObj>, double> >& rObjectList,
            bool bLineGeometryNeededOnly,
            bool bSortFilledObjectsToBack,
            sal_Int32 nIndex);
        SAL_DLLPRIVATE rtl::Reference<SdrObject> CreatePathObj( bool bLineGeometryNeededOnly );

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

        sal_Int32 GetLuminanceChange( sal_uInt32 nIndex ) const;
        SAL_DLLPRIVATE Degree100 GetRotateAngle() const { return m_nRotateAngle; };

        rtl::Reference<SdrObject>      CreateLineGeometry();
        rtl::Reference<SdrObject>      CreateObject( bool bLineGeometryNeededOnly );
        rtl::Reference<SdrObject>      CreateObject( bool bLineGeometryNeededOnly, SfxStyleSheet* pNewStyleSheet );
        void                    ApplyGluePoints( SdrObject* pObj );

        sal_uInt32              GetHdlCount() const;
        bool                    GetHandlePosition( const sal_uInt32 nIndex, Point& rReturnPosition ) const;

        /// How far the handle of that number may be moved. False where the shape has no such
        /// handle.
        /** How far the handle of that number may be moved. False where the shape has no such
            handle. Working the ends out puts values into the shape, so this is asked of a shape
            kept for the purpose and never of one that is drawn.
         */
        bool GetHandleRails(const sal_uInt32 nIndex, SdrHandleRails& rRails);

        /** Where the handle of that number would stand if the value it is bound to held that
            one. The shape this is asked of is changed, so it is asked of a shape kept for
            working places out and never of one that is drawn.
         */
        SAL_DLLPRIVATE bool GetHandlePositionAt(const sal_uInt32 nIndex, const sal_Int32 nValue,
                                                const double fValue, basegfx::B2DPoint& rPosition);

        bool                    SetHandleControllerPosition( const sal_uInt32 nIndex, const css::awt::Point& rPosition );

        EnhancedCustomShape2d(SdrObjCustomShape& rSdrObjCustomShape);
        virtual ~EnhancedCustomShape2d() override;


        static OUString         GetEquation( const sal_uInt16 nFlags, sal_Int32 nPara1, sal_Int32 nPara2, sal_Int32 nPara3 );

        /// Removes the shadow, the glow, the soft edge and the reflection from the items. The
        /// custom shape draws them once around all of its geometry and text.
        static void ClearEffects(SfxItemSet& rItemSet);

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
