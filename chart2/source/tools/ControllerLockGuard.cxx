/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControllerLockGuard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:57:07 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ControllerLockGuard.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ControllerLockGuard::ControllerLockGuard( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

ControllerLockGuard::~ControllerLockGuard()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

// ================================================================================

ControllerLockHelper::ControllerLockHelper( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{}

ControllerLockHelper::~ControllerLockHelper()
{}

void ControllerLockHelper::lockControllers()
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

void ControllerLockHelper::unlockControllers()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

// ================================================================================

ControllerLockHelperGuard::ControllerLockHelperGuard( ControllerLockHelper & rHelper ) :
        m_rHelper( rHelper )
{
    m_rHelper.lockControllers();
}

ControllerLockHelperGuard::~ControllerLockHelperGuard()
{
    m_rHelper.unlockControllers();
}

} //  namespace chart
