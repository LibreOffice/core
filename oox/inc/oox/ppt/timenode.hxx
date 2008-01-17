/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timenode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
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


#ifndef OOX_DRAWINGML_TIMENODE_HXX
#define OOX_DRAWINGML_TIMENODE_HXX

#include <boost/shared_ptr.hpp>
#include <vector>
#include <list>
#include <rtl/ustring.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "oox/helper/propertymap.hxx"
#include "oox/ppt/slidetransition.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "oox/ppt/animationspersist.hxx"
#include "oox/ppt/timenode.hxx"

namespace oox { namespace ppt {

    class TimeNode;
    class SlideTransition;

    typedef boost::shared_ptr< TimeNode > TimeNodePtr;
    typedef ::std::list< TimeNodePtr > TimeNodePtrList;

    class TimeNode
    {
    public:
        TimeNode( sal_Int16 nNodeType );
        virtual ~TimeNode();

        NodePropertyMap & getNodeProperties() { return maNodeProperties; }
        PropertyMap & getUserData() { return maUserData; }
        void addChild( const TimeNodePtr & pChildPtr )
            { maChilds.push_back( pChildPtr ); }

        TimeNodePtrList & getChilds()
            { return maChilds; }

        void setId( sal_Int32 nId );
        const ::rtl::OUString & getId() const { return msId; }

        void addNode( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &rxModel,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rxNode,
                                    const SlidePersistPtr & slide);
        // data setters
        void setTo( const ::com::sun::star::uno::Any & aTo );
        void setFrom( const ::com::sun::star::uno::Any & aFrom );
        void setBy( const ::com::sun::star::uno::Any & aBy );
        void setTransitionFilter( const SlideTransition & aTransition)
            { maTransitionFilter = aTransition; }

        void setNode( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &rxModel,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const SlidePersistPtr & pSlide );

        AnimTargetElementPtr getTarget()
            {
                if( !mpTarget )
                    mpTarget.reset( new AnimTargetElement );
                return mpTarget;
            }

        AnimationConditionList &getStartCondition()
            { return maStCondList; }
        AnimationConditionList &getEndCondition()
            { return maEndCondList; }
        AnimationConditionList &getNextCondition()
            { return maNextCondList; }
        AnimationConditionList &getPrevCondition()
            { return maPrevCondList; }
        AnimationCondition & getEndSyncValue()
            { mbHasEndSyncValue = true; return maEndSyncValue; }
    protected:

        static rtl::OUString getServiceName( sal_Int16 nNodeType );

        ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >
        createAndInsert( const rtl::OUString& rServiceName,
                                         const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &rxModel,
                                         const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rxNode );

    private:
        const sal_Int16 mnNodeType;

        TimeNodePtrList maChilds;

        rtl::OUString   msId;
        NodePropertyMap            maNodeProperties;
        PropertyMap                maUserData; // a sequence to be stored as "UserData" property
        SlideTransition            maTransitionFilter;
        AnimTargetElementPtr       mpTarget;
        bool                       mbHasEndSyncValue; // set to true if we try to get the endSync.
        AnimationCondition         maEndSyncValue;
        AnimationConditionList     maStCondList, maEndCondList;
        AnimationConditionList     maPrevCondList, maNextCondList;
    };

} }


#endif
