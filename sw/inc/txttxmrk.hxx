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
#ifndef INCLUDED_SW_INC_TXTTXMRK_HXX
#define INCLUDED_SW_INC_TXTTXMRK_HXX

#include <txatbase.hxx>

class SwTextNode;
class SwDoc;

// Attribute for indices/tables of contents.
class SwTextTOXMark : public SwTextAttrEnd
{
    SwTextNode * m_pTextNode;
    sal_Int32 * m_pEnd;   // 0 if SwTOXMark without AlternativeText

public:
    SwTextTOXMark( SwTOXMark& rAttr,
            sal_Int32 const nStart, sal_Int32 const*const pEnd = 0);
    virtual ~SwTextTOXMark();

    virtual sal_Int32 *GetEnd() override;     // SwTextAttr

    void CopyTOXMark( SwDoc* pDestDoc );

    // get and set TextNode pointer
    inline const SwTextNode* GetpTextNd() const { return m_pTextNode; }
    inline const SwTextNode& GetTextNode() const;
    void ChgTextNode( SwTextNode* pNew ) { m_pTextNode = pNew; }
};

inline const SwTextNode& SwTextTOXMark::GetTextNode() const
{
    assert( m_pTextNode );
    return *m_pTextNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
