/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationspersist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
