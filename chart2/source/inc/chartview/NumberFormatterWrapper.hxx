/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_VIEW_NUMBERFORMATTERWRAPPER_HXX
#define _CHART2_VIEW_NUMBERFORMATTERWRAPPER_HXX

#include <svl/zforlist.hxx>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include "chartviewdllapi.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class FixedNumberFormatter;

class OOO_DLLPUBLIC_CHARTVIEW NumberFormatterWrapper
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
    ::com::sun::star::uno::Any m_aNullDate;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
