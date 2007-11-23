/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valuenodeimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:48:41 $
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

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#define CONFIGMGR_VALUENODEBEHAVIOR_HXX_

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif

namespace configmgr
{
    class ValueChange;

    namespace data
    {
        class GroupNodeAccess;
        class ValueNodeAccess;
    }
//-----------------------------------------------------------------------------

    namespace configuration
    {
    typedef com::sun::star::uno::Any UnoAny;
    typedef com::sun::star::uno::Type UnoType;
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
            explicit ValueElementNodeImpl(data::ValueNodeAddress const& _aNodeRef) ;

    // the following delegate directly to the original node
    public:
        /// Does this node assume its default value
        /// retrieve the current value of this node
        UnoAny  getValue() const;

        /// get the type of this value
        UnoType getValueType()  const;

        typedef data::ValueNodeAccess DataAccess;
            DataAccess getDataAccess() const;
    };

    // domain-specific 'dynamic_cast' replacement
    ValueElementNodeImpl&   AsValueNode(NodeImpl& rNode);
    }
}

#endif // CONFIGMGR_VALUENODEBEHAVIOR_HXX_
