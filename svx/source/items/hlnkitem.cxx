/*************************************************************************
 *
 *  $RCSfile: hlnkitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

// include ---------------------------------------------------------------
#pragma hdrstop

#define _SVX_HLNKITEM_CXX

#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _MEMBERID_HRC
#include <memberid.hrc>
#endif

#ifndef __SBX_SBXVARIABLE_HXX
#include <svtools/sbxvar.hxx>
#endif

#include "hlnkitem.hxx"

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxHyperlinkItem, SfxPoolItem);

// class SvxHyperlinkItem ------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

#define HYPERLINKFF_MARKER  0x599401FE

SvStream& SvxHyperlinkItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    // store 'simple' data
    // UNICODE: rStrm << sName;
    rStrm.WriteByteString(sName);

    // UNICODE: rStrm << sURL;
    rStrm.WriteByteString(sURL);

    // UNICODE: rStrm << sTarget;
    rStrm.WriteByteString(sTarget);

    rStrm << (sal_uInt32) eType;

    // marker for versioninfo
    rStrm << (sal_uInt32) HYPERLINKFF_MARKER;

    // new data
    // UNICODE: rStrm << sIntName;
    rStrm.WriteByteString(sIntName);

    // macro-events
    rStrm << nMacroEvents;

    // store macros
    sal_uInt16 nCnt = pMacroTable ? (sal_uInt16)pMacroTable->Count() : 0;
    sal_uInt16 nMax = nCnt;
    if( nCnt )
    {
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
            if( STARBASIC != pMac->GetScriptType() )
                --nCnt;
    }

    rStrm << nCnt;

    if( nCnt )
    {
        // 1. StarBasic-Macros
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
        {
            if( STARBASIC == pMac->GetScriptType() )
            {
                rStrm << (sal_uInt16)pMacroTable->GetCurKey();

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteByteString(pMac->GetLibName());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteByteString(pMac->GetMacName());
            }
        }
    }

    nCnt = nMax - nCnt;
    rStrm << nCnt;
    if( nCnt )
    {
        // 2. ::com::sun::star::script::JavaScript-Macros
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
        {
            if( STARBASIC != pMac->GetScriptType() )
            {
                rStrm << (sal_uInt16)pMacroTable->GetCurKey();

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteByteString(pMac->GetLibName());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteByteString(pMac->GetMacName());

                rStrm << (sal_uInt16)pMac->GetScriptType();
            }
        }
    }

    return rStrm;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem*    SvxHyperlinkItem::Create( SvStream &rStrm, sal_uInt16 nItemVersion ) const
{
    SvxHyperlinkItem* pNew = new SvxHyperlinkItem( Which() );
    sal_uInt32 nType;

    // simple data-types
    // UNICODE: rStrm >> pNew->sName;
    rStrm.ReadByteString(pNew->sName);

    // UNICODE: rStrm >> pNew->sURL;
    rStrm.ReadByteString(pNew->sURL);

    // UNICODE: rStrm >> pNew->sTarget;
    rStrm.ReadByteString(pNew->sTarget);

    rStrm >> nType;
    pNew->eType = (SvxLinkInsertMode) nType;

    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nMarker;
    rStrm >> nMarker;
    if ( nMarker == HYPERLINKFF_MARKER )
    {
        // new data
        // UNICODE: rStrm >> pNew->sIntName;
        rStrm.ReadByteString(pNew->sIntName);

        // macro-events
        rStrm >> pNew->nMacroEvents;

        // macros
        sal_uInt16 nCnt;
        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey;
            String aLibName, aMacName;

            rStrm >> nCurKey;
            // UNICODE: rStrm >> aLibName;
            rStrm.ReadByteString(aLibName);

            // UNICODE: rStrm >> aMacName;
            rStrm.ReadByteString(aMacName);

            pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName, STARBASIC ) );
        }

        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey, nScriptType;
            String aLibName, aMacName;

            rStrm >> nCurKey;

            // UNICODE: rStrm >> aLibName;
            rStrm.ReadByteString(aLibName);

            // UNICODE: rStrm >> aMacName;
            rStrm.ReadByteString(aMacName);

            rStrm >> nScriptType;

            pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName,
                                        (ScriptType)nScriptType ) );
        }
    }
    else
        rStrm.Seek( nPos );

    return pNew;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SvxHyperlinkItem::SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem ):
            SfxPoolItem(rHyperlinkItem)
{
    sName   = rHyperlinkItem.sName;
    sURL    = rHyperlinkItem.sURL;
    sTarget = rHyperlinkItem.sTarget;
    eType   = rHyperlinkItem.eType;
    sIntName = rHyperlinkItem.sIntName;
    nMacroEvents = rHyperlinkItem.nMacroEvents;

    if( rHyperlinkItem.GetMacroTbl() )
        pMacroTable = new SvxMacroTableDtor( *rHyperlinkItem.GetMacroTbl() );
    else
        pMacroTable=NULL;

};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SvxHyperlinkItem::SvxHyperlinkItem( sal_uInt16 nWhich, String& rName, String& rURL,
                                    String& rTarget, String& rIntName, SvxLinkInsertMode eTyp,
                                    sal_uInt16 nEvents, SvxMacroTableDtor *pMacroTbl ):
    SfxPoolItem (nWhich),
    sName       (rName),
    sURL        (rURL),
    sTarget     (rTarget),
    eType       (eTyp),
    sIntName (rIntName),
    nMacroEvents (nEvents)
{
    if (pMacroTbl)
        pMacroTable = new SvxMacroTableDtor ( *pMacroTbl );
    else
        pMacroTable=NULL;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem* SvxHyperlinkItem::Clone( SfxItemPool* ) const
{
    return new SvxHyperlinkItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxHyperlinkItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SvxHyperlinkItem& rItem = (const SvxHyperlinkItem&) rAttr;

    sal_Bool bRet = ( sName   == rItem.sName   &&
                  sURL    == rItem.sURL    &&
                  sTarget == rItem.sTarget &&
                  eType   == rItem.eType   &&
                  sIntName == rItem.sIntName &&
                  nMacroEvents == rItem.nMacroEvents);
    if (!bRet)
        return sal_False;

    const SvxMacroTableDtor* pOther = ((SvxHyperlinkItem&)rAttr).pMacroTable;
    if( !pMacroTable )
        return ( !pOther || !pOther->Count() );
    if( !pOther )
        return 0 == pMacroTable->Count();

    const SvxMacroTableDtor& rOwn = *pMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    // Anzahl unterschiedlich => auf jeden Fall ungleich
    if( rOwn.Count() != rOther.Count() )
        return sal_False;

    // einzeln vergleichen; wegen Performance ist die Reihenfolge wichtig
    for( sal_uInt16 nNo = 0; nNo < rOwn.Count(); ++nNo )
    {
        const SvxMacro *pOwnMac = rOwn.GetObject(nNo);
        const SvxMacro *pOtherMac = rOther.GetObject(nNo);
        if (    rOwn.GetKey(pOwnMac) != rOther.GetKey(pOtherMac)  ||
                pOwnMac->GetLibName() != pOtherMac->GetLibName() ||
                pOwnMac->GetMacName() != pOtherMac->GetMacName() )
            return sal_False;
    }

    return sal_True;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxHyperlinkItem::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    if( nEvent < EVENT_SFX_START )
    {
        switch( nEvent )
        {
            case HYPERDLG_EVENT_MOUSEOVER_OBJECT:
                nEvent = SFX_EVENT_MOUSEOVER_OBJECT;
                break;
            case HYPERDLG_EVENT_MOUSECLICK_OBJECT:
                nEvent = SFX_EVENT_MOUSECLICK_OBJECT;
                break;
            case HYPERDLG_EVENT_MOUSEOUT_OBJECT:
                nEvent = SFX_EVENT_MOUSEOUT_OBJECT;
                break;
        }
    }

    if( !pMacroTable )
        pMacroTable = new SvxMacroTableDtor;

    SvxMacro *pOldMacro;
    if( 0 != ( pOldMacro = pMacroTable->Get( nEvent )) )
    {
        delete pOldMacro;
        pMacroTable->Replace( nEvent, new SvxMacro( rMacro ) );
    }
    else
        pMacroTable->Insert( nEvent, new SvxMacro( rMacro ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxHyperlinkItem::SetMacroTable( const SvxMacroTableDtor& rTbl )
{
    if ( pMacroTable )
        delete pMacroTable;

    pMacroTable = new SvxMacroTableDtor ( rTbl );
}


