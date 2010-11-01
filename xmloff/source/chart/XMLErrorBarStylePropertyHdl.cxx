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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLErrorBarStylePropertyHdl.hxx"
#include <unotools/saveopt.hxx>

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using namespace com::sun::star;

XMLErrorBarStylePropertyHdl::XMLErrorBarStylePropertyHdl(  const SvXMLEnumMapEntry* pEnumMap, const ::com::sun::star::uno::Type & rType )
        : XMLEnumPropertyHdl( pEnumMap, rType )
{
}

XMLErrorBarStylePropertyHdl::~XMLErrorBarStylePropertyHdl()
{
}

sal_Bool XMLErrorBarStylePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    uno::Any aValue(rValue);
    const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentVersion < SvtSaveOptions::ODFVER_012 )
    {
        sal_Int32 nValue = 0;
        if(rValue >>= nValue )
        {
            if( nValue == ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR
                || nValue == ::com::sun::star::chart::ErrorBarStyle::FROM_DATA )
            {
                nValue = ::com::sun::star::chart::ErrorBarStyle::NONE;
                aValue = uno::makeAny(nValue);
            }
        }
    }

    return XMLEnumPropertyHdl::exportXML( rStrExpValue, aValue, rUnitConverter );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
