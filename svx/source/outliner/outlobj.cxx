/*************************************************************************
 *
 *  $RCSfile: outlobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:23 $
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

#include <outl_pch.hxx>

#pragma hdrstop


#define _OUTLINER_CXX
#include <outliner.hxx>
#include <outlobj.hxx>
#include <outleeng.hxx>

#ifndef _EDITOBJ_HXX //autogen
#include <editobj.hxx>
#endif


#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif


#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

DBG_NAME(OutlinerParaObject);

OutlinerParaObject::OutlinerParaObject( USHORT nParaCount )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc = TRUE;
    pDepthArr = new USHORT[ nParaCount ];
    nCount = (ULONG)nParaCount;
}

OutlinerParaObject::OutlinerParaObject( const OutlinerParaObject& rObj )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc = rObj.bIsEditDoc;
    nCount = rObj.nCount;
    pDepthArr = new USHORT[ (USHORT)nCount ];
    memcpy( pDepthArr, rObj.pDepthArr, (size_t)(sizeof(USHORT)*nCount) );
    pText = rObj.pText->Clone();
}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rEditObj )
{
    DBG_CTOR(OutlinerParaObject,0);

    bIsEditDoc  = TRUE;
    pText       = rEditObj.Clone();
    nCount      = pText->GetParagraphCount();
    pDepthArr   = new USHORT[ nCount ];
    memset( pDepthArr, 0, nCount*sizeof(USHORT) );
}

OutlinerParaObject::~OutlinerParaObject()
{
    DBG_DTOR(OutlinerParaObject,0);

    delete pText;
    delete pDepthArr;
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
    for ( USHORT n = Count(); n; )
    {
        if ( GetDepth( --n ) == nLevel )
            pText->SetStyleSheet( n, rNewName, rNewFamily );
    }
}

void OutlinerParaObject::Store(SvStream& rStream ) const
{
    rStream << nCount;
    rStream << (ULONG) 0x42345678;
    pText->Store( rStream );

    for( USHORT nPos=0; nPos < nCount; nPos++ )
        rStream << pDepthArr[ nPos ];

    rStream << bIsEditDoc;
}

OutlinerParaObject* OutlinerParaObject::Create( SvStream& rStream, SfxItemPool* pTextObjectPool )
{
    OutlinerParaObject* pPObj = NULL;
    USHORT nVersion = 0;

    ULONG nCount;
    rStream >> nCount;

    ULONG nSyncRef;
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
                ULONG nSync = 0;
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
                pPObj->pDepthArr[ nCurPara ] = pPara->GetDepth();
                delete pPara;
                nCount--;
                nCurPara++;
                if( nCount )
                {
                    ULONG nSync = 0;
                    rStream >> nSync;
                    DBG_ASSERT(nSync==nSyncRef,"Stream out of sync")
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
                rStream >> pPObj->pDepthArr[ nCur ];
            rStream >> pPObj->bIsEditDoc;

            if ( pPObj->pText->GetVersion() < 501 )
                pPObj->pText->AdjustImportedLRSpaceItems( pPObj->bIsEditDoc );

            // MT: Bei der naechsten Version mal eine Recordlaenge einfuehren!
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

void OutlinerParaObject::SetLRSpaceItemFlags( BOOL bOutlineMode )
{
    pText->SetLRSpaceItemFlags( bOutlineMode );
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

void OutlinerParaObject::PrepareStore( SfxStyleSheetPool* pStyleSheetPool )
{
    pText->PrepareStore( pStyleSheetPool );
}

void OutlinerParaObject::FinishStore()
{
    pText->FinishStore();
}

void OutlinerParaObject::FinishLoad( SfxStyleSheetPool* pStyleSheetPool )
{
    pText->FinishLoad( pStyleSheetPool );
}

