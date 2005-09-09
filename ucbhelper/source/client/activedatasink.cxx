/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activedatasink.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:32:57 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _UCBHELPER_ACTIVEDATASINK_HXX
#include "ucbhelper/activedatasink.hxx"
#endif

using namespace com::sun::star;

namespace ucb
{

//=========================================================================
//=========================================================================
//
// ActiveDataSink Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( ActiveDataSink,
                   lang::XTypeProvider,
                   io::XActiveDataSink );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ActiveDataSink,
                      lang::XTypeProvider,
                      io::XActiveDataSink );

//=========================================================================
//
// XActiveDataSink methods.
//
//=========================================================================

// virtual
void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

} // namespace ucb
