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

#ifndef INCLUDED_VCL_SOURCE_EDIT_TEXTDOC_HXX
#define INCLUDED_VCL_SOURCE_EDIT_TEXTDOC_HXX

#include <rtl/ustring.hxx>
#include <vcl/textdata.hxx>
#include <vcl/txtattr.hxx>
#include <vector>
#include <memory>

class TextCharAttribList
{
private:
    TextCharAttribList(const TextCharAttribList&) = delete;
    TextCharAttribList& operator=(const TextCharAttribList&) = delete;

    std::vector<std::unique_ptr<TextCharAttrib> > maAttribs;
    bool            mbHasEmptyAttribs;

public:
                    TextCharAttribList();
                    ~TextCharAttribList();

    void            Clear();
    sal_uInt16          Count() const               { return maAttribs.size(); }

    const TextCharAttrib& GetAttrib( sal_uInt16 n ) const { return *maAttribs[n].get(); }
    TextCharAttrib& GetAttrib( sal_uInt16 n )       { return *maAttribs[n].get(); }
    void            RemoveAttrib( sal_uInt16 n )    { maAttribs[n].release(); maAttribs.erase( maAttribs.begin() + n ); }

    void            InsertAttrib( TextCharAttrib* pAttrib );

    void            DeleteEmptyAttribs();
    void            ResortAttribs();

    bool&           HasEmptyAttribs()       { return mbHasEmptyAttribs; }

    TextCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos );
    const TextCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_Int32 nFromPos, sal_Int32 nMaxPos ) const;
    TextCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos );
    bool            HasAttrib( sal_uInt16 nWhich ) const;
    bool            HasBoundingAttrib( sal_Int32 nBound );
};

class TextNode
{
private:
    OUString            maText;
    TextCharAttribList  maCharAttribs;

protected:
    void                ExpandAttribs( sal_Int32 nIndex, sal_Int32 nNewChars );
    void                CollapsAttribs( sal_Int32 nIndex, sal_Int32 nDelChars );

public:
                        TextNode( const OUString& rText );

    TextNode( const TextNode& ) = delete;
    void operator=( const TextNode& ) = delete;

    const OUString&     GetText() const         { return maText; }

    const TextCharAttrib&   GetCharAttrib(sal_uInt16 nPos) const  { return maCharAttribs.GetAttrib(nPos); }
    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( sal_Int32 nPos, const OUString& rText );
    void                InsertText( sal_Int32 nPos, sal_Unicode c );
    void                RemoveText( sal_Int32 nPos, sal_Int32 nChars );

    TextNode*           Split( sal_Int32 nPos );
    void                Append( const TextNode& rNode );
};

class TextDoc
{
private:
    std::vector<TextNode*>  maTextNodes;
    sal_uInt16              mnLeftMargin;

protected:
    void                DestroyTextNodes();

public:
                        TextDoc();
                        ~TextDoc();

    void                Clear();

    std::vector<TextNode*>&       GetNodes()              { return maTextNodes; }
    const std::vector<TextNode*>& GetNodes() const        { return maTextNodes; }

    void                RemoveChars( const TextPaM& rPaM, sal_Int32 nChars );
    TextPaM             InsertText( const TextPaM& rPaM, sal_Unicode c );
    TextPaM             InsertText( const TextPaM& rPaM, const OUString& rStr );

    TextPaM             InsertParaBreak( const TextPaM& rPaM );
    TextPaM             ConnectParagraphs( TextNode* pLeft, TextNode* pRight );

    sal_Int32           GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel = nullptr ) const;
    OUString            GetText( const sal_Unicode* pSep ) const;
    OUString            GetText( sal_uInt32 nPara ) const;

    void                SetLeftMargin( sal_uInt16 n )   { mnLeftMargin = n; }
    sal_uInt16          GetLeftMargin() const       { return mnLeftMargin; }

    bool            IsValidPaM( const TextPaM& rPaM );
};

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
