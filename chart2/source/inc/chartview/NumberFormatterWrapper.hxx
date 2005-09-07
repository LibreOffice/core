/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NumberFormatterWrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:49:15 $
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
#ifndef _CHART2_VIEW_NUMBERFORMATTERWRAPPER_HXX
#define _CHART2_VIEW_NUMBERFORMATTERWRAPPER_HXX

#ifndef _COM_SUN_STAR_CHART2_NUMBERFORMAT_HPP_
#include <com/sun/star/chart2/NumberFormat.hpp>
#endif

//only needed for temp numberformatter
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class FixedNumberFormatter;

class NumberFormatterWrapper
{
public:
    NumberFormatterWrapper();
    virtual ~NumberFormatterWrapper();

    ::com::sun::star::chart2::NumberFormat    getNumberFormatForKey( sal_Int32 nIndex ) const;
    sal_Int32  getKeyForNumberFormat( const ::com::sun::star::chart2::NumberFormat& rNumberFormat ) const;

    //this method should only be used by the ViewElementListProvider
    SvNumberFormatter* getSvNumberFormatter() const;

private: //private member
    SvNumberFormatter* m_pNumberFormatter;

friend class FixedNumberFormatter;
};


class FixedNumberFormatter
{
public:
    FixedNumberFormatter( NumberFormatterWrapper* pNumberFormatterWrapper
        , const ::com::sun::star::chart2::NumberFormat& rNumberFormat );
    virtual ~FixedNumberFormatter();

    rtl::OUString getFormattedString( double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const;

private:
    NumberFormatterWrapper*     m_pNumberFormatterWrapper;
    ULONG                       m_nFormatIndex;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
