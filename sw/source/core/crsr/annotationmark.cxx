/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <annotationmark.hxx>

#include <doc.hxx>
#include <IDocumentMarkAccess.hxx>
#include <fldbas.hxx>
#include <switerator.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <IDocumentUndoRedo.hxx>
#include <UndoBookmark.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>

namespace sw { namespace mark
{
    AnnotationMark::AnnotationMark(
        const SwPaM& rPaM,
        const ::rtl::OUString& rName )
        : MarkBase( rPaM, rName )
    {
        if ( rName.getLength() == 0 )
        {
            SetName( MarkBase::GenerateNewName( ::rtl::OUString::createFromAscii("__Annotation__") ) );
        }
    }


    AnnotationMark::~AnnotationMark()
    {
    }


    void AnnotationMark::InitDoc(SwDoc* const io_pDoc)
    {
        SwTxtFld* pTxtFld =
            GetMarkEnd().nNode.GetNode().GetTxtNode()->GetFldTxtAttrAt(
            GetMarkEnd().nContent.GetIndex()-1, true );
        if ( pTxtFld != NULL )
        {
            const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pTxtFld->GetFmtFld().GetField());
            ASSERT( pPostItField != NULL, "<AnnotationMark::InitDoc(..)> - annotation field missing!" );
            if ( pPostItField != NULL )
            {
                // use the annotation mark's name as the annotation name, if
                // - the annotation field has an empty annotation name or
                // - the annotation mark's name differs (on mark creation a name clash had been detected)
                if ( pPostItField->GetName().Len() == 0
                    || rtl::OUString( pPostItField->GetName() ) != GetName() )
                {
                    const_cast<SwPostItField*>(pPostItField)->SetName( GetName() );
                }
            }
        }

        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo( new SwUndoInsBookmark(*this) );
        }
        io_pDoc->SetModified();
    }


    const SwFmtFld* AnnotationMark::GetAnnotationFmtFld() const
    {
        SwDoc* pDoc = GetMarkPos().GetDoc();
        if ( pDoc == NULL )
        {
            ASSERT( false, "<AnnotationMark::GetAnnotationFmtFld()> - missing document at annotation mark" );
            return NULL;
        }

        SwFmtFld* pAnnotationFmtFld = NULL;

        SwFieldType* pType = pDoc->GetFldType( RES_POSTITFLD, String(), false );
        SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld != NULL; pFmtFld = aIter.Next() )
        {
            if ( pFmtFld->IsFldInDoc() )
            {
                const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pFmtFld->GetField());
                if ( pPostItField != NULL
                     && rtl::OUString( pPostItField->GetName() ) == GetName() )
                {
                    pAnnotationFmtFld = pFmtFld;
                    break;
                }
            }
        }

        return pAnnotationFmtFld;
    }
}}

