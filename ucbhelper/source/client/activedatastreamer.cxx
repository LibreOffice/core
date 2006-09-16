/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activedatastreamer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:18:57 $
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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _UCBHELPER_ACTIVEDATASTREAMER_HXX
#include "ucbhelper/activedatastreamer.hxx"
#endif

using namespace com::sun::star;

namespace ucb
{

//=========================================================================
//=========================================================================
//
// ActiveDataStreamer Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( ActiveDataStreamer,
                   lang::XTypeProvider,
                   io::XActiveDataStreamer );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ActiveDataStreamer,
                      lang::XTypeProvider,
                      io::XActiveDataStreamer );

//=========================================================================
//
// XActiveDataStreamer methods.
//
//=========================================================================

// virtual
void SAL_CALL ActiveDataStreamer::setStream( const uno::Reference< io::XStream >& xStream )
    throw( uno::RuntimeException )
{
    m_xStream = xStream;
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL ActiveDataStreamer::getStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

} // namespace ucb
