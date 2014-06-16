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

#include <boost/shared_ptr.hpp>

class SwPaM;
class SwTxtNode;

class SwTxtFld : public SwTxtAttr
{
    mutable OUString m_aExpand; // only used to determine, if field content is changing in <ExpandTxtFld()>
    SwTxtNode * m_pTxtNode;

public:
    SwTxtFld(
        SwFmtFld & rAttr,
        sal_Int32 const nStart,
        bool const bInClipboard );

    virtual ~SwTxtFld();

    void CopyTxtFld( SwTxtFld *pDest ) const;

    void ExpandTxtFld( const bool bForceNotify = false ) const;

    // get and set TxtNode pointer
    SwTxtNode* GetpTxtNode() const
    {
        return m_pTxtNode;
    }
    SwTxtNode& GetTxtNode() const
    {
        OSL_ENSURE( m_pTxtNode, "SwTxtFld:: where is my TxtNode?" );
        return *m_pTxtNode;
    }
    void ChgTxtNode( SwTxtNode* pNew )
    {
        m_pTxtNode = pNew;
    }

    bool IsFldInDoc() const;

    // enable notification that field content has changed and needs reformatting
    virtual void NotifyContentChange( SwFmtFld& rFmtFld );

    // deletes the given field via removing the corresponding text selection from the document's content
    static void DeleteTxtFld( const SwTxtFld& rTxtFld );

    // return text selection for the given field
    static void GetPamForTxtFld( const SwTxtFld& rTxtFld,
                                 boost::shared_ptr< SwPaM >& rPamForTxtFld );

};

class SwTxtInputFld : public SwTxtFld
{
public:
    SwTxtInputFld(
        SwFmtFld & rAttr,
        sal_Int32 const nStart,
        sal_Int32 const nEnd,
        bool const bInClipboard );

    virtual ~SwTxtInputFld();

    virtual sal_Int32* GetEnd() SAL_OVERRIDE;

    void LockNotifyContentChange();
    void UnlockNotifyContentChange();
    virtual void NotifyContentChange( SwFmtFld& rFmtFld ) SAL_OVERRIDE;

    void UpdateTextNodeContent( const OUString& rNewContent );

    const OUString GetFieldContent() const;
    void UpdateFieldContent();

private:
    sal_Int32 m_nEnd;

    bool m_bLockNotifyContentChange;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
