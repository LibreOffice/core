/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeconverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:52:04 $
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

#ifndef CONFIGMGR_NODECONVERTER_HXX
#define CONFIGMGR_NODECONVERTER_HXX

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

//..........................................................................
namespace configmgr
{
//..........................................................................
    class OTreeNodeFactory;
//..........................................................................
    class OTreeNodeConverter
    {
        OTreeNodeFactory& m_rFactory;
    public:
        OTreeNodeConverter();
        OTreeNodeConverter(OTreeNodeFactory& _rFactory)
            : m_rFactory(_rFactory) {}

        OTreeNodeFactory& nodeFactory() const { return m_rFactory; }
    // node conversion functions
        std::auto_ptr<ISubtree>  createCorrespondingNode(SubtreeChange   const& _rChange);
        std::auto_ptr<ValueNode> createCorrespondingNode(ValueChange     const& _rChange);

        std::auto_ptr<INode>     createCorrespondingNode(Change& _rChange);
        std::auto_ptr<ISubtree>  createCorrespondingTree(SubtreeChange& _rChange);
    };

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // CONFIGMGR_NODECONVERTER_HXX


