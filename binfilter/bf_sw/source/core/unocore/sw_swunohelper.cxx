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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define _SVSTDARR_STRINGS

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hdl>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <tools/debug.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>
#include <bf_svtools/svstdarr.hxx>

#include <swunohelper.hxx>
#include <swunodef.hxx>
#include <errhdl.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {
namespace SWUnoHelper {

sal_Int32 GetEnumAsInt32( const UNO_NMSPC::Any& rVal )
{
    sal_Int32 eVal;
    try
    {
        eVal = comphelper::getEnumAsINT32( rVal );
    }
    catch( UNO_NMSPC::Exception & )
    {
        eVal = 0;
        ASSERT( FALSE, "can't get EnumAsInt32" );
    }
    return eVal;
}


// methods for UCB actions
BOOL UCB_DeleteFile( const String& rURL )
{
    BOOL bRemoved;
    try
    {
        ucbhelper::Content aTempContent( rURL,
                                STAR_REFERENCE( ucb::XCommandEnvironment )());
        aTempContent.executeCommand(
                        ::rtl::OUString::createFromAscii( "delete" ),
                        UNO_NMSPC::makeAny( sal_Bool( sal_True ) ) );
        bRemoved = TRUE;
    }
    catch( UNO_NMSPC::Exception& )
    {
        bRemoved = FALSE;
        ASSERT( FALSE, "Exeception from executeCommand( delete )" );
    }
    return bRemoved;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
