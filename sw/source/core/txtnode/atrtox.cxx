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

#include <doc.hxx>
#include <txttxmrk.hxx>
#include <swfont.hxx>
#include <tox.hxx>
#include <ndtxt.hxx>

SwTextTOXMark::SwTextTOXMark( SwTOXMark& rAttr,
            sal_Int32 const nStartPos, sal_Int32 const*const pEnd)
    : SwTextAttr( rAttr, nStartPos )
    , SwTextAttrEnd( rAttr, nStartPos, nStartPos )
    , m_pTextNode( nullptr )
    , m_pEnd( nullptr )
{
    rAttr.m_pTextAttr = this;
    if ( rAttr.GetAlternativeText().isEmpty() )
    {
        m_nEnd = *pEnd;
        m_pEnd = & m_nEnd;
    }
    else
    {
        SetHasDummyChar(true);
    }
    SetDontMoveAttr( true );
    SetOverlapAllowedAttr( true );
}

SwTextTOXMark::~SwTextTOXMark()
{
}

sal_Int32* SwTextTOXMark::GetEnd()
{
    return m_pEnd;
}

void SwTextTOXMark::CopyTOXMark( SwDoc* pDoc )
{
    SwTOXMark& rTOX = (SwTOXMark&)GetTOXMark();
    TOXTypes    eType   = rTOX.GetTOXType()->GetType();
    const sal_uInt16 nCount = pDoc->GetTOXTypeCount( eType );
    const SwTOXType* pType = nullptr;
    const OUString rNm = rTOX.GetTOXType()->GetTypeName();

    // kein entsprechender Verzeichnistyp vorhanden -> anlegen
    // sonst verwenden
    for(sal_uInt16 i=0; i < nCount; ++i)
    {
        const SwTOXType* pSrcType = pDoc->GetTOXType(eType, i);
        if(pSrcType->GetTypeName() == rNm )
        {
            pType = pSrcType;
            break;
        }
    }
    // kein entsprechender Typ vorhanden -> neu erzeugen

    if(!pType)
    {
        pDoc->InsertTOXType( SwTOXType( eType, rNm ) );
        pType = pDoc->GetTOXType(eType, 0);
    }
    // Verzeichnistyp umhaengen

    const_cast<SwTOXType*>(pType)->Add( &rTOX );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
