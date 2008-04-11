/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodevisitor.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_NODEVISITOR_HXX
#define CONFIGMGR_NODEVISITOR_HXX

#include "nodeaccess.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
      //        class NodeAccess;
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
            Result visitNode(NodeAccess const& _aNode);

        protected:
            /// do the operation on <var>aNode</var>. Default implementation returns CONTINUE.
            virtual Result handle(NodeAccess const& _aNode);

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
            using NodeVisitor::handle;
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

