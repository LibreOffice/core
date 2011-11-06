/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_Bool                    bWasInPara; // bInPara vor HeadingStart merken, weil sonst hinterher weg
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
