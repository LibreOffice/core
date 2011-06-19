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
#include "precompiled_extensions.hxx"
#include "pcrcommon.hxx"
#include "modulepcr.hxx"
#include "propresid.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/util/MeasureUnit.hpp>
/** === end UNO includes === **/
#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::util;

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    //------------------------------------------------------------------------
    rtl::OString HelpIdUrl::getHelpId( const ::rtl::OUString& _rHelpURL )
    {
        INetURLObject aHID( _rHelpURL );
        if ( aHID.GetProtocol() == INET_PROT_HID )
              return rtl::OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 );
        else
            return rtl::OUStringToOString( _rHelpURL, RTL_TEXTENCODING_UTF8 );
    }

    //------------------------------------------------------------------------
    ::rtl::OUString HelpIdUrl::getHelpURL( const rtl::OString& sHelpId )
    {
        ::rtl::OUStringBuffer aBuffer;
        ::rtl::OUString aTmp( sHelpId, sHelpId.getLength(), RTL_TEXTENCODING_UTF8 );
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INET_PROT_NOT_VALID )
            aBuffer.appendAscii( INET_HID_SCHEME );
        aBuffer.append( aTmp.getStr() );
        return aBuffer.makeStringAndClear();
    }
//............................................................................
} // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
