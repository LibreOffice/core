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

#include <editdoc.hxx>
#include <svtools/parhtml.hxx>

class EditEngine;

struct AnchorInfo
{
    String  aHRef;
    String  aText;
};

class EditHTMLParser : public HTMLParser
{
    using HTMLParser::CallParser;
private:
    ::rtl::OUStringBuffer maStyleSource;
    EditSelection           aCurSel;
    String                  aBaseURL;
    EditEngine* mpEditEngine;
    AnchorInfo*             pCurAnchor;

    bool                    bInPara:1;
    bool                    bWasInPara:1; // Remember bInPara before HeadingStart, because afterwards it will be gone.
    bool                    bFieldsInserted:1;
    bool                    bInTitle:1;

    sal_uInt8                   nInTable;
    sal_uInt8                   nInCell;
    sal_uInt8                   nDefListLevel;

    void                    StartPara( bool bReal );
    void                    EndPara( bool bReal );
    void                    AnchorStart();
    void                    AnchorEnd();
    void                    HeadingStart( int nToken );
    void                    HeadingEnd( int nToken );
    void                    SkipGroup( int nEndToken );
    bool                    ThrowAwayBlank();
    bool                    HasTextInCurrentPara();

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const String& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel = 0 );
    void                    ImpSetStyleSheet( sal_uInt16 nHeadingLevel );

protected:
    virtual void            NextToken( int nToken );

public:
    EditHTMLParser(SvStream& rIn, const String& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs);
    ~EditHTMLParser();

    SvParserState CallParser(EditEngine* pEE, const EditPaM& rPaM);

    const EditSelection&    GetCurSelection() const { return aCurSel; }
};

SV_DECL_REF( EditHTMLParser )
SV_IMPL_REF( EditHTMLParser );

#endif // _EEHTML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
