/*************************************************************************
 *
 *  $RCSfile: apinodeupdate.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include "apinodeupdate.hxx"

#include "apitreeimplobj.hxx"

#include "apitreeaccess.hxx"
#include "noderef.hxx"
#include "configset.hxx"
#include "configgroup.hxx"
#include "configpath.hxx"

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------

configuration::GroupUpdater NodeGroupAccess::getNodeUpdater()
{
    return configuration::GroupUpdater(getTree(),getNode());
}
//-----------------------------------------------------------------------------

ISynchronizedData* NodeGroupAccess::getDataLock()
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

ISynchronizedData* NodeSetAccess::getDataLock()
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

configuration::SetElementFactory NodeTreeSetAccess::getElementFactory()
{
    configuration::TemplateProvider aProvider = getApiTree().getProvider().getTemplateProvider();
    return configuration::SetElementFactory(aProvider);
}
//-----------------------------------------------------------------------------

configuration::TreeSetUpdater NodeTreeSetAccess::getNodeUpdater()
{
    return configuration::TreeSetUpdater(getTree(),getNode(),getElementInfo());
}
//-----------------------------------------------------------------------------


configuration::ValueSetUpdater NodeValueSetAccess::getNodeUpdater()
{
    return configuration::ValueSetUpdater(getTree(),getNode(),getElementInfo());
}
//-----------------------------------------------------------------------------

UpdateGuardImpl::UpdateGuardImpl(NodeGroupAccess& rNode) throw()
: m_aProviderLock(rNode.getProviderLock())
, m_aLock(rNode.getDataLock())
, m_rNode(rNode)
{
    rNode.checkAlive();
}
//-----------------------------------------------------------------------------

UpdateGuardImpl::UpdateGuardImpl(NodeSetAccess& rNode) throw()
: m_aProviderLock(rNode.getProviderLock())
, m_aLock(rNode.getDataLock())
, m_rNode(rNode)
{
}
//-----------------------------------------------------------------------------

UpdateGuardImpl::~UpdateGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------
    }
}
