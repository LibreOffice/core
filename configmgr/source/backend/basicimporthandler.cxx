/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basicimporthandler.cxx,v $
 * $Revision: 1.7 $
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

#include "basicimporthandler.hxx"
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

BasicImportHandler::BasicImportHandler(
    uno::Reference< backenduno::XBackend > const & xBackend,rtl::OUString const & aEntity, const sal_Bool&  bNotify)
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
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aComponentName = rtl::OUString();
}
// -----------------------------------------------------------------------------

void SAL_CALL
    BasicImportHandler::endLayer(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
    m_aComponentName = rtl::OUString();
}
// -----------------------------------------------------------------------------

bool BasicImportHandler::startComponent( const rtl::OUString& aName )
{
    if (hasComponent()) return false;

    m_aComponentName = aName;
    return true;
}
// -----------------------------------------------------------------------------

void BasicImportHandler::raiseMalformedDataException(sal_Char const * pMsg)
{
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

    throw backenduno::MalformedDataException(sMsg, *this, uno::Any());
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

