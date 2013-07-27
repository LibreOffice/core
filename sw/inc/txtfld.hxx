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
#ifndef _TXTFLD_HXX
#define _TXTFLD_HXX

#include <txatbase.hxx>
#include <tools/string.hxx>

class SwTxtNode;

// ATT_FLD ***********************************

class SwTxtFld : public SwTxtAttr
{
    mutable   String m_aExpand;
    SwTxtNode * m_pTxtNode;

public:
    SwTxtFld(SwFmtFld & rAttr, xub_StrLen const nStart,
            bool const bInClipboard);
    virtual ~SwTxtFld();

    void CopyFld( SwTxtFld *pDest ) const;
    void Expand() const;
    inline void ExpandAlways();

    // get and set TxtNode pointer
    SwTxtNode* GetpTxtNode() const { return m_pTxtNode; }
    inline SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }
    // enable notification that field content has changed and needs reformatting
    void NotifyContentChange(SwFmtFld& rFmtFld);

    /**
       Returns position of this field.

       @return position of this field. Has to be deleted explicitly.
    */
};

inline SwTxtNode& SwTxtFld::GetTxtNode() const
{
    assert( m_pTxtNode );
    return *m_pTxtNode;
}

inline void SwTxtFld::ExpandAlways()
{
    m_aExpand += ' ';
    Expand();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
