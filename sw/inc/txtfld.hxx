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
#ifndef INCLUDED_SW_INC_TXTFLD_HXX
#define INCLUDED_SW_INC_TXTFLD_HXX

#include <txatbase.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include <memory>

class SwPaM;
class SwTextNode;

class SwTextField : public virtual SwTextAttr
{
    mutable OUString m_aExpand; // only used to determine, if field content is changing in <ExpandTextField()>
    SwTextNode * m_pTextNode;

public:
    SwTextField(
        SwFormatField & rAttr,
        sal_Int32 const nStart,
        bool const bInClipboard );

    virtual ~SwTextField();

    void CopyTextField( SwTextField *pDest ) const;

    void ExpandTextField( const bool bForceNotify = false ) const;

    // get and set TextNode pointer
    SwTextNode* GetpTextNode() const
    {
        return m_pTextNode;
    }
    SwTextNode& GetTextNode() const
    {
        OSL_ENSURE( m_pTextNode, "SwTextField:: where is my TextNode?" );
        return *m_pTextNode;
    }
    void ChgTextNode( SwTextNode* pNew )
    {
        m_pTextNode = pNew;
    }

    bool IsFieldInDoc() const;

    // enable notification that field content has changed and needs reformatting
    virtual void NotifyContentChange( SwFormatField& rFormatField );

    // deletes the given field via removing the corresponding text selection from the document's content
    static void DeleteTextField( const SwTextField& rTextField );

    // return text selection for the given field
    static void GetPamForTextField( const SwTextField& rTextField,
                                 std::shared_ptr< SwPaM >& rPamForTextField );

};

class SwTextInputField
    : public SwTextAttrNesting
    , public SwTextField
{
public:
    SwTextInputField(
        SwFormatField & rAttr,
        sal_Int32 const nStart,
        sal_Int32 const nEnd,
        bool const bInClipboard );

    virtual ~SwTextInputField();

    void LockNotifyContentChange();
    void UnlockNotifyContentChange();
    virtual void NotifyContentChange( SwFormatField& rFormatField ) override;

    void UpdateTextNodeContent( const OUString& rNewContent );

    const OUString GetFieldContent() const;
    void UpdateFieldContent();

private:

    bool m_bLockNotifyContentChange;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
