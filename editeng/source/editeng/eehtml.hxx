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

#ifndef _EEHTML_HXX
#define _EEHTML_HXX

#include <svl/svarray.hxx>

#include <editdoc.hxx>
#include <svtools/parhtml.hxx>

class ImpEditEngine;

#define MAX_NUMBERLEVEL         10

struct AnchorInfo
{
    String  aHRef;
    String  aText;
};

class EditHTMLParser : public HTMLParser
{
    using HTMLParser::CallParser;
private:
    EditSelection           aCurSel;
    String                  aBaseURL;
    ImpEditEngine*          pImpEditEngine;
    AnchorInfo*             pCurAnchor;

    sal_Bool                    bInPara;
    sal_Bool                    bWasInPara; // Remember bInPara before HeadingStart, because afterwards it will be gone.
    sal_Bool                    bFieldsInserted;
    sal_uInt8                   nInTable;
    sal_uInt8                   nInCell;
    sal_Bool                    bInTitle;

    sal_uInt8                   nDefListLevel;
    sal_uInt8                   nBulletLevel;
    sal_uInt8                   nNumberingLevel;

    sal_uInt8                   nLastAction;

    void                    StartPara( sal_Bool bReal );
    void                    EndPara( sal_Bool bReal );
    void                    AnchorStart();
    void                    AnchorEnd();
    void                    HeadingStart( int nToken );
    void                    HeadingEnd( int nToken );
    void                    SkipGroup( int nEndToken );
    sal_Bool                    ThrowAwayBlank();
    sal_Bool                    HasTextInCurrentPara();
    void                    ProcessUnknownControl( sal_Bool bOn );

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const String& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel = 0 );
    void                    ImpSetStyleSheet( sal_uInt16 nHeadingLevel );

protected:
    virtual void            NextToken( int nToken );

public:
                            EditHTMLParser( SvStream& rIn, const String& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs );
                            ~EditHTMLParser();

    virtual SvParserState   CallParser( ImpEditEngine* pImpEE, const EditPaM& rPaM );

    const EditSelection&    GetCurSelection() const { return aCurSel; }
};

SV_DECL_REF( EditHTMLParser )
SV_IMPL_REF( EditHTMLParser );

#endif // _EEHTML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
