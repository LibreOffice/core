/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modsizeexceeded.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 15:06:18 $
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
#include "precompiled_basic.hxx"
#include "modsizeexceeded.hxx"

#include <framework/interaction.hxx>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

using namespace com::sun::star;
using namespace cppu;
using namespace rtl;
using namespace osl;

ModuleSizeExceeded::ModuleSizeExceeded( const uno::Sequence< ::rtl::OUString >& sModules )
{
    script::ModuleSizeExceededRequest aReq;
    aReq.Names = sModules;

    m_aRequest <<= aReq;

    m_xAbort.set( uno::Reference< task::XInteractionAbort >(new framework::ContinuationAbort), uno::UNO_QUERY );
    m_xApprove.set( uno::Reference< task::XInteractionApprove >(new framework::ContinuationApprove ), uno::UNO_QUERY );
    m_lContinuations.realloc( 2 );
    m_lContinuations[0] =  m_xApprove;
    m_lContinuations[1] = m_xAbort;
}

sal_Bool
ModuleSizeExceeded::isAbort() const
{
    framework::ContinuationAbort* pBase = static_cast< framework::ContinuationAbort* >( m_xAbort.get() );
    return pBase->isSelected();
}

sal_Bool
ModuleSizeExceeded::isApprove() const
{
    framework::ContinuationApprove* pBase = static_cast< framework::ContinuationApprove* >( m_xApprove.get() );
    return pBase->isSelected();
}


