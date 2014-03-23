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
#include "precompiled_sfx2.hxx"
#include <tools/stream.hxx>
#include <tools/cachestr.hxx>

#include <sot/storage.hxx>
#include <sot/formats.hxx>

#include <sfx2/mieclip.hxx>
#include <sfx2/sfxuno.hxx>

MSE40HTMLClipFormatObj::~MSE40HTMLClipFormatObj()
{
    delete pStrm;
}

SvStream* MSE40HTMLClipFormatObj::IsValid( SvStream& rStream )
{
    sal_Bool bRet = sal_False;
    if( pStrm )
        delete pStrm, pStrm = 0;

    ByteString sLine, sVersion;
    sal_uIntPtr nStt = 0, nEnd = 0;
    sal_uInt16 nIndex = 0;

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ResetError();

    if( rStream.ReadLine( sLine ) &&
        sLine.GetToken( 0, ':', nIndex ) == "Version" )
    {
        sVersion = sLine.Copy( nIndex );
        while( rStream.ReadLine( sLine ) )
        {
            nIndex = 0;
            ByteString sTmp( sLine.GetToken( 0, ':', nIndex ) );
            if( sTmp == "StartHTML" )
                nStt = (sal_uIntPtr)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "EndHTML" )
                nEnd = (sal_uIntPtr)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "SourceURL" )
                sBaseURL = String( sLine.Erase( 0, nIndex ), RTL_TEXTENCODING_UTF8);

            if( nEnd && nStt &&
                ( sBaseURL.Len() || rStream.Tell() >= nStt ))
            {
                bRet = sal_True;
                break;
            }
        }
    }

    if( bRet )
    {
        rStream.Seek( nStt );

        pStrm = new SvCacheStream( ( nEnd - nStt < 0x10000l
                                        ? nEnd - nStt + 32
                                        : 0 ));
        *pStrm << rStream;
        pStrm->SetStreamSize( nEnd - nStt + 1L );
        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
    }

    return pStrm;
}

