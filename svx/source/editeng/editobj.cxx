/*************************************************************************
 *
 *  $RCSfile: editobj.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:29:29 $
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


#include <eeng_pch.hxx>

#pragma hdrstop

#define ENABLE_STRING_STREAM_OPERATORS
#include <tools/stream.hxx>

#include <editobj2.hxx>
#include <editdata.hxx>
#include <editattr.hxx>
#include <editeng.hxx>
#include <textconv.hxx>
#include <fontitem.hxx>
#include <cscoitem.hxx>
#include <vcl/system.hxx>
#include <flditem.hxx>
#include <lrspitem.hxx>
#include <tstpitem.hxx>
#include <bulitem.hxx>
#include <numitem.hxx>
#include <brshitem.hxx>
#include <vcl/graph.hxx>
#include <svtools/intitem.hxx>

DBG_NAME( EE_EditTextObject );
DBG_NAME( XEditAttribute );

//--------------------------------------------------------------

BOOL lcl_CreateBulletItem( const SvxNumBulletItem& rNumBullet, USHORT nLevel, SvxBulletItem& rBullet )
{
    const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( nLevel );
    if ( pFmt )
    {
        rBullet.SetWidth( (-pFmt->GetFirstLineOffset()) + pFmt->GetCharTextDistance() );
        rBullet.SetSymbol( pFmt->GetBulletChar() );
        rBullet.SetPrevText( pFmt->GetPrefix() );
        rBullet.SetFollowText( pFmt->GetSuffix() );
        rBullet.SetStart( pFmt->GetStart() );
        rBullet.SetScale( pFmt->GetBulletRelSize() );

        Font aBulletFont( rBullet.GetFont() );
        if ( pFmt->GetBulletFont() )
            aBulletFont = *pFmt->GetBulletFont();
        aBulletFont.SetColor( pFmt->GetBulletColor() );
        rBullet.SetFont( aBulletFont );

        if ( pFmt->GetBrush() && pFmt->GetBrush()->GetGraphic() )
        {
            Bitmap aBmp( pFmt->GetBrush()->GetGraphic()->GetBitmap() );
            aBmp.SetPrefSize( pFmt->GetGraphicSize() );
            aBmp.SetPrefMapMode( MAP_100TH_MM );
            rBullet.SetBitmap( aBmp );
        }

        switch ( pFmt->GetNumberingType() )
        {
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_UPPER_LETTER_N:
                rBullet.SetStyle( BS_ABC_BIG );
                break;
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER_N:
                rBullet.SetStyle( BS_ABC_SMALL );
                break;
            case SVX_NUM_ROMAN_UPPER:
                rBullet.SetStyle( BS_ROMAN_BIG );
                break;
            case SVX_NUM_ROMAN_LOWER:
                rBullet.SetStyle( BS_ROMAN_SMALL );
                break;
            case SVX_NUM_ARABIC:
                rBullet.SetStyle( BS_123 );
                break;
            case SVX_NUM_NUMBER_NONE:
                rBullet.SetStyle( BS_NONE );
                break;
            case SVX_NUM_CHAR_SPECIAL:
                rBullet.SetStyle( BS_BULLET );
                break;
            case SVX_NUM_PAGEDESC:
                DBG_ERROR( "Unknown: SVX_NUM_PAGEDESC" );
                rBullet.SetStyle( BS_BULLET );
                break;
            case SVX_NUM_BITMAP:
                rBullet.SetStyle( BS_BMP );
                break;
            default:
                DBG_ERROR( "Unknown NumType" );
        }

        switch ( pFmt->GetNumAdjust() )
        {
            case SVX_ADJUST_LEFT:
                rBullet.SetJustification( BJ_VCENTER|BJ_HLEFT );
                break;
            case SVX_ADJUST_RIGHT:
                rBullet.SetJustification( BJ_VCENTER|BJ_HRIGHT );
                break;
            case SVX_ADJUST_CENTER:
                rBullet.SetJustification( BJ_VCENTER|BJ_HCENTER );
                break;
            default:
                DBG_ERROR( "Unknown or invalid NumAdjust" );
        }
    }
    return pFmt ? TRUE : FALSE;
}


XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, USHORT nStart, USHORT nEnd )
{
    // das neue Attribut im Pool anlegen
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}


XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr )
{
    DBG_CTOR( XEditAttribute, 0 );
    pItem = &rAttr;
    nStart = 0;
    nEnd = 0;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, USHORT nS, USHORT nE )
{
    DBG_CTOR( XEditAttribute, 0 );
    pItem = &rAttr;
    nStart = nS;
    nEnd = nE;
}

XEditAttribute::~XEditAttribute()
{
    DBG_DTOR( XEditAttribute, 0 );
    pItem = 0;  // Gehoert dem Pool.
}


ContentInfo::ContentInfo( SfxItemPool& rPool ) : aParaAttribs( rPool, EE_PARA_START, EE_CHAR_END )
{
    eFamily = SFX_STYLE_FAMILY_PARA;
    pWrongs = 0;
}

// Richtiger CopyCTOR unsinning, weil ich mit einem anderen Pool arbeiten muss!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse )
    : aParaAttribs( rPoolToUse, EE_PARA_START, EE_CHAR_END )
{
    pWrongs = 0;
#ifndef SVX_LIGHT
    if ( rCopyFrom.GetWrongList() )
        pWrongs = rCopyFrom.GetWrongList()->Clone();
#endif // !SVX_LIGHT
    // So sollten die Items im richtigen Pool landen!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );
    aText = rCopyFrom.GetText();
    aStyle = rCopyFrom.GetStyle();
    eFamily = rCopyFrom.GetFamily();

    // Attribute kopieren...
    for ( USHORT n = 0; n < rCopyFrom.GetAttribs().Count(); n++  )
    {
        XEditAttribute* pAttr = rCopyFrom.GetAttribs().GetObject( n );
        XEditAttribute* pMyAttr = MakeXEditAttribute( rPoolToUse, *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
        aAttribs.Insert( pMyAttr, aAttribs.Count()  );
    }

    // Wrongs
    pWrongs = NULL;
#ifndef SVX_LIGHT
    if ( rCopyFrom.GetWrongList() )
        pWrongs = rCopyFrom.GetWrongList()->Clone();
#endif // !SVX_LIGHT
}

ContentInfo::~ContentInfo()
{
    for ( USHORT nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
    {
        XEditAttribute* pAttr = aAttribs.GetObject(nAttr);
        // Item aus Pool entfernen!
        aParaAttribs.GetPool()->Remove( *pAttr->GetItem() );
        delete pAttr;
    }
    aAttribs.Remove( 0, aAttribs.Count() );
#ifndef SVX_LIGHT
    delete pWrongs;
#endif
}


EditTextObject::EditTextObject( USHORT n)
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = n;
}

EditTextObject::EditTextObject( const EditTextObject& r )
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = r.nWhich;
}

__EXPORT EditTextObject::~EditTextObject()
{
    DBG_DTOR( EE_EditTextObject, 0 );
}

USHORT EditTextObject::GetParagraphCount() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

XubString EditTextObject::GetText( USHORT nParagraph ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return XubString();
}

void EditTextObject::Insert( const EditTextObject& rObj, USHORT nPara )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

EditTextObject* EditTextObject::CreateTextObject( USHORT nPara, USHORT nParas ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

void EditTextObject::RemoveParagraph( USHORT nPara )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL EditTextObject::HasPortionInfo() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

void EditTextObject::ClearPortionInfo()
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL EditTextObject::HasOnlineSpellErrors() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

BOOL EditTextObject::HasCharAttribs( USHORT nWhich ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

void EditTextObject::GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart, USHORT nEnd )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL EditTextObject::IsFieldObject() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

const SvxFieldItem* EditTextObject::GetField() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

BOOL EditTextObject::HasField( TypeId aType ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

SfxItemSet EditTextObject::GetParaAttribs( USHORT nPara ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return SfxItemSet( *(SfxItemPool*)NULL );
}

void EditTextObject::SetParaAttribs( USHORT nPara, const SfxItemSet& rAttribs )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL EditTextObject::RemoveCharAttribs( USHORT nWhich )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

BOOL EditTextObject::RemoveParaAttribs( USHORT nWhich )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

BOOL EditTextObject::HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

void EditTextObject::GetStyleSheet( USHORT nPara, XubString& rName, SfxStyleFamily& eFamily ) const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::SetStyleSheet( USHORT nPara, const XubString& rName, const SfxStyleFamily& eFamily )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL __EXPORT EditTextObject::ChangeStyleSheets( const XubString&, SfxStyleFamily,
                                            const XubString&, SfxStyleFamily )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return FALSE;
}

void __EXPORT EditTextObject::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const XubString& rOldName, const XubString& rNewName )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

USHORT EditTextObject::GetUserType() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

void EditTextObject::SetUserType( USHORT n )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

ULONG EditTextObject::GetObjectSettings() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

void EditTextObject::SetObjectSettings( ULONG n )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BOOL EditTextObject::IsVertical() const
{
#if  SUPD >= 614
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
#endif
    return FALSE;
}

void EditTextObject::SetVertical( BOOL bVertical )
{
#if  SUPD >= 615
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
#endif
    ((BinTextObject*)this)->SetVertical( bVertical );
}

USHORT EditTextObject::GetScriptType() const
{
#if SUPD >= 615
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
#endif
    return ((const BinTextObject*)this)->GetScriptType();
}


BOOL EditTextObject::Store( SvStream& rOStream ) const
{
    if ( rOStream.GetError() )
        return FALSE;

    // Vorspann:
    ULONG nStartPos = rOStream.Tell();

    USHORT nWhich = Which();
    rOStream << nWhich;

    ULONG nStructSz = 0;
    rOStream << nStructSz;

    // Eigene Daten:
    StoreData( rOStream );

    // Nachspann:
    ULONG nEndPos = rOStream.Tell();
    nStructSz = nEndPos - nStartPos - sizeof( nWhich ) - sizeof( nStructSz );
    rOStream.Seek( nStartPos + sizeof( nWhich ) );
    rOStream << nStructSz;
    rOStream.Seek( nEndPos );

    return rOStream.GetError() ? FALSE : TRUE;
}

EditTextObject* EditTextObject::Create( SvStream& rIStream, SfxItemPool* pGlobalTextObjectPool )
{
    ULONG nStartPos = rIStream.Tell();

    // Ertmal sehen, was fuer ein Object...
    USHORT nWhich;
    rIStream >> nWhich;

    ULONG nStructSz;
    rIStream >> nStructSz;

    DBG_ASSERT( ( nWhich == 0x22 /*EE_FORMAT_BIN300*/ ) || ( nWhich == EE_FORMAT_BIN ), "CreateTextObject: Unbekanntes Objekt!" );

    if ( rIStream.GetError() )
        return NULL;

    EditTextObject* pTxtObj = NULL;
    switch ( nWhich )
    {
        case 0x22 /*BIN300*/:       pTxtObj = new BinTextObject( 0 );
                                    ((BinTextObject*)pTxtObj)->CreateData300( rIStream );
                                    break;
        case EE_FORMAT_BIN:         pTxtObj = new BinTextObject( pGlobalTextObjectPool );
                                    pTxtObj->CreateData( rIStream );
                                    break;
        default:
        {
            // Wenn ich das Format nicht kenne, ueberlese ich den Inhalt:
            rIStream.SetError( EE_READWRITE_WRONGFORMAT );
        }
    }

    // Sicherstellen, dass der Stream an der richtigen Stelle hinterlassen wird.
    ULONG nFullSz = sizeof( nWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
    return pTxtObj;
}

