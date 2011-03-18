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
#ifndef _TXTFTN_HXX
#define _TXTFTN_HXX

#include <txatbase.hxx>

class SwNodeIndex;
class SwTxtNode;
class SwNodes;
class SwDoc;

// ATT_FTN **********************************************************

class SW_DLLPUBLIC SwTxtFtn : public SwTxtAttr
{
    SwNodeIndex * m_pStartNode;
    SwTxtNode * m_pTxtNode;
    sal_uInt16 m_nSeqNo;

public:
    SwTxtFtn( SwFmtFtn& rAttr, xub_StrLen nStart );
    virtual ~SwTxtFtn();

    inline SwNodeIndex *GetStartNode() const { return m_pStartNode; }
    void SetStartNode( const SwNodeIndex *pNode, sal_Bool bDelNodes = sal_True );
    void SetNumber( const sal_uInt16 nNumber, const String* = 0 );
    void CopyFtn(SwTxtFtn & rDest, SwTxtNode & rDestNode) const;

    // get and set TxtNode pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( SwTxtNode* pNew ) { m_pTxtNode = pNew; }

        // lege eine neue leere TextSection fuer diese Fussnote an
    void MakeNewTextSection( SwNodes& rNodes );

        // loesche die FtnFrame aus der Seite
    void DelFrms();
        // bedingten Absatzvorlagen checken
    void CheckCondColl();

        // fuer die Querverweise auf Fussnoten
    sal_uInt16 SetSeqRefNo();
    void SetSeqNo( sal_uInt16 n )       { m_nSeqNo = n; }   // for Readers
    sal_uInt16 GetSeqRefNo() const      { return m_nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
};

inline const SwTxtNode& SwTxtFtn::GetTxtNode() const
{
    OSL_ENSURE( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    return *m_pTxtNode;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
