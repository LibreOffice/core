/*************************************************************************
 *
 *  $RCSfile: tabsthdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:05 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_PROPERTYHANDLER_TABSTOPTYPES_HXX
#include <tabsthdl.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif

using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontFamilyNamePropHdl
//

XMLTabStopPropHdl::~XMLTabStopPropHdl()
{
    // Nothing to do
}

sal_Bool XMLTabStopPropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    sal_Bool bEqual = sal_False;

    uno::Sequence< style::TabStop> aSeq1;
    if( r1 >>= aSeq1 )
    {
        uno::Sequence< style::TabStop> aSeq2;
        if( r2 >>= aSeq2 )
        {
            if( aSeq1.getLength() == aSeq2.getLength() )
            {
                const style::TabStop* pTabs1 = aSeq1.getConstArray();
                const style::TabStop* pTabs2 = aSeq2.getConstArray();

                int i=0;
                bEqual = sal_True;

                do
                {
                    bEqual = ( pTabs1[i].Position == pTabs2[i].Position       &&
                               pTabs1[i].Alignment == pTabs2[i].Alignment     &&
                               pTabs1[i].DecimalChar == pTabs2[i].DecimalChar &&
                               pTabs1[i].FillChar == pTabs2[i].FillChar );
                    i++;

                } while( bEqual && i < aSeq1.getLength() );
            }
        }
    }

    return bEqual;
}

sal_Bool XMLTabStopPropHdl::importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    return sal_False;
}

sal_Bool XMLTabStopPropHdl::exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    return sal_False;
}

