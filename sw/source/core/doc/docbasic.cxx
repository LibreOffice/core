/*************************************************************************
 *
 *  $RCSfile: docbasic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-13 10:49:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _RTL_USTRING_HXX //autogen
#include <rtl/ustring.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
#endif
#ifndef _SBXCORE_HXX
#include <svtools/sbxcore.hxx>
#endif
#ifndef __SBX_SBXVALUE
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SVCONFIG_HXX
#include <svtools/config.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::rtl;

Sequence<Any> *lcl_docbasic_convertArgs( SbxArray& rArgs )
{
    Sequence<Any> *pRet = 0;

#if 0
    switch( nEvent )
    {
    case SW_EVENT_FRM_KEYINPUT_ALPHA:
    case SW_EVENT_FRM_KEYINPUT_NOALPHA:
    case SW_EVENT_FRM_MOVE:
    case SW_EVENT_FRM_RESIZE:
    }
#endif

    USHORT nCount = rArgs.Count();
    if( nCount > 1 )
    {
        nCount--;
        pRet = new Sequence<Any>( nCount );
        Any *pUnoArgs = pRet->getArray();
        for( USHORT i=0; i<nCount; i++ )
        {
            SbxVariable *pVar = rArgs.Get( i+1 );
            switch( pVar->GetType() )
            {
            case SbxSTRING:
                pUnoArgs[i] <<= OUString( pVar->GetString() );
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

BOOL SwDoc::ExecMacro( const SvxMacro& rMacro, String* pRet, SbxArray* pArgs )
{
    ErrCode eErr = 0;
    switch( rMacro.GetScriptType() )
    {
    case STARBASIC:
        {
            SbxBaseRef aRef;
            SbxValue* pRetValue = new SbxValue;
            aRef = pRetValue;
            eErr = pDocShell->CallBasic( rMacro.GetMacName(),
                                         rMacro.GetLibName(),
                                         0, pArgs, pRet ? pRetValue : 0 );

            if( pRet && SbxNULL <  pRetValue->GetType() &&
                        SbxVOID != pRetValue->GetType() )
                // gueltiger Wert, also setzen
                *pRet = pRetValue->GetString();
        }
        break;
    case JAVASCRIPT:
        // ignore JavaScript calls
        break;
    case EXTENDED_STYPE:
        {
            Sequence<Any> *pUnoArgs = 0;
            Reference<XInterface> xThis;
            if( pArgs )
                pUnoArgs = lcl_docbasic_convertArgs( *pArgs );
            eErr = pDocShell->CallScript( rMacro.GetLanguage(),
                                          rMacro.GetMacName(),
                                          xThis,
                                          pUnoArgs );
            delete pUnoArgs;
        }
        break;
    }

    return 0 == eErr;
}



USHORT SwDoc::CallEvent( USHORT nEvent, const SwCallMouseEvent& rCallEvent,
                    BOOL bCheckPtr, SbxArray* pArgs, const Link* pCallBack )
{
    if( !pDocShell )        // ohne DocShell geht das nicht!
        return 0;

    USHORT nRet = 0;
    const SvxMacroTableDtor* pTbl = 0;
    switch( rCallEvent.eType )
    {
    case EVENT_OBJECT_INETATTR:
        if( bCheckPtr  )
        {
            const SfxPoolItem* pItem;
            USHORT n, nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_INETFMT );
            for( n = 0; n < nMaxItems; ++n )
                if( 0 != (pItem = GetAttrPool().GetItem( RES_TXTATR_INETFMT, n ) )
                    && rCallEvent.PTR.pINetAttr == pItem )
                {
                    bCheckPtr = FALSE;      // als Flag missbrauchen
                    break;
                }
        }
        if( !bCheckPtr )
            pTbl = rCallEvent.PTR.pINetAttr->GetMacroTbl();
        break;

    case EVENT_OBJECT_URLITEM:
    case EVENT_OBJECT_IMAGE:
        {
            const SwFrmFmtPtr pFmt = (SwFrmFmtPtr)rCallEvent.PTR.pFmt;
            if( bCheckPtr )
            {
                USHORT nPos = GetSpzFrmFmts()->GetPos( pFmt );
                if( USHRT_MAX != nPos )
                    bCheckPtr = FALSE;      // als Flag missbrauchen
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
                const SwFrmFmtPtr pFmt = (SwFrmFmtPtr)rCallEvent.PTR.IMAP.pFmt;
                USHORT nPos = GetSpzFrmFmts()->GetPos( pFmt );
                const ImageMap* pIMap;
                if( USHRT_MAX != nPos &&
                    0 != (pIMap = pFmt->GetURL().GetMap()) )
                {
                    for( nPos = pIMap->GetIMapObjectCount(); nPos; )
                        if( pIMapObj == pIMap->GetIMapObject( --nPos ))
                        {
                            bCheckPtr = FALSE;      // als Flag missbrauchen
                            break;
                        }
                }
            }
            if( !bCheckPtr )
                pTbl = &pIMapObj->GetMacroTable();
        }
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
                nRet += 0 == pDocShell->CallBasic( rMacro.GetMacName(),
                                    rMacro.GetLibName(), 0, pArgs ) ? 1 : 0;
            }
            else if( EXTENDED_STYPE == rMacro.GetScriptType() )
            {
                Sequence<Any> *pUnoArgs = 0;
                Reference<XInterface> xThis;
                if( pArgs )
                    pUnoArgs = lcl_docbasic_convertArgs( *pArgs );
                nRet += 0 == pDocShell->CallScript( rMacro.GetLanguage(),
                                      rMacro.GetMacName(), xThis, pUnoArgs );
                delete pUnoArgs;
            }
            // JavaScript calls are ignored
        }
    }
    return nRet;
}




