/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlobj.cxx,v $
 * $Revision: 1.12.78.1 $
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
#include "precompiled_svx.hxx"

#include <outl_pch.hxx>

#define _OUTLINER_CXX
#include <svx/outliner.hxx>
#include <svx/outlobj.hxx>
#include <outleeng.hxx>
#include <svx/editobj.hxx>


#include <vcl/bitmap.hxx>


#include <tools/stream.hxx>

DBG_NAME(OutlinerParaObject)

OutlinerParaObject::OutlinerParaObject( USHORT nParaCount )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc = TRUE;
    pParagraphDataArr = new ParagraphData[ nParaCount ];
    nCount = nParaCount;
}

OutlinerParaObject::OutlinerParaObject( const OutlinerParaObject& rObj )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc = rObj.bIsEditDoc;
    nCount = rObj.nCount;
    pParagraphDataArr = new ParagraphData[ nCount ];
    for( sal_uInt32 i = 0; i < nCount; i++ )
        pParagraphDataArr[i] = rObj.pParagraphDataArr[i];
    pText = rObj.pText->Clone();
}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rEditObj )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc  = TRUE;
    pText       = rEditObj.Clone();
    nCount      = pText->GetParagraphCount();
    pParagraphDataArr   = new ParagraphData[ nCount ];
}

OutlinerParaObject::~OutlinerParaObject()
{
    DBG_DTOR(OutlinerParaObject,0);

    delete pText;
    delete[] pParagraphDataArr;
}

sal_Int16 OutlinerParaObject::GetDepth( USHORT nPara ) const
{
    if( pParagraphDataArr && (nPara < nCount ) )
        return pParagraphDataArr[nPara].nDepth;
    else
        return -1;
}

void OutlinerParaObject::ClearPortionInfo()
{
    DBG_CHKTHIS(OutlinerParaObject,0);
    pText->ClearPortionInfo();
}

OutlinerParaObject* OutlinerParaObject::Clone() const
{
    DBG_CHKTHIS(OutlinerParaObject,0);
    OutlinerParaObject* pObj = new OutlinerParaObject(*this);
    return pObj;
}

void OutlinerParaObject::ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName )
{
    DBG_CHKTHIS(OutlinerParaObject,0);
    pText->ChangeStyleSheetName( eFamily, rOldName, rNewName );
}

BOOL OutlinerParaObject::ChangeStyleSheets( const XubString& rOldName, SfxStyleFamily eOldFamily, const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    DBG_CHKTHIS(OutlinerParaObject,0);
    return pText->ChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
}

void OutlinerParaObject::SetStyleSheets( USHORT nLevel, const XubString rNewName, const SfxStyleFamily& rNewFamily )
{
    for ( USHORT n = sal::static_int_cast< USHORT >( Count() ); n; )
    {
        if ( GetDepth( --n ) == nLevel )
            pText->SetStyleSheet( n, rNewName, rNewFamily );
    }
}

void OutlinerParaObject::Store(SvStream& rStream ) const
{
    rStream << nCount;
    rStream << static_cast<sal_uInt32>(0x42345678);
    pText->Store( rStream );

    for( USHORT nPos=0; nPos < nCount; nPos++ )
        rStream << pParagraphDataArr[ nPos ].nDepth;

    rStream << bIsEditDoc;
}

OutlinerParaObject* OutlinerParaObject::Create( SvStream& rStream, SfxItemPool* pTextObjectPool )
{
    OutlinerParaObject* pPObj = NULL;
    USHORT nVersion = 0;

    sal_uInt32 nCount;
    rStream >> nCount;

    sal_uInt32 nSyncRef;
    rStream >> nSyncRef;
    if( nSyncRef == 0x12345678 )
        nVersion = 1;
    else if( nSyncRef == 0x22345678 )
        nVersion = 2;
    else if( nSyncRef == 0x32345678 )
        nVersion = 3;
    else if ( nSyncRef == 0x42345678 )
        nVersion = 4;

    if ( nVersion )
    {
        pPObj = new OutlinerParaObject( (USHORT)nCount );
        if( nVersion <= 3 )
        {
            EditTextObject* pAllText = 0;
            USHORT nCurPara = 0;
            while ( nCount )
            {
                EditTextObject* pText = EditTextObject::Create( rStream, NULL );
                DBG_ASSERT(pText,"CreateEditTextObject failed")
                sal_uInt32 nSync = 0;
                rStream >> nSync;
                DBG_ASSERT(nSync==nSyncRef,"Stream out of sync")
                USHORT nDepth;
                rStream >> nDepth;
                Paragraph* pPara = new Paragraph( nDepth );
                if( nVersion == 1 )
                {
                    // Bullet ueberlesen
                    USHORT nFlags;
                    rStream >> nFlags;

                    if ( nFlags & 0x0001 )  // Bitmap
                    {
                        Bitmap aBmp;
                        rStream >> aBmp;
                    }
                    else
                    {
                        Color aColor;
                        rStream >> aColor;
                        rStream.SeekRel( 16 );
                        String aName;
                        rStream.ReadByteString(aName);
                        rStream.SeekRel( 12 );
                    }
                    long nDummy;
                    rStream >> nDummy;
                    rStream >> nDummy;
                }
                pPara->bVisible = TRUE;
                if( !pAllText )
                    pAllText = pText;
                else
                {
                    pAllText->Insert( *pText, 0xffff );
                    delete pText;
                }
                pPObj->pParagraphDataArr[ nCurPara ] = *pPara;
                delete pPara;
                nCount--;
                nCurPara++;
                if( nCount )
                {
                    sal_uInt32 _nSync = 0;
                    rStream >> _nSync;
                    DBG_ASSERT(_nSync==nSyncRef,"Stream out of sync")
                }
            }
            if( nVersion == 3 )
                rStream >> pPObj->bIsEditDoc;
            pPObj->pText = pAllText;
        }
        else // nVersion >= 4
        {
            pPObj->pText = EditTextObject::Create( rStream, pTextObjectPool );
            for( USHORT nCur=0; nCur < nCount; nCur++ )
                rStream >> pPObj->pParagraphDataArr[ nCur ].nDepth;
            rStream >> pPObj->bIsEditDoc;
        }
    }
    return pPObj;
}

USHORT OutlinerParaObject::GetOutlinerMode() const
{
    return pText->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode( USHORT n )
{
    pText->SetUserType( n );
}

BOOL OutlinerParaObject::RemoveCharAttribs( USHORT nWhich )
{
    return pText->RemoveCharAttribs( nWhich );
}

BOOL OutlinerParaObject::RemoveParaAttribs( USHORT nWhich )
{
    return pText->RemoveParaAttribs( nWhich );
}

void OutlinerParaObject::MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart, USHORT nEnd )
{
    pText->MergeParaAttribs( rAttribs, nStart, nEnd );
}

void OutlinerParaObject::SetVertical( BOOL bVertical )
{
    pText->SetVertical( bVertical );
}

BOOL OutlinerParaObject::IsVertical() const
{
    return pText->IsVertical();
}

