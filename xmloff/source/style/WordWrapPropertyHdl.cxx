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
#include <xmloff/xmlimp.hxx>
#include <xmloff/WordWrapPropertyHdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <comphelper/extract.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLWordWrapPropertyHdl
//

XMLWordWrapPropertyHdl::XMLWordWrapPropertyHdl( SvXMLImport* pImport )
: mpImport( pImport )
{
}

XMLWordWrapPropertyHdl::~XMLWordWrapPropertyHdl()
{
    // Nothing to do
}

sal_Bool XMLWordWrapPropertyHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bValue = sal_False, bRetValue = sal_False;
    if( rStrImpValue == GetXMLToken( xmloff::token::XML_WRAP ) )
    {
        bValue = sal_True;
        bRetValue = sal_True;
    }
    if( rStrImpValue == GetXMLToken( xmloff::token::XML_NO_WRAP ) )
    {
        bValue = sal_False;
        bRetValue = sal_True;
    }
    if ( bRetValue && mpImport )
    {
        sal_Int32 nUPD, nBuildId;
        if( mpImport->getBuildIds( nUPD, nBuildId ) )
        {
            if( nUPD == 300 )
            {
                if( ( nBuildId > 0 ) && (nBuildId < 9316 ) )
                    bValue = bValue ? sal_False : sal_True;     // treat OOo 3.0 beta1 as OOo 2.x
            }
            else if( ( nUPD == 680 ) || ( nUPD >= 640 && nUPD <= 645 ) )
                bValue = bValue ? sal_False : sal_True;
        }
        rValue <<= bValue;
    }
    return bRetValue;
}

sal_Bool XMLWordWrapPropertyHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    if( ::cppu::any2bool( rValue ) )
    {
        rStrExpValue = GetXMLToken( xmloff::token::XML_WRAP );
    }
    else
    {
        rStrExpValue = GetXMLToken( xmloff::token::XML_NO_WRAP );
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
