/*************************************************************************
 *
 *  $RCSfile: basecontainernode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:05:10 $
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

#ifndef _SLIDESHOW_BASECONTAINERNODE_HXX
#define _SLIDESHOW_BASECONTAINERNODE_HXX

#include <basenode.hxx>

namespace presentation
{
    namespace internal
    {
        /** This interface extends BaseNode with child handling methods.

            Used for XAnimationNode objects which have children
         */
        class BaseContainerNode : public BaseNode
        {
        public:
            BaseContainerNode( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::animations::XAnimationNode >& xNode,
                               const ::boost::shared_ptr< BaseContainerNode >&      rParent,
                               const NodeContext&                                   rContext );

            // overrides from BaseNode
            virtual bool init();
            virtual void dispose();
            virtual void end();

            virtual bool hasPendingAnimation() const;

            /** Add given child node to this container
              */
            virtual void appendChildNode( const BaseNodeSharedPtr& rNode );

            /** Requests node to resolve all children

                When parents have unresolved start times, and one
                of the children wants to start (e.g. because of a
                user event), it must call this method on its
                parent.
             */
            virtual void requestResolveOnChildren();

#if defined(VERBOSE) && defined(DBG_UTIL)
            virtual void showState() const;
            virtual const char* getDescription() const;
#endif

        protected:
            /** Overridden, to overrule indefinite begin event.

                When children requested resolve, indefinite begin
                times become resolved for us
             */
            virtual void scheduleActivationEvent();

            typedef ::std::vector< BaseNodeSharedPtr > VectorOfNodes;

            VectorOfNodes&      getChildren() { return maChildren; }
            ::std::bit_vector&  getFinishedStates() { return maFinishedStates; }
            ::std::size_t&      getFinishedCount() { return mnFinishedChildren; }
            bool                isDurationInfinite() const { return mbDurationIndefinite; }

        private:
            VectorOfNodes       maChildren;
            ::std::bit_vector   maFinishedStates;
            ::std::size_t       mnFinishedChildren;
            bool                mbOverrideIndefiniteBegin;
            const bool          mbDurationIndefinite;
        };

        typedef ::boost::shared_ptr< BaseContainerNode > BaseContainerNodeSharedPtr;
    }
}

#endif /* _SLIDESHOW_BASECONTAINERNODE_HXX */
