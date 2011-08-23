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


#ifndef OOX_PPT_ANIMATIONPERSIST
#define OOX_PPT_ANIMATIONPERSIST

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/XShape.hpp>

#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/ppt/slidepersist.hxx"

namespace oox { namespace ppt {

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
        _NP_SIZE
    };

    typedef boost::array< ::com::sun::star::uno::Any, _NP_SIZE > NodePropertyMap;


    /** data for CT_TLShapeTargetElement */
    struct ShapeTargetElement
    {
        ShapeTargetElement()
            : mnType( 0 )
            {}
        void convert( ::com::sun::star::uno::Any & aAny, sal_Int16 & rSubType ) const;

        sal_Int32               mnType;
        sal_Int32               mnRangeType;
        drawingml::IndexRange   maRange;
        ::rtl::OUString msSubShapeId;
    };


    /** data for CT_TLTimeTargetElement */
    struct AnimTargetElement
    {
        AnimTargetElement()
            : mnType( 0 )
            {}
        /** convert to a set of properties */
        ::com::sun::star::uno::Any convert(const SlidePersistPtr & pSlide, sal_Int16 & nSubType) const;

        sal_Int32                  mnType;
        ::rtl::OUString            msValue;

      ShapeTargetElement         maShapeTarget;
    };

    typedef boost::shared_ptr< AnimTargetElement > AnimTargetElementPtr;

    struct AnimationCondition;

    typedef ::std::list< AnimationCondition > AnimationConditionList;

    /** data for CT_TLTimeCondition */
    struct AnimationCondition
    {
        AnimationCondition()
            : mnType( 0 )
            {}

        ::com::sun::star::uno::Any convert(const SlidePersistPtr & pSlide) const;
        static ::com::sun::star::uno::Any convertList(const SlidePersistPtr & pSlide, const AnimationConditionList & l);

        AnimTargetElementPtr &     getTarget()
            { if(!mpTarget) mpTarget.reset( new AnimTargetElement ); return mpTarget; }
        ::com::sun::star::uno::Any maValue;
        sal_Int32                  mnType;
    private:
        AnimTargetElementPtr       mpTarget;
    };


    struct TimeAnimationValue
    {
        ::rtl::OUString            msFormula;
        ::rtl::OUString            msTime;
        ::com::sun::star::uno::Any maValue;
    };

    typedef ::std::list< TimeAnimationValue > TimeAnimationValueList;

} }





#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
