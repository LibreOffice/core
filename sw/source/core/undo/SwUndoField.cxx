/*************************************************************************
 *
 *  $RCSfile: SwUndoField.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-26 07:54:03 $
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
#include <tools/rtti.hxx>
#include <SwUndoField.hxx>
#include <swundo.hxx>
#include <txtfld.hxx>
#include <fldbas.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <dbfld.hxx>

using namespace com::sun::star::uno;

SwUndoField::SwUndoField(const SwPosition & rPos)
    : SwUndo(UNDO_FIELD)
{
    nNodeIndex = rPos.nNode.GetIndex();
    nOffset = rPos.nContent.GetIndex();
    pDoc = rPos.GetDoc();
}

SwUndoField::~SwUndoField()
{
}

void SwUndoField::RestoreFieldType(SwField * pField,
                                   SwFieldType * pNewType)
{
    SwFieldType * pOldType = pField->GetTyp();
    printf("SwUndoField::RestoreFieldType: %d\n", pField->Which());

    if (RES_DBFLD == pField->Which() && pOldType != pNewType)
    {
        SwClientIter aIter( *pOldType );
        SwDBField * pDBField = (SwDBField *) pField;

        for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
             pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
        {
            if( pFmtFld->GetFld() == pDBField)
            {
                pNewType->Add(pFmtFld);
                pDBField->ChgTyp(pNewType);

                break;
            }
        }
    }
}

SwPosition SwUndoField::GetPosition()
{
    SwNode * pNode = pDoc->GetNodes()[nNodeIndex];
    SwNodeIndex aNodeIndex(*pNode);
    SwIndex aIndex(pNode->GetCntntNode(), nOffset);
    SwPosition aResult(aNodeIndex, aIndex);

    return aResult;
}

static void lcl_AddDBFieldTypeRef(SwField * pField)
{
    if (TYP_DBFLD == pField->Which())
    {
        SwDBFieldType * pFieldType =
            (SwDBFieldType *) ((SwDBField *) pField)->GetTyp();

        pFieldType->AddRef();
    }
}

static void lcl_ReleaseDBFieldTypeRef(SwField * pField)
{
    if (TYP_DBFLD == pField->Which())
    {
        SwDBFieldType * pFieldType =
            (SwDBFieldType *) ((SwDBField *) pField)->GetTyp();

        pFieldType->ReleaseRef();
    }
}

SwUndoFieldFromDoc::SwUndoFieldFromDoc(const SwPosition & rPos,
                                       const SwField & _aOldField,
                         const SwField & _aNewField,
                         SwMsgPoolItem * _pHnt, BOOL _bUpdate)
    : SwUndoField(rPos), pOldField(_aOldField.Copy()),
      pNewField(_aNewField.Copy()), pHnt(_pHnt),
      bUpdate(_bUpdate)
{
    ASSERT(pOldField, "No old field!");
    ASSERT(pNewField, "No new field!");
    ASSERT(pDoc, "No document!");
}

SwUndoFieldFromDoc::~SwUndoFieldFromDoc()
{
    delete pOldField;
    delete pNewField;
}

void SwUndoFieldFromDoc::Undo(SwUndoIter & rIt)
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        BOOL bUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->UpdateFld(pTxtFld, *pOldField, pHnt, bUpdate);
        pDoc->DoUndo(bUndo);
    }
}

void SwUndoFieldFromDoc::Redo(SwUndoIter & rIt)
{
    SwTxtFld * pTxtFld = SwDoc::GetTxtFld(GetPosition());
    const SwField * pField = pTxtFld->GetFld().GetFld();

    if (pField)
    {
        BOOL bUndo = pDoc->DoesUndo();

        pDoc->DoUndo(FALSE);
        pDoc->UpdateFld(pTxtFld, *pNewField, pHnt, bUpdate);
        pDoc->DoUndo(bUndo);
    }
}

void SwUndoFieldFromDoc::Repeat(SwUndoIter & rIt)
{
    Redo(rIt);
}

SwUndoFieldFromAPI::SwUndoFieldFromAPI(const SwPosition & rPos,
                                       const Any & rOldVal, const Any & rNewVal,
                                       BYTE _nMId)
    : SwUndoField(rPos), aOldVal(rOldVal), aNewVal(rNewVal), nMId(_nMId)
{
}

SwUndoFieldFromAPI::~SwUndoFieldFromAPI()
{
}

void SwUndoFieldFromAPI::Undo(SwUndoIter & rIter)
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aOldVal, nMId);
}

void SwUndoFieldFromAPI::Redo(SwUndoIter & rIter)
{
    SwField * pField = SwDoc::GetField(GetPosition());

    if (pField)
        pField->PutValue(aNewVal, nMId);
}


void SwUndoFieldFromAPI::Repeat(SwUndoIter & rIter)
{
    Redo(rIter);
}
