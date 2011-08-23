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
        typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any > UserDataMap;

        TimeNode( sal_Int16 nNodeType );
        virtual ~TimeNode();

        NodePropertyMap & getNodeProperties() { return maNodeProperties; }
        UserDataMap & getUserData() { return maUserData; }
        void addChild( const TimeNodePtr & pChildPtr )
            { maChildren.push_back( pChildPtr ); }

        TimeNodePtrList & getChildren()
            { return maChildren; }

        void setId( sal_Int32 nId );
        const ::rtl::OUString & getId() const { return msId; }

        void addNode(
            const ::oox::core::XmlFilterBase& rFilter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rxNode,
            const SlidePersistPtr & slide);
        // data setters
        void setTo( const ::com::sun::star::uno::Any & aTo );
        void setFrom( const ::com::sun::star::uno::Any & aFrom );
        void setBy( const ::com::sun::star::uno::Any & aBy );
        void setTransitionFilter( const SlideTransition & aTransition)
            { maTransitionFilter = aTransition; }

        void setNode(
            const ::oox::core::XmlFilterBase& rFilter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
            const SlidePersistPtr & pSlide );

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
        createAndInsert(
            const ::oox::core::XmlFilterBase& rFilter,
            const rtl::OUString& rServiceName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rxNode );

    private:
        const sal_Int16 mnNodeType;

        TimeNodePtrList maChildren;

        rtl::OUString	msId;
        NodePropertyMap	           maNodeProperties;
        UserDataMap                maUserData; // a sequence to be stored as "UserData" property
        SlideTransition            maTransitionFilter;
        AnimTargetElementPtr       mpTarget;
        bool                       mbHasEndSyncValue; // set to true if we try to get the endSync.
        AnimationCondition         maEndSyncValue;
        AnimationConditionList     maStCondList, maEndCondList;
        AnimationConditionList     maPrevCondList, maNextCondList;
    };

} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
