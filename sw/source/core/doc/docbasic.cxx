/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>

#include <rtl/ustring.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>
#include <basic/sbx.hxx>
#include <frmfmt.hxx>
#include <fmtinfmt.hxx>
#include <fmturl.hxx>
#include <frmatr.hxx>
#include <docary.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <swevent.hxx>

using namespace ::com::sun::star::uno;

static Sequence<Any> *lcl_docbasic_convertArgs( SbxArray& rArgs )
{
    Sequence<Any> *pRet = 0;

    sal_uInt16 nCount = rArgs.Count();
    if( nCount > 1 )
    {
        nCount--;
        pRet = new Sequence<Any>( nCount );
        Any *pUnoArgs = pRet->getArray();
        for( sal_uInt16 i=0; i<nCount; i++ )
        {
            SbxVariable *pVar = rArgs.Get( i+1 );
            switch( pVar->GetType() )
            {
            case SbxSTRING:
                pUnoArgs[i] <<= OUString( pVar->GetOUString() );
                break;
            case SbxCHAR:
                pUnoArgs[i] <<= (sal_Int16)pVar->GetChar() ;
                break;
            case SbxUSHORT:
                pUnoArgs[i] <<= (sal_Int16)pVar->GetUShort();
                break;
            case SbxLONG:
                pUnoArgs[i] <<= (sal_Int32)pVar->GetLong();
                break;
            default:
                pUnoArgs[i].setValue(0, ::getVoidCppuType());
                break;
            }
        }
    }

    return pRet;
}

bool SwDoc::ExecMacro( const SvxMacro& rMacro, String* pRet, SbxArray* pArgs )
{
    ErrCode eErr = 0;
    switch( rMacro.GetScriptType() )
    {
    case STARBASIC:
        {
            SbxBaseRef aRef;
            SbxValue* pRetValue = new SbxValue;
            aRef = pRetValue;
            eErr = mpDocShell->CallBasic( rMacro.GetMacName(),
                                         rMacro.GetLibName(),
                                         pArgs, pRet ? pRetValue : 0 );

            if( pRet && SbxNULL <  pRetValue->GetType() &&
                        SbxVOID != pRetValue->GetType() )
            {
                // valid value, so set it
                *pRet = pRetValue->GetOUString();
            }
        }
        break;
    case JAVASCRIPT:
        // ignore JavaScript calls
        break;
    case EXTENDED_STYPE:
        {
            Sequence<Any> *pUnoArgs = 0;
            if( pArgs )
            {
                // better to rename the local function to lcl_translateBasic2Uno and
                // a much shorter routine can be found in sfx2/source/doc/objmisc.cxx
                pUnoArgs = lcl_docbasic_convertArgs( *pArgs );
            }

            if (!pUnoArgs)
            {
                pUnoArgs = new Sequence< Any > (0);
            }

            // TODO - return value is not handled
            Any aRet;
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;

            OSL_TRACE( "SwDoc::ExecMacro URL is %s", OUStringToOString( rMacro.GetMacName(),
                RTL_TEXTENCODING_UTF8).getStr() );

            eErr = mpDocShell->CallXScript(
                rMacro.GetMacName(), *pUnoArgs, aRet, aOutArgsIndex, aOutArgs);

            delete pUnoArgs;
            break;
        }
    }

    return 0 == eErr;
}

sal_uInt16 SwDoc::CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                    bool bCheckPtr, SbxArray* pArgs, const Link* )
{
    if( !mpDocShell )        // we can't do that without a DocShell!
        return 0;

    sal_uInt16 nRet = 0;
    const SvxMacroTableDtor* pTbl = 0;
    switch( rCallEvent.eType )
    {
    case EVENT_OBJECT_INETATTR:
        if( bCheckPtr  )
        {
            const SfxPoolItem* pItem;
            sal_uInt32 n, nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
            for( n = 0; n < nMaxItems; ++n )
                if( 0 != (pItem = GetAttrPool().GetItem2( RES_TXTATR_INETFMT, n ) )
                    && rCallEvent.PTR.pINetAttr == pItem )
                {
                    bCheckPtr = false;       // misuse as a flag
                    break;
                }
        }
        if( !bCheckPtr )
            pTbl = rCallEvent.PTR.pINetAttr->GetMacroTbl();
        break;

    case EVENT_OBJECT_URLITEM:
    case EVENT_OBJECT_IMAGE:
        {
            const SwFrmFmt* pFmt = (SwFrmFmt*)rCallEvent.PTR.pFmt;
            if( bCheckPtr )
            {
                if ( GetSpzFrmFmts()->Contains( pFmt ) )
                    bCheckPtr = false;      // misuse as a flag
            }
            if( !bCheckPtr )
                pTbl = &pFmt->GetMacro().GetMacroTable();
        }
        break;

    case EVENT_OBJECT_IMAGEMAP:
        {
            const IMapObject* pIMapObj = rCallEvent.PTR.IMAP.pIMapObj;
            if( bCheckPtr )
            {
                const SwFrmFmt* pFmt = (SwFrmFmt*)rCallEvent.PTR.IMAP.pFmt;
                const ImageMap* pIMap;
                if( GetSpzFrmFmts()->Contains( pFmt ) &&
                    0 != (pIMap = pFmt->GetURL().GetMap()) )
                {
                    for( sal_uInt16 nPos = pIMap->GetIMapObjectCount(); nPos; )
                        if( pIMapObj == pIMap->GetIMapObject( --nPos ))
                        {
                            bCheckPtr = false;      // misuse as a flag
                            break;
                        }
                }
            }
            if( !bCheckPtr )
                pTbl = &pIMapObj->GetMacroTable();
        }
        break;
    default:
        break;
    }

    if( pTbl )
    {
        nRet = 0x1;
        if( pTbl->IsKeyValid( nEvent ) )
        {
            const SvxMacro& rMacro = *pTbl->Get( nEvent );
            if( STARBASIC == rMacro.GetScriptType() )
            {
                nRet += 0 == mpDocShell->CallBasic( rMacro.GetMacName(),
                                    rMacro.GetLibName(), pArgs ) ? 1 : 0;
            }
            else if( EXTENDED_STYPE == rMacro.GetScriptType() )
            {
                Sequence<Any> *pUnoArgs = 0;

                if( pArgs )
                {
                    pUnoArgs = lcl_docbasic_convertArgs( *pArgs );
                }

                if (!pUnoArgs)
                {
                    pUnoArgs = new Sequence <Any> (0);
                }

                Any aRet;
                Sequence< sal_Int16 > aOutArgsIndex;
                Sequence< Any > aOutArgs;

                OSL_TRACE( "SwDoc::CallEvent URL is %s", OUStringToOString(
                    rMacro.GetMacName(), RTL_TEXTENCODING_UTF8).getStr() );

                nRet += 0 == mpDocShell->CallXScript(
                    rMacro.GetMacName(), *pUnoArgs,aRet, aOutArgsIndex, aOutArgs) ? 1 : 0;

                delete pUnoArgs;
            }
            // JavaScript calls are ignored
        }
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
