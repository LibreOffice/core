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
#include "precompiled_sc.hxx"
#include "lotimpop.hxx"

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "scerrors.hxx"
#include "root.hxx"
#include "filtopt.hxx"
#include "ftools.hxx"

//------------------------------------------------------------------------

extern FltError ScImportLotus123old( SvStream&, ScDocument*, CharSet eSrc );
        // alter Krempel in filter.cxx!

FltError ScFormatFilterPluginImpl::ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, CharSet eSrc )
{
        ScFilterOptions aFilterOpt;
    sal_Bool bWithWK3 = aFilterOpt.GetWK3Flag();

    SvStream*           pStream = rMedium.GetInStream();

    if( !pStream )
        return eERR_OPEN;

    FltError            eRet;

    pStream->Seek( 0UL );

    pStream->SetBufferSize( 32768 );

    ImportLotus         aLotusImport( *pStream, pDocument, eSrc );

    if( bWithWK3 )
        eRet = aLotusImport.Read();
    else
        eRet = 0xFFFFFFFF;  // WK1 /WKS erzwingen

    // ACHTUNG: QUICK-HACK fuer WK1 / WKS  <->  WK3 / WK4
    if( eRet == 0xFFFFFFFF )
    {
        pStream->Seek( 0UL );

        pStream->SetBufferSize( 32768 );

        eRet = ScImportLotus123old( *pStream, pDocument, eSrc );

        pStream->SetBufferSize( 0 );

        return eRet;
    }

    if( eRet != eERR_OK )
        return eRet;

    if( pLotusRoot->eFirstType == Lotus_WK3 )
    {// versuchen *.FM3-File zu laden
        INetURLObject aURL( rMedium.GetURLObject() );
        aURL.setExtension( CREATE_STRING( "FM3" ) );
        SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_STD_READ, sal_True );
        pStream = aMedium.GetInStream();
        if ( pStream )
        {
            if( aLotusImport.Read( *pStream ) != eERR_OK )
                eRet = SCWARN_IMPORT_WRONG_FM3;
        }
        else
            eRet = SCWARN_IMPORT_OPEN_FM3;
    }

    return eRet;
}