void EditTextObject::Skip( SvStream& rIStream )
{
    ULONG nStartPos = rIStream.Tell();

    USHORT nWhich;
    rIStream >> nWhich;

    ULONG nStructSz;
    rIStream >> nStructSz;

    ULONG nFullSz = sizeof( nWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
}

void __EXPORT EditTextObject::StoreData( SvStream& rOStream ) const
{
    DBG_ERROR( "StoreData: Basisklasse!" );
}

void __EXPORT EditTextObject::CreateData( SvStream& rIStream )
{
    DBG_ERROR( "CreateData: Basisklasse!" );
}

USHORT EditTextObject::GetVersion() const
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
    return 0;
}

void EditTextObject::SetLRSpaceItemFlags( BOOL bOutlineMode )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::AdjustImportedLRSpaceItems( BOOL bTurnOfBullets )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::PrepareStore( SfxStyleSheetPool* pStyleSheetPool )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::FinishStore()
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

void EditTextObject::FinishLoad( SfxStyleSheetPool* pStyleSheetPool )
{
    DBG_ERROR( "V-Methode direkt vom EditTextObject!" );
}

BinTextObject::BinTextObject( SfxItemPool* pP ) :
    EditTextObject( EE_FORMAT_BIN )
{
    nVersion = 0;
    nMetric = 0xFFFF;
    nUserType = 0;
    nObjSettings = 0;
    pPortionInfo = 0;
    if ( pP )
    {
        pPool = pP;
        bOwnerOfPool = FALSE;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool =  TRUE;
    }
    bVertical = FALSE;
    bStoreUnicodeStrings = FALSE;
    nScriptType = 0;
}

