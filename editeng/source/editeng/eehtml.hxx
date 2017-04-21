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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EEHTML_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EEHTML_HXX

#include <memory>
#include <editdoc.hxx>
#include <svtools/parhtml.hxx>

class EditEngine;

struct AnchorInfo
{
    OUString  aHRef;
    OUString  aText;
};

class EditHTMLParser : public HTMLParser
{
    using HTMLParser::CallParser;
private:
    OUStringBuffer          maStyleSource;
    EditSelection           aCurSel;
    OUString                aBaseURL;
    EditEngine*             mpEditEngine;
    std::unique_ptr<AnchorInfo>
                            pCurAnchor;

    bool                    bInPara:1;
    bool                    bWasInPara:1; // Remember bInPara before HeadingStart, because afterwards it will be gone.
    bool                    bFieldsInserted:1;
    bool                    bInTitle:1;

    sal_uInt8               nInTable;
    sal_uInt8               nInCell;
    sal_uInt8               nDefListLevel;

    void                    StartPara( bool bReal );
    void                    EndPara();
    void                    AnchorStart();
    void                    AnchorEnd();
    void                    HeadingStart( HtmlTokenId nToken );
    void                    HeadingEnd();
    void                    SkipGroup( HtmlTokenId nEndToken );
    bool                    ThrowAwayBlank();
    bool                    HasTextInCurrentPara();

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const OUString& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems );
    void                    ImpSetStyleSheet( sal_uInt16 nHeadingLevel );

protected:
    virtual void            NextToken( HtmlTokenId nToken ) override;

public:
    EditHTMLParser(SvStream& rIn, const OUString& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs);
    virtual ~EditHTMLParser() override;

    SvParserState CallParser(EditEngine* pEE, const EditPaM& rPaM);

    const EditSelection&    GetCurSelection() const { return aCurSel; }
};

typedef tools::SvRef<EditHTMLParser> EditHTMLParserRef;

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EEHTML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
