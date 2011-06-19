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

#include <vector>

#include <tools/link.hxx>

class Paragraph;

class ParagraphList
{
public:
    void            Clear( sal_Bool bDestroyParagraphs );

    sal_uInt32      GetParagraphCount() const
    {
        return maEntries.size();
    }

    Paragraph*      GetParagraph( sal_uLong nPos ) const
    {
        return nPos < maEntries.size() ? maEntries[nPos] : NULL;
    }

    sal_uLong           GetAbsPos( Paragraph* pParent ) const;
    sal_uLong           GetVisPos( Paragraph* pParagraph ) const;

    void            Append( Paragraph *pPara);
    void            Insert( Paragraph* pPara, sal_uLong nAbsPos);
    void            Remove( sal_uLong nPara );
    void            MoveParagraphs( sal_uLong nStart, sal_uLong nDest, sal_uLong nCount );

    Paragraph*      NextVisible( Paragraph* ) const;
    Paragraph*      PrevVisible( Paragraph* ) const;
    Paragraph*      LastVisible() const;

    Paragraph*      GetParent( Paragraph* pParagraph /*, sal_uInt16& rRelPos */ ) const;
    sal_Bool            HasChilds( Paragraph* pParagraph ) const;
    sal_Bool            HasHiddenChilds( Paragraph* pParagraph ) const;
    sal_Bool            HasVisibleChilds( Paragraph* pParagraph ) const;
    sal_uLong           GetChildCount( Paragraph* pParagraph ) const;

    void            Expand( Paragraph* pParent );
    void            Collapse( Paragraph* pParent );

    void            SetVisibleStateChangedHdl( const Link& rLink ) { aVisibleStateChangedHdl = rLink; }
    Link            GetVisibleStateChangedHdl() const { return aVisibleStateChangedHdl; }

private:

    Link aVisibleStateChangedHdl;
    std::vector<Paragraph*> maEntries;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
