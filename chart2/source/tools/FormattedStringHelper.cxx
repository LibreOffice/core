/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormattedStringHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:58:56 $
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

#include "FormattedStringHelper.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using rtl::OUString;

Sequence< Reference< chart2::XFormattedString > >
            FormattedStringHelper::createFormattedStringSequence(
                     const Reference< uno::XComponentContext > & xContext
                    , const OUString & rString
                    , const Reference< beans::XPropertySet > & xTextProperties ) throw()
{
    Reference< XFormattedString > xFormStr;
    try
    {
        if( xContext.is() )
        {
            xFormStr.set(
                xContext->getServiceManager()->createInstanceWithContext(
                    C2U("com.sun.star.chart2.FormattedString"), xContext ),
                uno::UNO_QUERY_THROW );

            xFormStr->setString( rString );

            // set character properties
            PropertyHelper::copyProperties(
                xTextProperties, Reference< beans::XPropertySet >( xFormStr, uno::UNO_QUERY ) );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return Sequence< Reference< XFormattedString > >( & xFormStr, 1 );
}

//.............................................................................
} //namespace chart
//.............................................................................