BinTextObject::BinTextObject( const BinTextObject& r ) :
    EditTextObject( r )
{
    nMetric = r.nMetric;
    nUserType = r.nUserType;
    nObjSettings = r.nObjSettings;
    bVertical = r.bVertical;
    nScriptType = r.nScriptType;
    pPortionInfo = NULL;    // PortionInfo nicht kopieren
    bStoreUnicodeStrings = FALSE;
    if ( !r.bOwnerOfPool )
    {
        // Dann den Pool mitverwenden
        pPool = r.pPool;
        bOwnerOfPool = FALSE;
    }
    else
    {
        pPool = EditEngine::CreatePool();
        bOwnerOfPool =  TRUE;

    }

    if ( bOwnerOfPool && pPool && r.pPool )
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    for ( USHORT n = 0; n < r.aContents.Count(); n++ )
    {
        ContentInfo* pOrg = r.aContents.GetObject( n );
        DBG_ASSERT( pOrg, "NULL-Pointer in ContentList!" );
        ContentInfo* pNew = new ContentInfo( *pOrg, *pPool );
        aContents.Insert( pNew, aContents.Count() );
    }
}

__EXPORT BinTextObject::~BinTextObject()
{
    ClearPortionInfo();
    DeleteContents();
    if ( bOwnerOfPool )
    {
        // Nicht mehr, wegen 1xDefItems.
        // siehe auch ~EditDoc().
//      pPool->ReleaseDefaults( TRUE /* bDelete */ );
        delete pPool;
    }
}

USHORT BinTextObject::GetUserType() const
{
    return nUserType;
}

void BinTextObject::SetUserType( USHORT n )
{
    nUserType = n;
}

ULONG BinTextObject::GetObjectSettings() const
{
    return nObjSettings;
}

void BinTextObject::SetObjectSettings( ULONG n )
{
    nObjSettings = n;
}

BOOL BinTextObject::IsVertical() const
{
    return bVertical;
}

void BinTextObject::SetVertical( BOOL b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}

USHORT BinTextObject::GetScriptType() const
{
    return nScriptType;
}

void BinTextObject::SetScriptType( USHORT nType )
{
    nScriptType = nType;
}


void BinTextObject::DeleteContents()
{
    for ( USHORT n = 0; n < aContents.Count(); n++ )
    {
        ContentInfo* p = aContents.GetObject( n );
        DBG_ASSERT( p, "NULL-Pointer in ContentList!" );
        delete p;
    }
    aContents.Remove( 0, aContents.Count() );
}

EditTextObject* __EXPORT BinTextObject::Clone() const
{
    return new BinTextObject( *this );
}

