/*************************************************************************
 *
 *  $RCSfile: valuenodeimpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:41:31 $
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

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#define CONFIGMGR_VALUENODEBEHAVIOR_HXX_

#include "nodeimpl.hxx"

namespace configmgr
{
    class ValueNode;
    class ValueChange;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;
        typedef com::sun::star::uno::Type UnoType;

        class ValueChangeImpl;

        class Name;
//-----------------------------------------------------------------------------

// Another types of node
//-----------------------------------------------------------------------------

        /** a special kind of node that is used to represent an element of a set of values
            <p> This is an immutable value (changes are done by adding/replacing/removing set elements)
            </p>
        */
        class ValueElementNodeImpl : public NodeImpl
        {
            ValueNode& m_rOriginal;
        public:
            explicit ValueElementNodeImpl(ValueNode& rOriginal) ;
            explicit ValueElementNodeImpl(ValueElementNodeImpl& rOriginal) ;

            /// retrieve the name of the underlying node
            OUString getOriginalNodeName() const;

        // the following delegate directly to m_rOriginal
        public:
            /// Does this node assume its default value
            /// retrieve the current value of this node
            UnoAny  getValue() const;

            /// get the type of this value
            UnoType getValueType()  const;

        protected:
            virtual Attributes doGetAttributes() const;

        private: // change handling. Is ineffective as this is an immutable object
            virtual bool doHasChanges() const;
            virtual void doMarkChanged();
            virtual void doCommitChanges();

            void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;

            // virtual NodeImplHolder doCloneIndirect(bool bIndirect) = 0;
        private:
            virtual NodeType::Enum  doGetType() const;
            virtual void            doDispatch(INodeHandler& rHandler);
        };

//-----------------------------------------------------------------------------
        // domain-specific 'dynamic_cast' replacement
        ValueElementNodeImpl&   AsValueNode(NodeImpl& rNode);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_VALUENODEBEHAVIOR_HXX_
