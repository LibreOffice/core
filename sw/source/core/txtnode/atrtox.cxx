/*************************************************************************
 *
 *  $RCSfile: atrtox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-06 10:43:55 $
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

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif

SwTxtTOXMark::SwTxtTOXMark( const SwTOXMark& rAttr,
                    xub_StrLen nStart, xub_StrLen* pEnde )
    : SwTxtAttrEnd( rAttr, nStart, nStart ),
    pEnd( 0 ),
    pMyTxtNd( 0 )
{
    ((SwTOXMark&)rAttr).pTxtAttr = this;
    if( !rAttr.GetAlternativeText().Len() )
    {
        nEnd = *pEnde;
        pEnd = &nEnd;
    }
    SetDontMergeAttr( TRUE );
    SetDontMoveAttr( TRUE );
    SetOverlapAllowedAttr( TRUE );
}

SwTxtTOXMark::~SwTxtTOXMark()
{
}

xub_StrLen* SwTxtTOXMark::GetEnd()
{
    return pEnd;
}

void SwTxtTOXMark::ChgFnt(SwFont *pFont)
{
    ASSERT( pFont->GetTox()<255, "ChgFnt: Tox-Schachtelungstiefe zu gross" );
    pFont->GetTox()++;
}

void SwTxtTOXMark::RstFnt(SwFont *pFont)
{
    ASSERT( pFont->GetTox(), "RstFnt: Tox-Rst ohne Tox-Chg?" );
    pFont->GetTox()--;
}

void SwTxtTOXMark::CopyTOXMark( SwDoc* pDoc )
{
    SwTOXMark& rTOX = (SwTOXMark&)GetTOXMark();
    TOXTypes    eType   = rTOX.GetTOXType()->GetType();
    USHORT      nCount  = pDoc->GetTOXTypeCount( eType );
    const SwTOXType* pType = 0;
    const XubString& rNm = rTOX.GetTOXType()->GetTypeName();

    // kein entsprechender Verzeichnistyp vorhanden -> anlegen
    // sonst verwenden
    for(USHORT i=0; i < nCount; ++i)
    {
        const SwTOXType* pSrcType = pDoc->GetTOXType(eType, i);
        if(pSrcType->GetTypeName() == rNm )
        {
            pType = pSrcType;
            break;
        }
    }
    // kein entsprechender Typ vorhanden -> neu erzeugen
    //
    if(!pType)
    {
        pDoc->InsertTOXType( SwTOXType( eType, rNm ) );
        pType = pDoc->GetTOXType(eType, 0);
    }
    // Verzeichnistyp umhaengen
    //
    ((SwTOXType*)pType)->Add( &rTOX );
}