XEditAttribute* BinTextObject::CreateAttrib( const SfxPoolItem& rItem, USHORT nStart, USHORT nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void BinTextObject::DestroyAttrib( XEditAttribute* pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
    delete pAttr;
}

ContentInfo* BinTextObject::CreateAndInsertContent()
{
    ContentInfo* pC = new ContentInfo( *pPool );
    aContents.Insert( pC, aContents.Count() );
    return pC;
}

USHORT BinTextObject::GetParagraphCount() const
{
    return aContents.Count();
}

XubString BinTextObject::GetText( USHORT nPara ) const
{
    DBG_ASSERT( nPara < aContents.Count(), "BinTextObject::GetText: Absatz existiert nicht!" );
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        return pC->GetText();
    }
    return XubString();
}

void BinTextObject::Insert( const EditTextObject& rObj, USHORT nDestPara )
{
    DBG_ASSERT( rObj.Which() == EE_FORMAT_BIN, "UTO: Unbekanntes Textobjekt" );

    const BinTextObject& rBinObj = (const BinTextObject&)rObj;

    if ( nDestPara > aContents.Count() )
        nDestPara = aContents.Count();

    const USHORT nParas = rBinObj.GetContents().Count();
    for ( USHORT nP = 0; nP < nParas; nP++ )
    {
        ContentInfo* pC = rBinObj.GetContents()[ nP ];
        ContentInfo* pNew = new ContentInfo( *pC, *GetPool() );
        aContents.Insert( pNew, nDestPara+nP );
    }
    ClearPortionInfo();
}

EditTextObject* BinTextObject::CreateTextObject( USHORT nPara, USHORT nParas ) const
{
    if ( ( nPara >= aContents.Count() ) || !nParas )
        return NULL;

    // Pool nur teilen, wenn von aussen eingestellter Pool.
    BinTextObject* pObj = new BinTextObject( bOwnerOfPool ? 0 : pPool );
    if ( bOwnerOfPool && pPool )
        pObj->GetPool()->SetDefaultMetric( pPool->GetMetric( DEF_METRIC ) );

    // If complete text is only one ScriptType, this is valid.
    // If text contains different ScriptTypes, this shouldn't be a problem...
    pObj->nScriptType = nScriptType;

    const USHORT nEndPara = nPara+nParas-1;
    for ( USHORT nP = nPara; nP <= nEndPara; nP++ )
    {
        ContentInfo* pC = aContents[ nP ];
        ContentInfo* pNew = new ContentInfo( *pC, *pObj->GetPool() );
        pObj->GetContents().Insert( pNew, pObj->GetContents().Count() );
    }
    return pObj;
}

void BinTextObject::RemoveParagraph( USHORT nPara )
{
    DBG_ASSERT( nPara < aContents.Count(), "BinTextObject::GetText: Absatz existiert nicht!" );
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        aContents.Remove( nPara );
        delete pC;
        ClearPortionInfo();
    }
}

BOOL BinTextObject::HasPortionInfo() const
{
    return pPortionInfo ? TRUE : FALSE;
}

void BinTextObject::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        for ( USHORT n = pPortionInfo->Count(); n; )
            delete pPortionInfo->GetObject( --n );
        delete pPortionInfo;
        pPortionInfo = NULL;
    }
}

BOOL BinTextObject::HasOnlineSpellErrors() const
{
#ifndef SVX_LIGHT
    for ( USHORT n = 0; n < aContents.Count(); n++ )
    {
        ContentInfo* p = aContents.GetObject( n );
        if ( p->GetWrongList() && p->GetWrongList()->Count() )
            return TRUE;
    }
#endif // !SVX_LIGHT
    return FALSE;

}

BOOL BinTextObject::HasCharAttribs( USHORT nWhich ) const
{
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        USHORT nAttribs = pC->GetAttribs().Count();
        if ( nAttribs && !nWhich )
            return TRUE;

        for ( USHORT nAttr = nAttribs; nAttr; )
        {
            XEditAttribute* pX = pC->GetAttribs().GetObject( --nAttr );
            if ( pX->GetItem()->Which() == nWhich )
                return TRUE;
        }
    }
    return FALSE;
}

void BinTextObject::GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const
{
    rLst.Remove( 0, rLst.Count() );
    ContentInfo* pC = GetContents().GetObject( nPara );
    if ( pC )
    {
        for ( USHORT nAttr = 0; nAttr < pC->GetAttribs().Count(); nAttr++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( nAttr );
            EECharAttrib aEEAttr;
            aEEAttr.pAttr = pAttr->GetItem();
            aEEAttr.nPara = nPara;
            aEEAttr.nStart = pAttr->GetStart();
            aEEAttr.nEnd = pAttr->GetEnd();
            rLst.Insert( aEEAttr, rLst.Count() );
        }
    }
}

