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
#ifndef INCLUDED_SW_INC_UNDOPARAGRAPHSIGNATURE_HXX
#define INCLUDED_SW_INC_UNDOPARAGRAPHSIGNATURE_HXX

#include <undobj.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>

struct SwPosition;
class SwDoc;

/// Undo/Redo Paragraph Signature.
class SwUndoParagraphSigning : public SwUndo
{
private:
    SwDoc* m_pDoc;
    uno::Reference<text::XTextField> m_xField;
    uno::Reference<text::XTextContent> m_xParent;
    OUString m_signature;
    OUString m_usage;
    OUString m_display;
    const bool m_bRemove;

public:
    SwUndoParagraphSigning(const SwPosition& rPos,
                           const uno::Reference<text::XTextField>& xField,
                           const uno::Reference<text::XTextContent>& xParent,
                           const bool bRemove);

    virtual void UndoImpl(::sw::UndoRedoContext&) override;
    virtual void RedoImpl(::sw::UndoRedoContext&) override;
    virtual void RepeatImpl(::sw::RepeatContext&) override;

private:
    void Insert();
    void Remove();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
