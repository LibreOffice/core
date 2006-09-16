/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoclbck.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 21:55:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
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
/* -----------------------------01.09.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
SwXReferenceMark* SwUnoCallBack::GetRefMark(const SwFmtRefMark& rMark)
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
SwXFootnote* SwUnoCallBack::GetFootnote(const SwFmtFtn& rMark)
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

/* -----------------------------27.11.00 17:15--------------------------------

 ---------------------------------------------------------------------------*/
SwXDocumentIndexMark* SwUnoCallBack::GetTOXMark(const SwTOXMark& rMark)
{
    SwClientIter aIter( *this );
    SwXDocumentIndexMark* pxIndexMark = (SwXDocumentIndexMark*)aIter.First( TYPE( SwXDocumentIndexMark ));
    while(pxIndexMark)
    {
        const SwTOXMark* pMark = pxIndexMark->GetTOXMark();
        if(pMark == &rMark)
            return pxIndexMark;

        pxIndexMark = (SwXDocumentIndexMark*)aIter.Next( );
    }
    return 0;
}