void BinTextObject::MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart, USHORT nEnd )
{
    BOOL bChanged = FALSE;

    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        for ( USHORT nW = nStart; nW <= nEnd; nW++ )
        {
            if ( ( pC->GetParaAttribs().GetItemState( nW, FALSE ) != SFX_ITEM_ON )
                    && ( rAttribs.GetItemState( nW, FALSE ) == SFX_ITEM_ON ) )
            {
                pC->GetParaAttribs().Put( rAttribs.Get( nW ) );
                bChanged = TRUE;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();
}

BOOL BinTextObject::IsFieldObject() const
{
    return BinTextObject::GetField() ? TRUE : FALSE;
}

const SvxFieldItem* BinTextObject::GetField() const
{
    if ( GetContents().Count() == 1 )
    {
        ContentInfo* pC = GetContents()[0];
        if ( pC->GetText().Len() == 1 )
        {
            USHORT nAttribs = pC->GetAttribs().Count();
            for ( USHORT nAttr = nAttribs; nAttr; )
            {
                XEditAttribute* pX = pC->GetAttribs().GetObject( --nAttr );
                if ( pX->GetItem()->Which() == EE_FEATURE_FIELD )
                    return (const SvxFieldItem*)pX->GetItem();
            }
        }
    }
    return 0;
}

BOOL BinTextObject::HasField( TypeId aType ) const
{
    USHORT nParagraphs = GetContents().Count();
    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        USHORT nAttrs = pC->GetAttribs().Count();
        for ( USHORT nAttr = 0; nAttr < nAttrs; nAttr++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs()[nAttr];
            if ( pAttr->GetItem()->Which() == EE_FEATURE_FIELD )
            {
                if ( !aType )
                    return TRUE;

                const SvxFieldData* pFldData = ((const SvxFieldItem*)pAttr->GetItem())->GetField();
                if ( pFldData && pFldData->IsA( aType ) )
                    return TRUE;
            }
        }
    }
    return FALSE;
}

SfxItemSet BinTextObject::GetParaAttribs( USHORT nPara ) const
{
    ContentInfo* pC = GetContents().GetObject( nPara );
    return pC->GetParaAttribs();
}

void BinTextObject::SetParaAttribs( USHORT nPara, const SfxItemSet& rAttribs )
{
    ContentInfo* pC = GetContents().GetObject( nPara );
    pC->GetParaAttribs().Set( rAttribs );
    ClearPortionInfo();
}

BOOL BinTextObject::RemoveCharAttribs( USHORT nWhich )
{
    BOOL bChanged = FALSE;

    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        for ( USHORT nAttr = pC->GetAttribs().Count(); nAttr; )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( --nAttr );
            if ( !nWhich || ( pAttr->GetItem()->Which() == nWhich ) )
            {
                pC->GetAttribs().Remove( nAttr );
                DestroyAttrib( pAttr );
                bChanged = TRUE;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

BOOL BinTextObject::RemoveParaAttribs( USHORT nWhich )
{
    BOOL bChanged = FALSE;

    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        if ( !nWhich )
        {
            if( pC->GetParaAttribs().Count() )
                bChanged = TRUE;
            pC->GetParaAttribs().ClearItem();
        }
        else
        {
            if ( pC->GetParaAttribs().GetItemState( nWhich ) == SFX_ITEM_ON )
            {
                pC->GetParaAttribs().ClearItem( nWhich );
                bChanged = TRUE;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();

    return bChanged;
}

BOOL BinTextObject::HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const
{
    USHORT nParagraphs = GetContents().Count();
    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        if ( ( pC->GetFamily() == eFamily ) && ( pC->GetStyle() == rName ) )
            return TRUE;
    }
    return FALSE;
}

void BinTextObject::GetStyleSheet( USHORT nPara, XubString& rName, SfxStyleFamily& rFamily ) const
{
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        rName = pC->GetStyle();
        rFamily = pC->GetFamily();
    }
}

void BinTextObject::SetStyleSheet( USHORT nPara, const XubString& rName, const SfxStyleFamily& rFamily )
{
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        pC->GetStyle() = rName;
        pC->GetFamily() = rFamily;
    }
}

BOOL BinTextObject::ImpChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    const USHORT nParagraphs = GetContents().Count();
    BOOL bChanges = FALSE;

    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        if ( pC->GetFamily() == eOldFamily )
        {
            if ( pC->GetStyle() == rOldName )
            {
                pC->GetStyle() = rNewName;
                pC->GetFamily() = eNewFamily;
                bChanges = TRUE;
            }
        }
    }
    return bChanges;
}

BOOL __EXPORT BinTextObject::ChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    BOOL bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void __EXPORT BinTextObject::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const XubString& rOldName, const XubString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

void __EXPORT BinTextObject::StoreData( SvStream& rOStream ) const
{
    USHORT nVer = 602;
    rOStream << nVer;

    rOStream << bOwnerOfPool;

    // Erst den Pool speichern, spaeter nur noch Surregate
    if ( bOwnerOfPool )
    {
        GetPool()->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
        GetPool()->Store( rOStream );
    }

    // Aktuelle Zeichensatz speichern...
    // GetStoreCharSet: Dateiformat fuer West-Europaeische Versionen kompatibel halten.
    rtl_TextEncoding eEncoding = GetStoreCharSet( gsl_getSystemTextEncoding(), (USHORT) rOStream.GetVersion() );
    rOStream << (USHORT) eEncoding;

    // Die Anzahl der Absaetze...
    USHORT nParagraphs = GetContents().Count();
    rOStream << nParagraphs;

    char cFeatureConverted = ByteString( CH_FEATURE, eEncoding ).GetChar(0);

    // Die einzelnen Absaetze...
    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );

        // Text...
        ByteString aText( pC->GetText(), eEncoding );
        // Convert CH_FEATURE to CH_FEATURE_OLD
        aText.SearchAndReplaceAll( cFeatureConverted, CH_FEATURE_OLD );
        rOStream.WriteByteString( aText );

        // StyleName und Family...
        rOStream.WriteByteString( ByteString( pC->GetStyle(), eEncoding ) );
        rOStream << (USHORT)pC->GetFamily();

        // Absatzattribute...
        pC->GetParaAttribs().Store( rOStream );

        // Die Anzahl der Attribute...
        USHORT nAttribs = pC->GetAttribs().Count();
        rOStream << nAttribs;

        // Und die einzelnen Attribute
        // Items als Surregate => immer 8 Byte pro Attrib
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            XEditAttribute* pX = pC->GetAttribs().GetObject( nAttr );

            rOStream << pX->GetItem()->Which();
            GetPool()->StoreSurrogate( rOStream, pX->GetItem() );
            rOStream << pX->GetStart();
            rOStream << pX->GetEnd();
        }
    }

    // Ab 400:
    rOStream << nMetric;

    // Ab 600
    rOStream << nUserType;
    rOStream << nObjSettings;

    // Ab 601
    rOStream << bVertical;

    // Ab 602
    rOStream << nScriptType;

    rOStream << bStoreUnicodeStrings;
    if ( bStoreUnicodeStrings )
    {
        for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
        {
            ContentInfo* pC = GetContents().GetObject( nPara );
            USHORT nL = pC->GetText().Len();
            rOStream.WriteNumber( nL );
            rOStream.Write( pC->GetText().GetBuffer(), nL*sizeof(sal_Unicode) );
        }
    }
}

