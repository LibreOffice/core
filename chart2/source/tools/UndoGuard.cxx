/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoGuard.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:10:53 $
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

#include "UndoGuard.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

UndoGuard_Base::UndoGuard_Base( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager
        , const uno::Reference< frame::XModel > & xModel )
        : m_xModel( xModel )
        , m_xUndoManager( xUndoManager )
        , m_aUndoString( rUndoString )
        , m_bActionPosted( false )
{
}

UndoGuard_Base::~UndoGuard_Base()
{
}

void UndoGuard_Base::commitAction()
{
    if( !m_bActionPosted )
        m_xUndoManager->postAction( m_aUndoString );
    m_bActionPosted = true;
}

//-----------------------------------------------------------------------------

UndoGuard::UndoGuard( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager
        , const uno::Reference< frame::XModel > & xModel )
        : UndoGuard_Base( rUndoString, xUndoManager, xModel )
{
    m_xUndoManager->preAction( m_xModel );
}

UndoGuard::~UndoGuard()
{
    if( !m_bActionPosted )
        m_xUndoManager->cancelAction();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuard::UndoLiveUpdateGuard( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager
        , const uno::Reference< frame::XModel > & xModel )
        : UndoGuard_Base( rUndoString, xUndoManager, xModel )
{
    m_xUndoManager->preAction( m_xModel );
}

UndoLiveUpdateGuard::~UndoLiveUpdateGuard()
{
    if( !m_bActionPosted )
        m_xUndoManager->cancelActionWithUndo( m_xModel );
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuardWithData::UndoLiveUpdateGuardWithData( const OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager
        , const uno::Reference< frame::XModel > & xModel )
        : UndoGuard_Base( rUndoString, xUndoManager, xModel )
{
    Sequence< beans::PropertyValue > aArgs(1);
    aArgs[0] = beans::PropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("WithData")), -1, uno::Any(),
        beans::PropertyState_DIRECT_VALUE );
    m_xUndoManager->preActionWithArguments( m_xModel, aArgs );
}

UndoLiveUpdateGuardWithData::~UndoLiveUpdateGuardWithData()
{
    if( !m_bActionPosted )
        m_xUndoManager->cancelActionWithUndo( m_xModel );
}

//-----------------------------------------------------------------------------

UndoGuardWithSelection::UndoGuardWithSelection( const rtl::OUString& rUndoString
        , const uno::Reference< chart2::XUndoManager > & xUndoManager
        , const uno::Reference< frame::XModel > & xModel )
        : UndoGuard_Base( rUndoString, xUndoManager, xModel )
{
    Sequence< beans::PropertyValue > aArgs(1);
    aArgs[0] = beans::PropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("WithSelection")), -1, uno::Any(),
        beans::PropertyState_DIRECT_VALUE );
    m_xUndoManager->preActionWithArguments( m_xModel, aArgs );
}

UndoGuardWithSelection::~UndoGuardWithSelection()
{
    if( !m_bActionPosted )
        m_xUndoManager->cancelAction();
}

} //  namespace chart
