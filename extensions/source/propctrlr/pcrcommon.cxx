/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "pcrcommon.hxx"
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_PROPRESID_HRC
#include "propresid.hrc"
#endif

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
        ::rtl::OUString aTmp( rtl::OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ));
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INET_PROT_NOT_VALID )
            aBuffer.appendAscii( INET_HID_SCHEME );
        aBuffer.append( aTmp.getStr() );
        return aBuffer.makeStringAndClear();
    }
//............................................................................
} // namespace pcr
//............................................................................

