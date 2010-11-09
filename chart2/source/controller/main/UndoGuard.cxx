/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_chart2.hxx"

#include "UndoGuard.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

UndoGuard_Base::UndoGuard_Base( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager )
        : m_xUndoManager( xUndoManager )
        , m_aUndoString( rUndoString )
        , m_bActionPosted( false )
{
}

UndoGuard_Base::~UndoGuard_Base()
{
}

void UndoGuard_Base::commitAction()
{
    if( !m_bActionPosted && m_xUndoManager.is() )
        m_xUndoManager->postAction( m_aUndoString );
    m_bActionPosted = true;
}

//-----------------------------------------------------------------------------

UndoGuard::UndoGuard( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager )
        : UndoGuard_Base( rUndoString, xUndoManager )
{
    if( m_xUndoManager.is() )
        m_xUndoManager->preAction();
}

UndoGuard::~UndoGuard()
{
    if( !m_bActionPosted && m_xUndoManager.is() )
        m_xUndoManager->cancelAction();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuard::UndoLiveUpdateGuard( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager )
        : UndoGuard_Base( rUndoString, xUndoManager )
{
    if( m_xUndoManager.is() )
        m_xUndoManager->preAction();
}

UndoLiveUpdateGuard::~UndoLiveUpdateGuard()
{
    if( !m_bActionPosted && m_xUndoManager.is() )
        m_xUndoManager->cancelActionWithUndo();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuardWithData::UndoLiveUpdateGuardWithData( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager )
        : UndoGuard_Base( rUndoString, xUndoManager )
{
    if( m_xUndoManager.is() )
    {
        Sequence< beans::PropertyValue > aArgs(1);
        aArgs[0] = beans::PropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("WithData")), -1, uno::Any(),
            beans::PropertyState_DIRECT_VALUE );
        m_xUndoManager->preActionWithArguments( aArgs );
    }
}

UndoLiveUpdateGuardWithData::~UndoLiveUpdateGuardWithData()
{
    if( !m_bActionPosted && m_xUndoManager.is() )
        m_xUndoManager->cancelActionWithUndo();
}

//-----------------------------------------------------------------------------

UndoGuardWithSelection::UndoGuardWithSelection( const rtl::OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager )
        : UndoGuard_Base( rUndoString, xUndoManager )
{
    if( m_xUndoManager.is() )
    {
        Sequence< beans::PropertyValue > aArgs(1);
        aArgs[0] = beans::PropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("WithSelection")), -1, uno::Any(),
            beans::PropertyState_DIRECT_VALUE );
        m_xUndoManager->preActionWithArguments( aArgs );
    }
}

UndoGuardWithSelection::~UndoGuardWithSelection()
{
    if( !m_bActionPosted && m_xUndoManager.is() )
        m_xUndoManager->cancelAction();
}

} //  namespace chart