void __EXPORT BinTextObject::CreateData( SvStream& rIStream )
{
    rIStream >> nVersion;

    // Das Textobject wurde erstmal mit der aktuellen Einstellung
    // von pTextObjectPool erzeugt.
    BOOL bOwnerOfCurrent = bOwnerOfPool;
    rIStream >> bOwnerOfPool;

    if ( bOwnerOfCurrent && !bOwnerOfPool )
    {
        // Es wurde ein globaler Pool verwendet, mir jetzt nicht uebergeben,
        // aber ich brauche ihn!
        DBG_ERROR( "Man gebe mir den globalen TextObjectPool!" );
        return;
    }
    else if ( !bOwnerOfCurrent && bOwnerOfPool )
    {
        // Es soll ein globaler Pool verwendet werden, aber dieses
        // Textobject hat einen eigenen.
        pPool = EditEngine::CreatePool();
    }

    if ( bOwnerOfPool )
        GetPool()->Load( rIStream );

    // CharSet, in dem gespeichert wurde:
    USHORT nCharSet;
    rIStream >> nCharSet;

    rtl_TextEncoding eSrcEncoding = (rtl_TextEncoding)nCharSet;

    // Die Anzahl der Absaetze...
    USHORT nParagraphs;
    rIStream >> nParagraphs;

    // Die einzelnen Absaetze...
    for ( ULONG nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // Der Text...
        ByteString aByteString;
        rIStream.ReadByteString( aByteString );
        pC->GetText() = String( aByteString, eSrcEncoding );

        // StyleName und Family...
        rIStream.ReadByteString( pC->GetStyle(), eSrcEncoding );
        USHORT nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Absatzattribute...
        pC->GetParaAttribs().Load( rIStream );

        // Die Anzahl der Attribute...
        USHORT nAttribs;
        rIStream >> nAttribs;

        // Und die einzelnen Attribute
        // Items als Surregate => immer 8 Byte pro Attrib
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( ULONG nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            USHORT nWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> nWhich;
            nWhich = pPool->GetNewWhich( nWhich );
            pItem = pPool->LoadSurrogate( rIStream, nWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                if ( pItem->Which() == EE_FEATURE_NOTCONV )
                {
                    pC->GetText().SetChar( nStart, ByteString::ConvertToUnicode( aByteString.GetChar( nStart ), ((SvxCharSetColorItem*)pItem)->GetCharSet() ) );
                }
                else
                {
                    XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                    pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );

                    if ( ( nWhich >= EE_FEATURE_START ) && ( nWhich <= EE_FEATURE_END ) )
                    {
                        // Convert CH_FEATURE to CH_FEATURE_OLD
                        DBG_ASSERT( (BYTE) aByteString.GetChar( nStart ) == CH_FEATURE_OLD, "CreateData: CH_FEATURE expected!" );
                        if ( (BYTE) aByteString.GetChar( nStart ) == CH_FEATURE_OLD )
                            pC->GetText().SetChar( nStart, CH_FEATURE );
                    }
                }
            }
        }
    }

    // Ab 400 auch die DefMetric:
    if ( nVersion >= 400 )
    {
        USHORT nTmpMetric;
        rIStream >> nTmpMetric;
        if ( nVersion >= 401 )
        {
            // In der 400 gab es noch einen Bug bei Textobjekten mit eigenem
            // Pool, deshalb erst ab 401 auswerten.
            nMetric = nTmpMetric;
            if ( bOwnerOfPool && pPool && ( nMetric != 0xFFFF ) )
                pPool->SetDefaultMetric( (SfxMapUnit)nMetric );
        }
    }

    if ( nVersion >= 600 )
    {
        rIStream >> nUserType;
        rIStream >> nObjSettings;
    }

    if ( nVersion >= 601 )
    {
        rIStream >> bVertical;
    }

    if ( nVersion >= 602 )
    {
        rIStream >> nScriptType;

        BOOL bUnicodeStrings;
        rIStream >> bUnicodeStrings;
        if ( bUnicodeStrings )
        {
            for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
            {
                ContentInfo* pC = GetContents().GetObject( nPara );
                USHORT nL;
                rIStream.ReadNumber( nL );
                pC->GetText().AllocBuffer( nL );
                rIStream.Read( pC->GetText().GetBufferAccess(), nL*sizeof(sal_Unicode) );
                pC->GetText().ReleaseBufferAccess( nL );
            }
        }
    }


    // Ab 500 werden die Tabs anders interpretiert: TabPos + LI, vorher nur TabPos.
    // Wirkt nur wenn auch Tab-Positionen eingestellt wurden, nicht beim DefTab.
    if ( nVersion < 500 )
    {
        for ( USHORT n = 0; n < aContents.Count(); n++ )
        {
            ContentInfo* pC = aContents.GetObject( n );
            const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( EE_PARA_LRSPACE );
            if ( rLRSpace.GetTxtLeft() && ( pC->GetParaAttribs().GetItemState( EE_PARA_TABS ) == SFX_ITEM_ON ) )
            {
                const SvxTabStopItem& rTabs = (const SvxTabStopItem&) pC->GetParaAttribs().Get( EE_PARA_TABS );
                SvxTabStopItem aNewTabs( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
                for ( USHORT t = 0; t < rTabs.Count(); t++ )
                {
                    const SvxTabStop& rT = rTabs[ t ];
                    aNewTabs.Insert( SvxTabStop( rT.GetTabPos() - rLRSpace.GetTxtLeft(),
                                rT.GetAdjustment(), rT.GetDecimal(), rT.GetFill() ) );
                }
                pC->GetParaAttribs().Put( aNewTabs );
            }
        }
    }
}

