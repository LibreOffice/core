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
#include "XMLTextOrientationHdl.hxx"

#include "xmluconv.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

XMLTextOrientationHdl::~XMLTextOrientationHdl()
{
}

sal_Bool XMLTextOrientationHdl::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval( sal_False );

    if( IsXMLToken( rStrImpValue, XML_LTR ))
    {
        rValue <<= static_cast< sal_Bool >( sal_False );
        bRetval = sal_True;
    }
    else if( IsXMLToken( rStrImpValue, XML_TTB ))
    {
        rValue <<= static_cast< sal_Bool >( sal_True );
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XMLTextOrientationHdl::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bVal;
    sal_Bool bRetval( sal_False );

    if( rValue >>= bVal )
    {
        if( bVal )
            rStrExpValue = GetXMLToken( XML_TTB );
        else
            rStrExpValue = GetXMLToken( XML_LTR );
        bRetval = sal_True;
    }

    return bRetval;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
