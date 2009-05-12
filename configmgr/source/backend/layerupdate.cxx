/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdate.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "layerupdate.hxx"
#include "updatedata.hxx"
#include "configpath.hxx"

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

void LayerUpdate::setContextNode(rtl::Reference<NodeUpdate> const & _xContextNode)
{
    m_xContextNode = _xContextNode;
    if (_xContextNode.is())
        makeContextPath(_xContextNode->getName());

    else
        m_aContextPath.clear();
}
// -----------------------------------------------------------------------------

void LayerUpdate::makeContextPath(rtl::OUString const & _aPath)
{
    configuration::AbsolutePath const aParsedPath = configuration::AbsolutePath::parse(_aPath);

    m_aContextPath.clear();
    m_aContextPath.reserve( aParsedPath.getDepth() );
    for (std::vector<configuration::Path::Component>::const_reverse_iterator it = aParsedPath.begin(); it != aParsedPath.end(); ++it)
    {
        m_aContextPath.push_back( it->getName() );
    }
}
// -----------------------------------------------------------------------------

rtl::Reference<NodeUpdate>   LayerUpdate::getContextNode() const
{
    return m_xContextNode;
}
// -----------------------------------------------------------------------------

std::vector<rtl::OUString> const & LayerUpdate::getContextPath() const
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
