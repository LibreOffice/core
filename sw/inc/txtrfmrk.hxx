/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _TXTRFMRK_HXX
#define _TXTRFMRK_HXX

#include <txatbase.hxx>

class SwTxtNode;

// SWTXT_REFMARK *************************************
// Attribut fuer Inhalts-/Positions-Referenzen im Text

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
    ASSERT( m_pTxtNode, "SwTxtRefMark: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

