/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apitreeaccess.cxx,v $
 * $Revision: 1.12 $
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
#include <stdio.h>
#include "apitreeaccess.hxx"

#include "apitreeimplobj.hxx"
#include "configset.hxx"
#include "confignotifier.hxx"
#include "committer.hxx"
#include "apinodeaccess.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------

        // self-locked methods for dispose handling
void NodeElement::checkAlive() const
{
    getApiTree().checkAlive();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::Tree > TreeElement::getTree() const
{
    return getTreeRef();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::Tree > TreeElement::getTreeRef() const
{
    return getApiTree().getTree();
}
//-----------------------------------------------------------------------------
        // self-locked methods for dispose handling
bool SetElement::disposeTree(bool bForce)
{
    return getApiTree().disposeTree(bForce);
}
//-----------------------------------------------------------------------------
Factory& TreeElement::getFactory()
{
    return getApiTree().getFactory();
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::ElementTree > SetElement::getElementRef() const
{
    return dynamic_cast< configuration::ElementTree * >(getTreeRef().get());
}
//-----------------------------------------------------------------------------

rtl::Reference< configuration::Template > SetElement::getTemplateInfo() const
{
    rtl::Reference< configuration::ElementTree > aTree(dynamic_cast< configuration::ElementTree * >(getTreeRef().get()));
    OSL_ENSURE(aTree.is(), "This really must be a set element");
    return aTree->getTemplate();
}
//-----------------------------------------------------------------------------

void SetElement::haveNewParent(NodeSetInfoAccess* pNewParent)
{
    ApiTreeImpl* pNewParentImpl = pNewParent ? &pNewParent->getApiTree() : 0;

    this->getApiTree().haveNewParent( pNewParentImpl );
}
//-----------------------------------------------------------------------------

bool RootElement::disposeTree()
{
    return getRootTree().disposeTree();
}
//-----------------------------------------------------------------------------

Committer UpdateRootElement::getCommitter()
{
    return Committer(getRootTree());
}
    }
}

