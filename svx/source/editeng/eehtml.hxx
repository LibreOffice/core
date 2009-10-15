/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: eehtml.hxx,v $
 * $Revision: 1.6 $
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
#include <sfx2/sfxhtml.hxx>

class ImpEditEngine;

#define MAX_NUMBERLEVEL         10

struct AnchorInfo
{
    String  aHRef;
    String  aText;
};

#ifndef SVX_LIGHT

class EditHTMLParser : public SfxHTMLParser
{
    using HTMLParser::CallParser;
private:
    EditSelection           aCurSel;
    String                  aBaseURL;
    ImpEditEngine*          pImpEditEngine;
    AnchorInfo*             pCurAnchor;

    BOOL                    bInPara;
    BOOL                    bWasInPara; // bInPara vor HeadingStart merken, weil sonst hinterher weg
    BOOL                    bFieldsInserted;
    BYTE                    nInTable;
    BYTE                    nInCell;

    BYTE                    nDefListLevel;
    BYTE                    nBulletLevel;
    BYTE                    nNumberingLevel;

    BYTE                    nLastAction;

    void                    StartPara( BOOL bReal );
    void                    EndPara( BOOL bReal );
    void                    AnchorStart();
    void                    AnchorEnd();
    void                    HeadingStart( int nToken );
    void                    HeadingEnd( int nToken );
    void                    SkipGroup( int nEndToken );
    BOOL                    ThrowAwayBlank();
    BOOL                    HasTextInCurrentPara();
    void                    ProcessUnknownControl( BOOL bOn );

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const String& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel = 0 );
    void                    ImpSetStyleSheet( USHORT nHeadingLevel );

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

#endif

#endif // _EEHTML_HXX
