/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        OSL_ENSURE( pTxtFld != NULL, "<AnnotationMark::InitDoc(..)> - missing text attribute for annotation field!" );
        if ( pTxtFld != NULL )
        {
            const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pTxtFld->GetFmtFld().GetField());
            OSL_ENSURE( pPostItField != NULL, "<AnnotationMark::InitDoc(..)> - annotation field missing!" );
            if ( pPostItField != NULL )
            {
                // use the annotation mark's name as the annotation name, if
                // - the annotation field has an empty annotation name or
                // - the annotation mark's name differs (on mark creation a name clash had been detected)
                if ( pPostItField->GetName().isEmpty()
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
            OSL_ENSURE( false, "<AnnotationMark::GetAnnotationFmtFld()> - missing document at annotation mark" );
            return NULL;
        }

        SwFmtFld* pAnnotationFmtFld = NULL;

        SwFieldType* pType = pDoc->GetFldType( RES_POSTITFLD, OUString(), false );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
