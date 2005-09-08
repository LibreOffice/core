/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerupdate.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:30:51 $
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

#include "layerupdate.hxx"

#ifndef CONFIGMGR_BACKEND_UPDATEDATA_HXX
#include "updatedata.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

LayerUpdate::LayerUpdate()
: m_xContextNode()
{
}
// -----------------------------------------------------------------------------

LayerUpdate::LayerUpdate(LayerUpdate const & _aOther)
: m_xContextNode(_aOther.m_xContextNode)
, m_aContextPath(_aOther.m_aContextPath)
{
}
// -----------------------------------------------------------------------------

LayerUpdate::~LayerUpdate()
{
}
// -----------------------------------------------------------------------------

LayerUpdate & LayerUpdate::operator =(LayerUpdate const & _aOther)
{
    m_xContextNode = _aOther.m_xContextNode;
    return *this;
}
// -----------------------------------------------------------------------------

void LayerUpdate::setContextNode(NodeUpdateRef const & _xContextNode)
{
    m_xContextNode = _xContextNode;
    if (_xContextNode.is())
        makeContextPath(_xContextNode->getName());

    else
        m_aContextPath.clear();
}
// -----------------------------------------------------------------------------

void LayerUpdate::makeContextPath(OUString const & _aPath)
{
    using configuration::AbsolutePath;
    AbsolutePath const aParsedPath = configuration::AbsolutePath::parse(_aPath);

    m_aContextPath.clear();
    m_aContextPath.reserve( aParsedPath.getDepth() );
    for (AbsolutePath::Iterator it = aParsedPath.begin(); it != aParsedPath.end(); ++it)
    {
        m_aContextPath.push_back( it->getName().toString() );
    }
}
// -----------------------------------------------------------------------------

NodeUpdateRef   LayerUpdate::getContextNode() const
{
    return m_xContextNode;
}
// -----------------------------------------------------------------------------

LayerUpdate::ContextPath const & LayerUpdate::getContextPath() const
{
    OSL_PRECOND( m_xContextNode.is(), "Cannot get context path without context node" );

    return m_aContextPath;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
