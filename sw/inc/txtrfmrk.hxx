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
#ifndef _TXTRFMRK_HXX
#define _TXTRFMRK_HXX

#include <txatbase.hxx>

class SwTxtNode;

// SWTXT_REFMARK *************************************
// Attribute for content-/position references in text.

class SwTxtRefMark : public SwTxtAttrEnd
{
    SwTxtNode * m_pTxtNode;
    xub_StrLen * m_pEnd; // end is optional (point reference)

public:
    SwTxtRefMark( SwFmtRefMark& rAttr,
            xub_StrLen const nStart, xub_StrLen const*const pEnd = 0);

    virtual xub_StrLen * GetEnd();       // SwTxtAttr
    inline const xub_StrLen * GetEnd() const { return m_pEnd; }

    // get and set TxtNode pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }
};

inline const SwTxtNode& SwTxtRefMark::GetTxtNode() const
{
    assert( m_pTxtNode );
    return *m_pTxtNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
