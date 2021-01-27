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


#ifndef INCLUDED_OOX_PPT_ANIMATIONSPERSIST_HXX
#define INCLUDED_OOX_PPT_ANIMATIONSPERSIST_HXX

#include <array>
#include <vector>
#include <memory>

#include <com/sun/star/uno/Any.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox::ppt {

    enum {
        NP_TO = 0,
        NP_FROM, NP_BY, NP_USERDATA, NP_ATTRIBUTENAME,
        NP_ACCELERATION, NP_AUTOREVERSE, NP_DECELERATE, NP_DURATION, NP_FILL,
        NP_REPEATCOUNT, NP_REPEATDURATION, NP_RESTART,
        NP_DIRECTION, NP_COLORINTERPOLATION, NP_CALCMODE, NP_TRANSFORMTYPE,
        NP_PATH,
        NP_ENDSYNC, NP_ITERATETYPE, NP_ITERATEINTERVAL,
        NP_SUBITEM, NP_TARGET, NP_COMMAND, NP_PARAMETER,
        NP_VALUES, NP_FORMULA, NP_KEYTIMES, NP_DISPLAY,
        NP_HIDEDURINGSHOW,
        NP_ISNARRATION,
        NP_SIZE_
    };

    typedef std::array< css::uno::Any, NP_SIZE_ > NodePropertyMap;


    /** data for CT_TLShapeTargetElement */
    struct ShapeTargetElement
    {
        ShapeTargetElement()
            : mnType(0)
            , mnRangeType(0)
        {
            maRange.start = maRange.end = 0;
        }
        void convert( css::uno::Any & aAny, sal_Int16 & rSubType ) const;

        sal_Int32               mnType;
        sal_Int32               mnRangeType;
        drawingml::IndexRange   maRange;
        OUString msSubShapeId;
    };


    /** data for CT_TLTimeTargetElement */
    struct AnimTargetElement
    {
        AnimTargetElement()
            : mnType( 0 )
            {}
        /** convert to a set of properties */
        css::uno::Any convert(const SlidePersistPtr & pSlide, sal_Int16 & nSubType) const;

        sal_Int32                  mnType;
        OUString                   msValue;
        ShapeTargetElement         maShapeTarget;
    };

    typedef std::shared_ptr< AnimTargetElement > AnimTargetElementPtr;

    struct AnimationCondition;

    typedef std::vector< AnimationCondition > AnimationConditionList;

    /** data for CT_TLTimeCondition */
    struct AnimationCondition
    {
        AnimationCondition()
            : mnType( 0 )
            {}

        css::uno::Any convert(const SlidePersistPtr & pSlide) const;
        static css::uno::Any convertList(const SlidePersistPtr & pSlide, const AnimationConditionList & l);

        AnimTargetElementPtr &     getTarget()
            { if(!mpTarget) mpTarget = std::make_shared<AnimTargetElement>(); return mpTarget; }
        css::uno::Any              maValue;
        sal_Int32                  mnType;
    private:
        AnimTargetElementPtr       mpTarget;
    };


    struct TimeAnimationValue
    {
        OUString            msFormula;
        OUString            msTime;
        css::uno::Any       maValue;
    };

    typedef ::std::vector< TimeAnimationValue > TimeAnimationValueList;

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
