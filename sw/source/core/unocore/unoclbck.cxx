/*************************************************************************
 *
 *  $RCSfile: unoclbck.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-16 10:31:05 $
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

#include <swtypes.hxx>
#include <hintids.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _UNOCLBCK_HXX
#include <unoclbck.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTRFMRK_HXX
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX
#include <txtrfmrk.hxx>
#endif

/* -----------------------------06.01.00 13:51--------------------------------

 ---------------------------------------------------------------------------*/
SwUnoCallBack::SwUnoCallBack(SwModify *pToRegisterIn)   :
    SwModify(pToRegisterIn)
{
}
/* -----------------------------06.01.00 13:51--------------------------------

 ---------------------------------------------------------------------------*/
SwUnoCallBack::~SwUnoCallBack()
{
}
/* -----------------------------06.01.00 13:51--------------------------------

 ---------------------------------------------------------------------------*/
void SwUnoCallBack::Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue )
{
    switch( pOldValue ? pOldValue->Which() : 0 )
    {
        case  RES_FOOTNOTE_DELETED:
        {
            SwClientIter aIter( *this );
            SwXFootnote* pxFootnote = (SwXFootnote*)aIter.First( TYPE( SwXFootnote ));
            while(pxFootnote)
            {
                const SwFmtFtn* pFmt = pxFootnote->FindFmt();
                if(!pFmt)
                    pxFootnote->Invalidate();
                else
                {
                    const SwTxtFtn *pTxtFtn = pFmt ? pFmt->GetTxtFtn() : 0;
                    if(pTxtFtn && (void*)pTxtFtn == ((SwPtrMsgPoolItem *)pOldValue)->pObject)
                    {
                        pxFootnote->Invalidate();
                        break;
                    }
                }
                pxFootnote = (SwXFootnote*)aIter.Next( );
            }
        }
        break;
        case  RES_REFMARK_DELETED:
        {
            SwClientIter aIter( *this );
            SwXReferenceMark* pxRefMark = (SwXReferenceMark*)aIter.First( TYPE( SwXReferenceMark ));
            while(pxRefMark)
            {
                SwDoc* pDoc = pxRefMark->GetDoc();
                if(pDoc)
                {
                    const SwFmtRefMark* pFmt = pDoc->GetRefMark(pxRefMark->GetMarkName());
                    if(!pFmt)
                        pxRefMark->Invalidate();
                    else if(pFmt == pxRefMark->GetMark())
                    {
                        const SwTxtRefMark *pTxtRef = pFmt ? pFmt->GetTxtRefMark() : 0;
                        if(pTxtRef && (void*)pTxtRef == ((SwPtrMsgPoolItem *)pOldValue)->pObject)
                        {
                            pxRefMark->Invalidate();
                            break;
                        }
                    }
                }
                pxRefMark = (SwXReferenceMark*)aIter.Next( );
            }

        }
        break;
        case  RES_TOXMARK_DELETED:
        {
            SwClientIter aIter( *this );
            SwXDocumentIndexMark* pxIdxMark = (SwXDocumentIndexMark*)aIter.First( TYPE( SwXDocumentIndexMark ));
            while(pxIdxMark)
            {
                SwTOXType* pToxType = pxIdxMark->GetTOXType();
                if(pToxType)
                {
                    const SwTOXMark* pTOXMark = pxIdxMark->GetTOXMark();
                    if(!pTOXMark)
                        pxIdxMark->Invalidate();
                    else
                    {
                        const SwTxtTOXMark* pTxtTOXMark = pTOXMark ? pTOXMark->GetTxtTOXMark() : 0;
                        if(pTxtTOXMark && (void*)pTxtTOXMark == ((SwPtrMsgPoolItem *)pOldValue)->pObject)
                        {
                            pxIdxMark->Invalidate();
                            break;
                        }
                    }
                }
                else
                    pxIdxMark->Invalidate();
                pxIdxMark = (SwXDocumentIndexMark*)aIter.Next( );
            }

        }
        break;
    }
}
/* -----------------------------01.09.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
SwXReferenceMark*   SwUnoCallBack::GetRefMark(const SwFmtRefMark& rMark)
{
    SwClientIter aIter( *this );
    SwXReferenceMark* pxRefMark = (SwXReferenceMark*)aIter.First( TYPE( SwXReferenceMark ));
    while(pxRefMark)
    {
        SwDoc* pDoc = pxRefMark->GetDoc();
        if(pDoc)
        {
            const SwFmtRefMark* pFmt = pDoc->GetRefMark(pxRefMark->GetMarkName());
            if(pFmt == &rMark)
                return pxRefMark;
        }
        pxRefMark = (SwXReferenceMark*)aIter.Next( );
    }
    return 0;
}
/* -----------------------------05.09.00 12:38--------------------------------

 ---------------------------------------------------------------------------*/
SwXFootnote*    SwUnoCallBack::GetFootnote(const SwFmtFtn& rMark)
{
    SwClientIter aIter( *this );
    SwXFootnote* pxFootnote = (SwXFootnote*)aIter.First( TYPE( SwXFootnote ));
    while(pxFootnote)
    {
        SwDoc* pDoc = pxFootnote->GetDoc();
        if(pDoc)
        {
            const SwFmtFtn* pFtn = pxFootnote->FindFmt();
            if(pFtn == &rMark)
                return pxFootnote;
        }
        pxFootnote = (SwXFootnote*)aIter.Next( );
    }
    return 0;
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/19 00:08:28  hr
    initial import

    Revision 1.4  2000/09/18 16:04:31  willem.vandorp
    OpenOffice header added.

    Revision 1.3  2000/09/05 15:12:22  os
    new: GetFootnote()

    Revision 1.2  2000/09/01 14:22:15  os
    new::GetRefMark()

    Revision 1.1  2000/01/07 13:49:03  os
    #67019# #65681# SwXReferenceMarks/SwXFootnotes: Modify via UnoCallBack


------------------------------------------------------------------------*/

