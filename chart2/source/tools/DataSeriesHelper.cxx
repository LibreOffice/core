/*************************************************************************
 *
 *  $RCSfile: DataSeriesHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2004-01-26 09:01:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
