/*************************************************************************
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:38:21 $
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

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#define CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_

/*
#include "template.hxx"
#include <osl/diagnose.h>

#include <memory>
*/

#include "apitypes.hxx"
#include "attributes.hxx"

#include <vos/refernce.hxx>
#include <vos/ref.hxx>
namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef unsigned int NodeOffset;

        class TreeImpl;

        class NodeChange;
        class NodeChanges;
        class NodeChangesInformation;
//-----------------------------------------------------------------------------

// Specific types of nodes
//-----------------------------------------------------------------------------

        namespace NodeType
        {
        //---------------------------------------------------------------------
            enum Enum { eVALUE, eGROUP, eSET, eVALUESET, eTREESET };
            inline bool isSet(Enum e)   { return e >= eSET; }
            inline bool isGroup(Enum e) { return e == eGROUP; }
            inline bool isValue(Enum e) { return e == eVALUE; }

        //---------------------------------------------------------------------
        }

//-----------------------------------------------------------------------------
        class NodeImpl;
        typedef vos::ORef<NodeImpl> NodeImplHolder;

        struct INodeHandler;

        // Almost an interface, but derives from concrete OReference
        class NodeImpl : public vos::OReference
        {
        public:
            void collectChanges(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const
                { doCollectChangesWithTarget(rChanges,pParent,nNode); }

            bool hasChanges()               const   { return doHasChanges(); }
            void markChanged()                      { doMarkChanged(); }
            void directCommitChanges()              { doCommitChanges(); }

            static void makeIndirect(NodeImplHolder&    aThis, bool bIndirect);

            NodeType::Enum  getType()   const   { return doGetType(); }
            Attributes getAttributes()  const   { return doGetAttributes(); };
            void dispatch(INodeHandler& rHandler_)  { doDispatch(rHandler_); }

        private:
            virtual NodeType::Enum  doGetType()         const = 0;
            virtual Attributes      doGetAttributes()   const = 0;
            virtual void            doDispatch(INodeHandler& rHandler_) = 0;

            virtual bool doHasChanges() const = 0;
            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const = 0;
            virtual void doMarkChanged() = 0;
            virtual void doCommitChanges() = 0;

            virtual NodeImplHolder doCloneIndirect(bool bIndirect) = 0;

        protected:
            //helper for migration to new (info based) model for adjusting to changes
            static void addLocalChangeHelper( NodeChangesInformation& rLocalChanges, NodeChange const& aChange);

        };

//-----------------------------------------------------------------------------
        class ValueElementNodeImpl;
        class GroupNodeImpl;
        class SetNodeImpl;
//-----------------------------------------------------------------------------

        struct INodeHandler
        {
            virtual void handle( ValueElementNodeImpl& rNode) = 0;
            virtual void handle( GroupNodeImpl& rNode) = 0;
            virtual void handle( SetNodeImpl& rNode) = 0;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
