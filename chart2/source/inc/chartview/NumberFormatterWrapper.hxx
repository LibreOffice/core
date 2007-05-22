/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NumberFormatterWrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:26:48 $
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

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
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
    NumberFormatterWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xSupplier );
    virtual ~NumberFormatterWrapper();

    SvNumberFormatter* getSvNumberFormatter() const;
    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                getNumberFormatsSupplier() { return m_xNumberFormatsSupplier; };

    rtl::OUString getFormattedString( sal_Int32 nNumberFormatKey, double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const;

private: //private member
    ::com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier >
                        m_xNumberFormatsSupplier;

    SvNumberFormatter* m_pNumberFormatter;
};


class FixedNumberFormatter
{
public:
    FixedNumberFormatter( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xSupplier
        , sal_Int32 nNumberFormatKey );
    virtual ~FixedNumberFormatter();

    rtl::OUString getFormattedString( double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const;

private:
    NumberFormatterWrapper      m_aNumberFormatterWrapper;
    ULONG                       m_nNumberFormatKey;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
