/*************************************************************************
 *
 *  $RCSfile: nodevisitor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:34:01 $
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

#ifndef CONFIGMGR_NODEVISITOR_HXX
#define CONFIGMGR_NODEVISITOR_HXX

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class NodeAccessRef;
            class ValueNodeAccess;
            class GroupNodeAccess;
            class SetNodeAccess;
        class TreeAccessor;

    // -------------------------------------------------------------------------
        /** interface for a class that can be used to do some operation on a set of <type>NodeAccess</type>s
            <p>Concrete classes should override at least one of the 'handle' methods.</p>
        */
        class NodeVisitor
        {
        public:
            /// returned from <method>handle</method> to indicate whether the operation is complete or should continue
            enum Result { DONE, CONTINUE };

            /// destructor. is pure to make this class abstract
            virtual ~NodeVisitor() ; // = 0; - does not work well with SunCC 5.2

            /// dispatch this to the children of <var>aNode</var>, until one returns DONE
            Result visitChildren(GroupNodeAccess const& _aNode);

            /// dispatch to <var>aNode</var> as the proper type
            Result visitNode(NodeAccessRef const& _aNode);

        protected:
            /// do the operation on <var>aNode</var>. Default implementation returns CONTINUE.
            virtual Result handle(NodeAccessRef const& _aNode);

            /// do the operation on <var>aNode</var>. Default implementation calls handle(NodeAccess(_aNode));
            virtual Result handle(ValueNodeAccess const& _aNode);

            /** do the operation on <var>aNode</var>. Default implementation calls handle(NodeAccess(_aNode));
                <p>To recursively visit the whole tree, call <method>visitChildren</method>
                    in the implementation</p>
            */
            virtual Result handle(GroupNodeAccess const& _aNode);

            /** do the operation on <var>aNode</var>. Default implementation calls handle(NodeAccess(_aNode));
                <p>To recursively visit the whole tree, call <method>visitChildren</method>
                    in the implementation</p>
            */
            virtual Result handle(SetNodeAccess const& _aNode);

            struct Dispatcher;
            friend struct Dispatcher;
        };

        inline NodeVisitor::~NodeVisitor() {}

    // -------------------------------------------------------------------------
        /** interface for a class that can be used to do some operation on a set of <type>TreeAccess</type>s
            <p>Concrete classes should override at least one of the 'handle' methods.</p>
        */
        class SetVisitor : public NodeVisitor
        {
        public:
            /// destructor. is pure to make this class abstract
            virtual ~SetVisitor() ; //= 0; - does not work well with SunCC 5.2

            /// dispatch to <var>_aElementTree</var>
            Result visitTree(TreeAccessor const& _aElementTree);

            /// dispatch this to the elements of <var>aNode</var>, until one returns DONE
            Result visitElements(SetNodeAccess const& _aNode);

        protected:
            /// do the operation on <var>aNode</var>. Default implementation call NodeVisitor::visit for the rootnode.
            virtual Result handle(TreeAccessor const& _aElementTree);

            struct Dispatcher;
            friend struct Dispatcher;
        };

        inline SetVisitor::~SetVisitor() {}
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_NODEVISITOR_HXX

