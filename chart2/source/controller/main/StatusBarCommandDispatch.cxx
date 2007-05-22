/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatusBarCommandDispatch.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:09:28 $
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

#include "StatusBarCommandDispatch.hxx"
#include "ObjectNameProvider.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

// #ifndef _VOS_MUTEX_HXX_
// #include <vos/mutex.hxx>
// #endif
// #ifndef _SV_SVAPP_HXX
// #include <vcl/svapp.hxx>
// #endif

// for ressource strings STR_UNDO and STR_REDO
#include <sfx2/sfx.hrc>

#include "ResId.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

StatusBarCommandDispatch::StatusBarCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XModel > & xModel,
    const Reference< view::XSelectionSupplier > & xSelSupp ) :
        impl::StatusBarCommandDispatch_Base( xContext ),
        m_xModifiable( xModel, uno::UNO_QUERY  ),
        m_xSelectionSupplier( xSelSupp ),
        m_bIsModified( false )
{}

StatusBarCommandDispatch::~StatusBarCommandDispatch()
{}

void StatusBarCommandDispatch::initialize()
{
    if( m_xModifiable.is())
    {
        Reference< util::XModifyBroadcaster > xModifyBroadcaster( m_xModifiable, uno::UNO_QUERY );
        OSL_ASSERT( xModifyBroadcaster.is());
        if( xModifyBroadcaster.is())
            xModifyBroadcaster->addModifyListener( this );
    }

    if( m_xSelectionSupplier.is())
    {
        m_xSelectionSupplier->addSelectionChangeListener( this );
    }
}

void StatusBarCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    bool bFireAll(rURL.getLength() == 0);
    bool bFireContext(  bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Context")));
    bool bFireModified( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:ModifiedStatus")));

    if( bFireContext )
    {
        uno::Any aArg;
        Reference< chart2::XChartDocument > xDoc( m_xModifiable, uno::UNO_QUERY );
        aArg <<= ObjectNameProvider::getSelectedObjectText( m_aSelectedCID, xDoc );
        fireStatusEventForURL( C2U(".uno:Context"), aArg, true, xSingleListener );
    }
    if( bFireModified )
    {
        uno::Any aArg;
        if( m_bIsModified )
            aArg <<= C2U("*");
        fireStatusEventForURL( C2U(".uno:ModifiedStatus"), aArg, true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL StatusBarCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    // nothing to do here
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL StatusBarCommandDispatch::disposing()
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StatusBarCommandDispatch::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL StatusBarCommandDispatch::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if( m_xModifiable.is())
        m_bIsModified = m_xModifiable->isModified();

    CommandDispatch::modified( aEvent );
}

// ____ XSelectionChangeListener ____
void SAL_CALL StatusBarCommandDispatch::selectionChanged( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if( m_xSelectionSupplier.is())
        m_xSelectionSupplier->getSelection() >>= m_aSelectedCID;
    else
        m_aSelectedCID = OUString();
    fireAllStatusEvents( 0 );
}

} //  namespace chart
