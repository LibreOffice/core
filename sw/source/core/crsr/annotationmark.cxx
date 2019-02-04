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
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <fldbas.hxx>
#include <calbck.hxx>
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
        const OUString& rName )
        : MarkBase( rPaM, rName )
    {
        if ( rName.getLength() == 0 )
        {
            SetName( MarkBase::GenerateNewName("__Annotation__") );
        }
    }

    AnnotationMark::~AnnotationMark()
    {
    }

    void AnnotationMark::InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode const)
    {
        SwTextNode *pTextNode = GetMarkEnd().nNode.GetNode().GetTextNode();
        assert(pTextNode);
        SwTextField *const pTextField = pTextNode->GetFieldTextAttrAt(
            GetMarkEnd().nContent.GetIndex()-1, true);
        assert(pTextField != nullptr);
        const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pTextField->GetFormatField().GetField());
        assert(pPostItField != nullptr);
        // use the annotation mark's name as the annotation name, if
        // - the annotation field has an empty annotation name or
        // - the annotation mark's name differs (on mark creation a name clash had been detected)
        if ( pPostItField->GetName().isEmpty()
            || pPostItField->GetName() != GetName() )
        {
            const_cast<SwPostItField*>(pPostItField)->SetName( GetName() );
        }

        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoInsBookmark>(*this) );
        }
        io_pDoc->getIDocumentState().SetModified();
    }

    const SwFormatField* AnnotationMark::GetAnnotationFormatField() const
    {
        SwDoc* pDoc = GetMarkPos().GetDoc();
        assert(pDoc != nullptr);

        SwFormatField* pAnnotationFormatField = nullptr;

        SwFieldType* pType = pDoc->getIDocumentFieldsAccess().GetFieldType( SwFieldIds::Postit, OUString(), false );
        SwIterator<SwFormatField,SwFieldType> aIter( *pType );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField != nullptr; pFormatField = aIter.Next() )
        {
            if ( pFormatField->IsFieldInDoc() )
            {
                const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pFormatField->GetField());
                if (pPostItField != nullptr && pPostItField->GetName() == GetName())
                {
                    pAnnotationFormatField = pFormatField;
                    break;
                }
            }
        }

        return pAnnotationFormatField;
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
