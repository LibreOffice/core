/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treefragment.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:25:38 $
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

#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#define INCLUDED_SHARABLE_TREEFRAGMENT_HXX

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#include "types.hxx"
#endif
#ifndef INCLUDED_DATA_FLAGS_HXX
#include "flags.hxx"
#endif
#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
    //-----------------------------------------------------------------------------
        namespace State = data::State;
    //-----------------------------------------------------------------------------
    /* a TreeFragment header is interpreted differently, depending on the kind of TreeFragment

         - for a set element
             name points to the element name (the Name in the root node is the template name)
             parent  points to the SetNode that is the parent. The containing treefragment can
                     be recovered from this with some care
             next points to the next element of the same set. It is null for the last element.
             state is fully used here

         - for a template tree
             name points to the template name (same as the Name in the root node)
             component points to the home component name of the template
                        (often the same as 'component' in the component tree)
             next points to another template TreeFragment. It is null for the last template.
             state must be 'replaced' here (rarely it might be marked as mandatory)

         - for a component tree
             name points to the component name (same as the Name in the root node)
             component is equal to name (or NULL ?)
             next points to another template TreeFragment. It is null if there is no template.
             state must be either 'defaulted' or 'merged'
                   (it should be marked as mandatory although that is not used yet)
    */
        struct TreeFragmentHeader
        {
            struct TreeFragment *next;       // next sibling set element or template
            String               name;       // element-name/template name
            union // context
            {
                union Node *parent;          // parent node
                String      component;       // component name
            };
            Offset               count;      // number of contained nodes
            State::Field         state;
            sal_uInt8            reserved;
       };
    //-----------------------------------------------------------------------------
    /* a tree fragment is stored as a variable-sized struct
       containing a header and a fixed sequence of nodes

        R - - A - - A1
           |     |
           |      - A2
           |
            - B
           |
            - C - - C1 - - C11
           |     |      |
           |     |       - C12
           |     |
           |      - C2 - - C21
           |
            - D

      is stored as

          H(count = 12) : [R;A;A1;A2;B;C;C1;C11;C12;C2;C21;D]
    */
    //-----------------------------------------------------------------------------
    /* tree fragments are used for: Component trees, Template trees, Set elements

        They are only fragments, as for a TreeFragment a Set is considered a leaf node.
        Set elements are maintained as a singly linked list that is accessible from the set node

        A cache component has the Root (component tree) TreeFragment at a well-known location.
        The 'next' element of the component tree points to the list of template TreeFragments
        used in the component.
    */
        struct TreeFragment
        {
            TreeFragmentHeader  header;     // really variable-sized:
            Node                nodes[1];   //   nodes[header.count]

            // header info access
            bool hasDefaults() const;
            bool hasDefaultsAvailable() const;

            bool isDefault() const;
            bool isNew() const;

            bool isNamed(rtl::OUString const & _aName) const;

            rtl::OUString               getName() const;
            configmgr::node::Attributes getAttributes()const;

            static TreeFragment *allocate(sal_uInt32 nFragments);
            static void free_shallow( TreeFragment *pFragment );
        };
    //-----------------------------------------------------------------------------

    }
  namespace data {
    typedef sharable::TreeFragment * TreeAddress;
  }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_SHARABLE_TREEFRAGMENT_HXX
