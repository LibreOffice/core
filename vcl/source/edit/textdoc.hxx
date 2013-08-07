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

#ifndef _TEXTDOC_HXX
#define _TEXTDOC_HXX

#include <vcl/textdata.hxx>
#include <vcl/txtattr.hxx>

#include <tools/string.hxx>
#include <vector>

class TextCharAttribs : public std::vector<TextCharAttrib*> {
public:
    ~TextCharAttribs()
    {
        for( iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};

class TextCharAttribList : private TextCharAttribs
{
private:
    sal_Bool            mbHasEmptyAttribs;

                    TextCharAttribList( const TextCharAttribList& ) : TextCharAttribs() {}

public:
                    TextCharAttribList();
                    ~TextCharAttribList();

    void            Clear( sal_Bool bDestroyAttribs );
    sal_uInt16          Count() const               { return TextCharAttribs::size(); }

    TextCharAttrib* GetAttrib( sal_uInt16 n ) const { return TextCharAttribs::operator[]( n ); }
    void            RemoveAttrib( sal_uInt16 n )    { TextCharAttribs::erase( begin() + n ); }

    void            InsertAttrib( TextCharAttrib* pAttrib );

    void            DeleteEmptyAttribs();
    void            ResortAttribs();

    sal_Bool            HasEmptyAttribs() const { return mbHasEmptyAttribs; }
    sal_Bool&           HasEmptyAttribs()       { return mbHasEmptyAttribs; }

    TextCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    TextCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos, sal_uInt16 nMaxPos = 0xFFFF ) const;
    TextCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    sal_Bool            HasAttrib( sal_uInt16 nWhich ) const;
    sal_Bool            HasBoundingAttrib( sal_uInt16 nBound );
};


class TextNode
{
private:
    OUString            maText;
    TextCharAttribList  maCharAttribs;

                        TextNode( const TextNode& ) {;}
protected:
    void                ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNewChars );
    void                CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDelChars );

public:
                        TextNode( const OUString& rText );


    const OUString&     GetText() const         { return maText; }

    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( sal_uInt16 nPos, const OUString& rText );
    void                InsertText( sal_uInt16 nPos, sal_Unicode c );
    void                RemoveText( sal_uInt16 nPos, sal_uInt16 nChars );

    TextNode*           Split( sal_uInt16 nPos, sal_Bool bKeepEndigAttribs );
    void                Append( const TextNode& rNode );
};

class TextDoc
{
private:
    ToolsList<TextNode*> maTextNodes;
    sal_uInt16              mnLeftMargin;

protected:
    void                DestroyTextNodes();

public:
                        TextDoc();
                        ~TextDoc();

    void                Clear();

    ToolsList<TextNode*>&       GetNodes()              { return maTextNodes; }
    const ToolsList<TextNode*>& GetNodes() const        { return maTextNodes; }

    TextPaM             RemoveChars( const TextPaM& rPaM, sal_uInt16 nChars );
    TextPaM             InsertText( const TextPaM& rPaM, sal_Unicode c );
    TextPaM             InsertText( const TextPaM& rPaM, const OUString& rStr );

    TextPaM             InsertParaBreak( const TextPaM& rPaM, sal_Bool bKeepEndingAttribs );
    TextPaM             ConnectParagraphs( TextNode* pLeft, TextNode* pRight );

    sal_uLong           GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel = NULL ) const;
    OUString            GetText( const sal_Unicode* pSep ) const;
    OUString            GetText( sal_uLong nPara ) const;

    void                SetLeftMargin( sal_uInt16 n )   { mnLeftMargin = n; }
    sal_uInt16          GetLeftMargin() const       { return mnLeftMargin; }

    sal_Bool            IsValidPaM( const TextPaM& rPaM );
};

#endif // _TEXTDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
