/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:27:54 $
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
#include "DataSeriesHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ----------------------------------------
namespace
{

class lcl_MatchesRole : public ::std::unary_function< bool, Reference< chart2::XDataSequence > >
{
public:
    lcl_MatchesRole( const OUString & aRole ) :
            m_aRole( aRole )
    {}

    bool operator () ( const Reference< chart2::XDataSequence > & xSeq )
    {
        Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
        OUString aRole;
        return ( xProp.is() &&
                 (xProp->getPropertyValue(
                     OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )) ) >>= aRole ) &&
                 m_aRole.equals( aRole ));
    }

private:
    OUString m_aRole;
};

} // anonymous namespace
// ----------------------------------------

namespace chart
{

namespace DataSeriesHelper
{

Reference< chart2::XDataSequence >
    getDataSequenceByRole(
        const Reference< chart2::XDataSource > & xSource, OUString aRole )
{
    Reference< chart2::XDataSequence > aNoResult;
    if( ! xSource.is())
        return aNoResult;

    Sequence< Reference< chart2::XDataSequence > > aSeq( xSource->getDataSequences());

    const Reference< chart2::XDataSequence > * pBegin = aSeq.getConstArray();
    const Reference< chart2::XDataSequence > * pEnd = pBegin + aSeq.getLength();
    const Reference< chart2::XDataSequence > * pMatch =
        ::std::find_if( pBegin, pEnd, lcl_MatchesRole( aRole ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

} //  namespace DataSeriesHelper
} //  namespace chart
