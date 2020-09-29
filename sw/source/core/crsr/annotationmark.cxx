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

#include <algorithm>
#include <annotationmark.hxx>

#include <doc.hxx>
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

namespace sw::mark
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

    void AnnotationMark::InitDoc(SwDoc& io_rDoc,
            sw::mark::InsertMode const, SwPosition const*const)
    {
        SwTextNode *pTextNode = GetMarkEnd().nNode.GetNode().GetTextNode();
        assert(pTextNode);
        SwTextField *const pTextField = pTextNode->GetFieldTextAttrAt(
            GetMarkEnd().nContent.GetIndex()-1, true);
        assert(pTextField != nullptr);
        auto pPostItField
            = dynamic_cast<const SwPostItField*>(pTextField->GetFormatField().GetField());
        assert(pPostItField);
        // use the annotation mark's name as the annotation name, if
        // - the annotation field has an empty annotation name or
        // - the annotation mark's name differs (on mark creation a name clash had been detected)
        if ( pPostItField->GetName().isEmpty()
            || pPostItField->GetName() != GetName() )
        {
            const_cast<SwPostItField*>(pPostItField)->SetName( GetName() );
        }

        if (io_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            io_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoInsBookmark>(*this) );
        }
        io_rDoc.getIDocumentState().SetModified();
    }

    const SwFormatField* AnnotationMark::GetAnnotationFormatField() const
    {
        SwDoc* pDoc = GetMarkPos().GetDoc();
        assert(pDoc != nullptr);

        const auto sName = GetName();
        SwFieldType* pType = pDoc->getIDocumentFieldsAccess().GetFieldType( SwFieldIds::Postit, OUString(), false );
        std::vector<SwFormatField*> vFields;
        pType->GatherFields(vFields);
        auto ppFound = std::find_if(vFields.begin(), vFields.end(), [&sName](SwFormatField* pF)
        {
            auto pPF = dynamic_cast<const SwPostItField*>(pF->GetField());
            return pPF && pPF->GetName() == sName;
        });
        return ppFound != vFields.end() ? *ppFound : nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
