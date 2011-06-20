/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _TXTFLD_HXX
#define _TXTFLD_HXX

#include <txatbase.hxx>
#include <tools/string.hxx>
#include <pam.hxx>

class SwTxtNode;

// ATT_FLD ***********************************

class SwTxtFld : public SwTxtAttr
{
    mutable   String m_aExpand;
    SwTxtNode * m_pTxtNode;

public:
    SwTxtFld(SwFmtFld & rAttr, xub_StrLen const nStart);
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
    OSL_ENSURE( m_pTxtNode, "SwTxtFld:: where is my TxtNode?" );
    return *m_pTxtNode;
}

inline void SwTxtFld::ExpandAlways()
{
    m_aExpand += ' ';
    Expand();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
