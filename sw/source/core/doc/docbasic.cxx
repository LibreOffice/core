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
#include <sal/log.hxx>
#include <vcl/imap.hxx>
#include <vcl/imapobj.hxx>
#include <basic/sbx.hxx>
#include <frmfmt.hxx>
#include <fmtinfmt.hxx>
#include <fmturl.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <swevent.hxx>
#include <frameformats.hxx>
#include <memory>

using namespace ::com::sun::star::uno;

static Sequence<Any> *lcl_docbasic_convertArgs( SbxArray& rArgs )
{
    Sequence<Any> *pRet = nullptr;

    sal_uInt32 nCount = rArgs.Count();
    if( nCount > 1 )
    {
        nCount--;
        pRet = new Sequence<Any>( nCount );
        Any *pUnoArgs = pRet->getArray();
        for( sal_uInt32 i=0; i<nCount; i++ )
        {
            SbxVariable* pVar = rArgs.Get(i + 1);
            switch( pVar->GetType() )
            {
            case SbxSTRING:
                pUnoArgs[i] <<= pVar->GetOUString();
                break;
            case SbxCHAR:
                pUnoArgs[i] <<= static_cast<sal_Int16>(pVar->GetChar()) ;
                break;
            case SbxUSHORT:
                pUnoArgs[i] <<= static_cast<sal_Int16>(pVar->GetUShort());
                break;
            case SbxLONG:
                pUnoArgs[i] <<= pVar->GetLong();
                break;
            default:
                pUnoArgs[i].clear();
                break;
            }
        }
    }

    return pRet;
}

void SwDoc::ExecMacro( const SvxMacro& rMacro, OUString* pRet, SbxArray* pArgs )
{
    switch( rMacro.GetScriptType() )
    {
    case STARBASIC:
        {
            SbxBaseRef aRef;
            SbxValue* pRetValue = new SbxValue;
            aRef = pRetValue;
            mpDocShell->CallBasic( rMacro.GetMacName(),
                                         rMacro.GetLibName(),
                                         pArgs, pRet ? pRetValue : nullptr );

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
            std::unique_ptr<Sequence<Any> > pUnoArgs;
            if( pArgs )
            {
                // better to rename the local function to lcl_translateBasic2Uno and
                // a much shorter routine can be found in sfx2/source/doc/objmisc.cxx
                pUnoArgs.reset(lcl_docbasic_convertArgs( *pArgs ));
            }

            if (!pUnoArgs)
            {
                pUnoArgs.reset(new Sequence< Any > (0));
            }

            // TODO - return value is not handled
            Any aRet;
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;

            SAL_INFO("sw", "SwDoc::ExecMacro URL is " << rMacro.GetMacName() );

            mpDocShell->CallXScript(
                rMacro.GetMacName(), *pUnoArgs, aRet, aOutArgsIndex, aOutArgs);

            break;
        }
    }
}

sal_uInt16 SwDoc::CallEvent( SvMacroItemId nEvent, const SwCallMouseEvent& rCallEvent,
                    bool bCheckPtr )
{
    if( !mpDocShell )        // we can't do that without a DocShell!
        return 0;

    sal_uInt16 nRet = 0;
    const SvxMacroTableDtor* pTable = nullptr;
    switch( rCallEvent.eType )
    {
    case EVENT_OBJECT_INETATTR:
        if( bCheckPtr  )
        {
            for (const SfxPoolItem* pItem : GetAttrPool().GetItemSurrogates(RES_TXTATR_INETFMT))
            {
                auto pFormatItem = dynamic_cast<const SwFormatINetFormat*>(pItem);
                if( pFormatItem && rCallEvent.PTR.pINetAttr == pFormatItem )
                {
                    bCheckPtr = false;       // misuse as a flag
                    break;
                }
            }
        }
        if( !bCheckPtr )
            pTable = rCallEvent.PTR.pINetAttr->GetMacroTable();
        break;

    case EVENT_OBJECT_URLITEM:
    case EVENT_OBJECT_IMAGE:
        {
            const SwFrameFormat* pFormat = rCallEvent.PTR.pFormat;
            if( bCheckPtr )
            {
                if (GetSpzFrameFormats()->IsAlive(pFormat))
                    bCheckPtr = false;      // misuse as a flag
                else
                    // this shouldn't be possible now that SwCallMouseEvent
                    // listens for dying format?
                    assert(false);
            }
            if( !bCheckPtr )
                pTable = &pFormat->GetMacro().GetMacroTable();
        }
        break;

    case EVENT_OBJECT_IMAGEMAP:
        {
            const IMapObject* pIMapObj = rCallEvent.PTR.IMAP.pIMapObj;
            if( bCheckPtr )
            {
                const SwFrameFormat* pFormat = rCallEvent.PTR.IMAP.pFormat;
                if (GetSpzFrameFormats()->IsAlive(pFormat))
                {
                    const ImageMap* pIMap = pFormat->GetURL().GetMap();
                    if (pIMap)
                    {
                        for( size_t nPos = pIMap->GetIMapObjectCount(); nPos; )
                            if( pIMapObj == pIMap->GetIMapObject( --nPos ))
                            {
                                bCheckPtr = false;      // misuse as a flag
                                break;
                            }
                    }
                }
            }
            if( !bCheckPtr )
                pTable = &pIMapObj->GetMacroTable();
        }
        break;
    default:
        break;
    }

    if( pTable )
    {
        nRet = 0x1;
        if( pTable->IsKeyValid( nEvent ) )
        {
            const SvxMacro& rMacro = *pTable->Get( nEvent );
            if( STARBASIC == rMacro.GetScriptType() )
            {
                nRet += ERRCODE_NONE == mpDocShell->CallBasic( rMacro.GetMacName(),
                                    rMacro.GetLibName(), nullptr ) ? 1 : 0;
            }
            else if( EXTENDED_STYPE == rMacro.GetScriptType() )
            {
                std::unique_ptr<Sequence<Any> > pUnoArgs(new Sequence<Any>());

                Any aRet;
                Sequence< sal_Int16 > aOutArgsIndex;
                Sequence< Any > aOutArgs;

                SAL_INFO("sw", "SwDoc::CallEvent URL is " << rMacro.GetMacName() );

                nRet += ERRCODE_NONE == mpDocShell->CallXScript(
                    rMacro.GetMacName(), *pUnoArgs,aRet, aOutArgsIndex, aOutArgs) ? 1 : 0;
            }
            // JavaScript calls are ignored
        }
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
