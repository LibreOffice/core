/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OEnumeration.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:27:19 $
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
#include "OEnumeration.hxx"

using namespace ::com::sun::star;

namespace comphelper
{

OEnumeration::OEnumeration(
    const ::std::vector< uno::Any > & rContainer ) :
        m_aContainer( rContainer ),
        m_aIter( m_aContainer.begin() )
{}

OEnumeration::~OEnumeration()
{}

sal_Bool SAL_CALL OEnumeration::hasMoreElements()
    throw (uno::RuntimeException)
{
    return ! m_aContainer.empty();
}

uno::Any SAL_CALL OEnumeration::nextElement()
    throw (container::NoSuchElementException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    if( m_aIter == m_aContainer.end())
        throw container::NoSuchElementException();

    return *m_aIter++;
}


} //  namespace comphelper
