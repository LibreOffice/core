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

#ifndef _PARALIST_HXX
#define _PARALIST_HXX

class Paragraph;

#include <tools/list.hxx>
#include <tools/link.hxx>

class ParagraphList : private List
{
private:
    Link            aVisibleStateChangedHdl;

public:
    void            Clear( BOOL bDestroyParagraphs );

    ULONG           GetParagraphCount() const           { return List::Count(); }
    Paragraph*      GetParagraph( ULONG nPos ) const    { return (Paragraph*)List::GetObject( nPos ); }

    ULONG           GetAbsPos( Paragraph* pParent ) const { return List::GetPos( pParent ); }
    ULONG           GetVisPos( Paragraph* pParagraph );

    void            Insert( Paragraph* pPara, ULONG nAbsPos = LIST_APPEND ) { List::Insert( pPara, nAbsPos ); }
    void            Remove( ULONG nPara ) { List::Remove( nPara ); }
    void            MoveParagraphs( ULONG nStart, ULONG nDest, ULONG nCount );

    Paragraph*      NextVisible( Paragraph* ) const;
    Paragraph*      PrevVisible( Paragraph* ) const;
    Paragraph*      LastVisible() const;

    Paragraph*      GetParent( Paragraph* pParagraph /*, USHORT& rRelPos */ ) const;
    BOOL            HasChilds( Paragraph* pParagraph ) const;
    BOOL            HasHiddenChilds( Paragraph* pParagraph ) const;
    BOOL            HasVisibleChilds( Paragraph* pParagraph ) const;
    ULONG           GetChildCount( Paragraph* pParagraph ) const;

    void            Expand( Paragraph* pParent );
    void            Collapse( Paragraph* pParent );

    void            SetVisibleStateChangedHdl( const Link& rLink ) { aVisibleStateChangedHdl = rLink; }
    Link            GetVisibleStateChangedHdl() const { return aVisibleStateChangedHdl; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