USHORT BinTextObject::GetVersion() const
{
    return nVersion;
}

void BinTextObject::SetLRSpaceItemFlags( BOOL bOutlineMode )
{
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );
        for ( USHORT n = 0; n <=1; n++ )
        {
            USHORT nItemId = n ? EE_PARA_LRSPACE : EE_PARA_OUTLLRSPACE;
            if ( pC->GetParaAttribs().GetItemState( nItemId ) == SFX_ITEM_ON )
            {
                const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( nItemId );
                if ( rItem.IsBulletFI() != bOutlineMode )
                {
                    SvxLRSpaceItem aNewItem( rItem );
                    aNewItem.SetBulletFI( bOutlineMode );
                    pC->GetParaAttribs().Put( aNewItem );
                }
            }
        }
    }
}

void BinTextObject::PrepareStore( SfxStyleSheetPool* pStyleSheetPool )
{
    // Some Items must be generated for the 5.0 file format,
    // because we don't have a special format for 5.x or 6.x
    USHORT nParas = GetContents().Count();
    const SvxNumBulletItem** ppNumBulletItems = new const SvxNumBulletItem*[nParas];
    for ( USHORT nPara = nParas; nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );
        const SvxNumBulletItem* pSvxNumBulletItem = NULL;
        const SfxPoolItem* pTmpItem = NULL;
        if ( pC->GetParaAttribs().GetItemState(EE_PARA_NUMBULLET, FALSE, &pTmpItem ) == SFX_ITEM_ON )
        {
            pSvxNumBulletItem = (const SvxNumBulletItem*)pTmpItem;
        }
        else if ( pStyleSheetPool && pC->GetStyle().Len() )
        {
            SfxStyleSheet* pStyle = (SfxStyleSheet*)pStyleSheetPool->Find( pC->GetStyle(), pC->GetFamily() );
            if ( pStyle )
                pSvxNumBulletItem = &(const SvxNumBulletItem&)pStyle->GetItemSet().Get(EE_PARA_NUMBULLET);
        }

        ppNumBulletItems[nPara] = pSvxNumBulletItem;

        if ( pSvxNumBulletItem )
        {
            // Check if Item allready used, don't create a new one in this case.
            BOOL bInserted = FALSE;
            for ( USHORT nP = nPara+1; nP < nParas; nP++ )
            {
                if ( ppNumBulletItems[nP] == pSvxNumBulletItem )
                {
                    ContentInfo* pTmpC = GetContents().GetObject( nP );
                    pC->GetParaAttribs().Put( pTmpC->GetParaAttribs().Get( EE_PARA_BULLET ) );
                    bInserted = TRUE;
                    break;
                }
            }
            if ( !bInserted )
            {
                SvxBulletItem aNewBullet( EE_PARA_BULLET );
                const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pC->GetParaAttribs().Get( EE_PARA_OUTLLEVEL );
                lcl_CreateBulletItem( *pSvxNumBulletItem, rLevel.GetValue(), aNewBullet );
                pC->GetParaAttribs().Put( aNewBullet );
            }
        }
     }
    delete ppNumBulletItems;
}

void BinTextObject::FinishStore()
{
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );
        pC->GetParaAttribs().ClearItem( EE_PARA_BULLET );
    }
}

