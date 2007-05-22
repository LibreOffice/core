/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoCommandDispatch.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:09:48 $
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

#include "UndoCommandDispatch.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

// for ressource strings STR_UNDO and STR_REDO
#include <sfx2/sfx.hrc>

#include "ResId.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

UndoCommandDispatch::UndoCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    UndoManager * pUndoManager,
    const Reference< frame::XModel > & xModel ) :
        CommandDispatch( xContext ),
        m_pUndoManager( pUndoManager ),
        m_xModel( xModel )
{}

UndoCommandDispatch::~UndoCommandDispatch()
{}

void UndoCommandDispatch::initialize()
{
    if( m_pUndoManager )
    {
        m_pUndoManager->addModifyListener( this );
    }
}

void UndoCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    if( m_pUndoManager )
    {
        bool bFireAll = (rURL.getLength() == 0);
        uno::Any aUndoState, aRedoState;
        if( m_pUndoManager->canUndo())
        {
            OUString aUndo( String( SchResId( STR_UNDO )));
            aUndoState <<= ( aUndo + m_pUndoManager->getCurrentUndoString());
        }
        if( m_pUndoManager->canRedo())
        {
            OUString aRedo( String( SchResId( STR_REDO )));
            aRedoState <<= ( aRedo + m_pUndoManager->getCurrentRedoString());
        }

        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Undo")))
            fireStatusEventForURL( C2U(".uno:Undo"), aUndoState, m_pUndoManager->canUndo(), xSingleListener );
        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Redo")))
            fireStatusEventForURL( C2U(".uno:Redo"), aRedoState, m_pUndoManager->canRedo(), xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL UndoCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    if( m_pUndoManager )
    {
        // why is it necessary to lock the solar mutex here?
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        if( URL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Undo" )))
            m_pUndoManager->undo( m_xModel );
        else
            m_pUndoManager->redo( m_xModel );
        // \--
    }
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL UndoCommandDispatch::disposing()
{
    if( m_pUndoManager )
    {
        m_pUndoManager->removeModifyListener( this );
        // not owner
        m_pUndoManager = 0;
    }

    m_xModel.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL UndoCommandDispatch::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    m_pUndoManager = 0;
    m_xModel.clear();
}

} //  namespace chart
