/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valuenodeimpl.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#define CONFIGMGR_VALUENODEBEHAVIOR_HXX_

#include "nodeimpl.hxx"

namespace configmgr
{
    class ValueChange;

    namespace configuration
    {
//-----------------------------------------------------------------------------
// Another types of node
//-----------------------------------------------------------------------------

        /** a special kind of node that is used to represent an element of a set of values
            <p> This is an immutable value (changes are done by adding/replacing/removing set elements)
            </p>
        */
    class ValueElementNodeImpl : public NodeImpl
    {
    public:
            explicit ValueElementNodeImpl(sharable::ValueNode * const& _aNodeRef) ;

    // the following delegate directly to the original node
    public:
        /// Does this node assume its default value
        /// retrieve the current value of this node
        com::sun::star::uno::Any    getValue() const;

        /// get the type of this value
        com::sun::star::uno::Type   getValueType()  const;

        sharable::ValueNode * getDataAccess() const;
    };

    // domain-specific 'dynamic_cast' replacement
    ValueElementNodeImpl&   AsValueNode(NodeImpl& rNode);
    }
}

#endif // CONFIGMGR_VALUENODEBEHAVIOR_HXX_