void BinTextObject::FinishLoad( SfxStyleSheetPool* pStyleSheetPool )
{
    BOOL bCreateNumBulletItem = nVersion && ( nVersion < 501 );
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        if ( bCreateNumBulletItem )
        {
            BOOL bBulletInPara = pC->GetParaAttribs().GetItemState( EE_PARA_BULLET ) == SFX_ITEM_ON;
            BOOL bLRSpaceInPara = pC->GetParaAttribs().GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON;

            // Nur wenn ein Attribut hart gesetzt, ansonsten muss es in den Vorlagen
            // richtig konvertiert sein.

            if ( bBulletInPara || bLRSpaceInPara )
            {
                const SvxBulletItem* pBullet = NULL;
                const SvxLRSpaceItem* pLRSpace = NULL;
                SvxNumBulletItem* pNumBullet = NULL;
                SfxStyleSheet* pStyle = NULL;

                if ( pC->GetStyle().Len() )
                    pStyle = (SfxStyleSheet*)pStyleSheetPool->Find( pC->GetStyle(), pC->GetFamily() );

                const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pC->GetParaAttribs().Get( EE_PARA_OUTLLEVEL );
                USHORT nLevel = rLevel.GetValue();

                if ( !pStyle || bBulletInPara )
                    pBullet = (const SvxBulletItem*) &pC->GetParaAttribs().Get( EE_PARA_BULLET );
                else
                    pBullet = (const SvxBulletItem*) &pStyle->GetItemSet().Get( EE_PARA_BULLET );

                if ( !pStyle || bLRSpaceInPara )
                    pLRSpace = (const SvxLRSpaceItem*) &pC->GetParaAttribs().Get( EE_PARA_LRSPACE );
                else
                    pLRSpace = (const SvxLRSpaceItem*) &pStyle->GetItemSet().Get( EE_PARA_LRSPACE );

                if ( !pStyle || ( pC->GetParaAttribs().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON ) )
                    pNumBullet = (SvxNumBulletItem*) &pC->GetParaAttribs().Get( EE_PARA_NUMBULLET );
                else
                    pNumBullet = (SvxNumBulletItem*) &pStyle->GetItemSet().Get( EE_PARA_NUMBULLET );

                SvxNumBulletItem aNumBullet( *pNumBullet );
                EditEngine::ImportBulletItem( aNumBullet, nLevel, pBullet, pLRSpace );
                pC->GetParaAttribs().Put( aNumBullet );

                if ( bLRSpaceInPara )
                    pC->GetParaAttribs().ClearItem( EE_PARA_LRSPACE );
            }
        }

        // MT 07/00: EE_PARA_BULLET no longer needed
        pC->GetParaAttribs().ClearItem( EE_PARA_BULLET );
    }
}

void BinTextObject::AdjustImportedLRSpaceItems( BOOL bTurnOfBullets )
{
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );
        if ( !bTurnOfBullets )
        {
            for ( USHORT n = 0; n <=1; n++ )
            {
                USHORT nItemId = n ? EE_PARA_LRSPACE : EE_PARA_OUTLLRSPACE;
                if ( pC->GetParaAttribs().GetItemState( nItemId ) == SFX_ITEM_ON )
                {
                    const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( nItemId );
                    if ( !rItem.GetTxtFirstLineOfst() )
                    {
                        SvxLRSpaceItem aNewItem( rItem );
                        aNewItem.SetTxtFirstLineOfst( (short) -1200 ); // Outliner im Draw mit MAP100TH_MM
                        if( pC->GetParaAttribs().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                        {
                            const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pC->GetParaAttribs().Get( EE_PARA_NUMBULLET );
                            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pC->GetParaAttribs().Get( EE_PARA_OUTLLEVEL );
                            const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( rLevel.GetValue() );
                            if ( pFmt )
                                aNewItem.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
                        }
                        pC->GetParaAttribs().Put( aNewItem );
                    }
                }
            }
        }

        // Wurden in alten Versionen vom Outliner mal am Absatz gesetzt, soll nicht mehr sein
        if ( !bTurnOfBullets && ( nVersion < 500 ) )
        {
            for ( USHORT nW = EE_CHAR_START; nW <= EE_CHAR_END; nW++  )
                pC->GetParaAttribs().ClearItem( nW );
        }

        if ( bTurnOfBullets )
        {
            SfxUInt16Item aBulletState( EE_PARA_BULLETSTATE, 0 );
            pC->GetParaAttribs().Put( aBulletState );
        }
    }
}



#define CHARSETMARKER   0x9999

void __EXPORT BinTextObject::CreateData300( SvStream& rIStream )
{
    // Fuer Aufwaertskompatibilitaet.

    // Erst den Pool laden...
    // Ist in der 300 immer gespeichert worden!
    GetPool()->Load( rIStream );

    // Die Anzahl der Absaetze...
    ULONG nParagraphs;
    rIStream >> nParagraphs;

    // Die einzelnen Absaetze...
    for ( ULONG nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // Der Text...
        rIStream.ReadByteString( pC->GetText() );

        // StyleName und Family...
        rIStream.ReadByteString( pC->GetStyle() );
        USHORT nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Absatzattribute...
        pC->GetParaAttribs().Load( rIStream );

        // Die Anzahl der Attribute...
        ULONG nAttribs;
        rIStream >> nAttribs;

        // Und die einzelnen Attribute
        // Items als Surregate => immer 8 Byte pro Attrib
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( ULONG nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            USHORT nWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> nWhich;
            nWhich = pPool->GetNewWhich( nWhich );
            pItem = pPool->LoadSurrogate( rIStream, nWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );
            }
        }
    }

    // Prueffen, ob ein Zeichensatz gespeichert wurde
    USHORT nCharSetMarker;
    rIStream >> nCharSetMarker;
    if ( nCharSetMarker == CHARSETMARKER )
    {
        USHORT nCharSet;
        rIStream >> nCharSet;
    }
}
