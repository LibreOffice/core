/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicimporthandler.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:18:10 $
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

#include "basicimporthandler.hxx"
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

BasicImportHandler::BasicImportHandler(
    Backend const & xBackend,OUString const & aEntity, const sal_Bool&  bNotify)
: m_bSendNotification(bNotify)
, m_xBackend(xBackend)
, m_aComponentName()
, m_aEntity(aEntity)
{
    OSL_ENSURE( m_xBackend.is(), "Creating an import handler without a target backend" );
}
// -----------------------------------------------------------------------------

BasicImportHandler::~BasicImportHandler()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL
    BasicImportHandler::startLayer(  )
        throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aComponentName = OUString();
}
// -----------------------------------------------------------------------------

void SAL_CALL
    BasicImportHandler::endLayer(  )
        throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if ( m_bSendNotification)
    {
        backenduno::ComponentChangeEvent aEvent;
        aEvent.Source=*this;
        aEvent.Component = m_aComponentName;

        uno::Reference<backenduno::XBackendChangesListener> xListener(m_xBackend, uno::UNO_QUERY);
        if( xListener.is())
        {
            xListener->componentDataChanged(aEvent);
        }
        else
        {
            OSL_ENSURE(false, "ImportMergeHandler: target backend does not support notifications");
        }
    }
    m_aComponentName = OUString();
}
// -----------------------------------------------------------------------------

bool BasicImportHandler::startComponent( const OUString& aName )
{
    if (hasComponent()) return false;

    m_aComponentName = aName;
    return true;
}
// -----------------------------------------------------------------------------

void BasicImportHandler::raiseMalformedDataException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);

    throw MalformedDataException(sMsg, *this, uno::Any());
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

