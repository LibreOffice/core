/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedSeriesAreaOrLineProperty.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:23:21 $
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

#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "DataSeriesPointWrapper.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
namespace wrapper
{

WrappedSeriesAreaOrLineProperty::WrappedSeriesAreaOrLineProperty(
      const rtl::OUString& rOuterName
    , const rtl::OUString& rInnerAreaTypeName
    , const rtl::OUString& rInnerLineTypeName
    , DataSeriesPointWrapper* pDataSeriesPointWrapper )
    : WrappedProperty( rOuterName, C2U( "" ) )
    , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
    , m_aInnerAreaTypeName( rInnerAreaTypeName )
    , m_aInnerLineTypeName( rInnerLineTypeName )
{
}
WrappedSeriesAreaOrLineProperty::~WrappedSeriesAreaOrLineProperty()
{
}

//virtual
::rtl::OUString WrappedSeriesAreaOrLineProperty::getInnerName() const
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aInnerLineTypeName;
    return m_aInnerAreaTypeName;
}

} //namespace wrapper
} //namespace chart
//.............................................................................
