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
    String              maText;
    TextCharAttribList  maCharAttribs;

                        TextNode( const TextNode& ) {;}
protected:
    void                ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNewChars );
    void                CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDelChars );

public:
                        TextNode( const String& rText );


    const String&               GetText() const         { return maText; }

    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( sal_uInt16 nPos, const String& rText );
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
    TextPaM             InsertText( const TextPaM& rPaM, const String& rStr );

    TextPaM             InsertParaBreak( const TextPaM& rPaM, sal_Bool bKeepEndingAttribs );
    TextPaM             ConnectParagraphs( TextNode* pLeft, TextNode* pRight );

    sal_uLong               GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel = NULL ) const;
    String              GetText( const sal_Unicode* pSep ) const;
    String              GetText( sal_uLong nPara ) const;

    void                SetLeftMargin( sal_uInt16 n )   { mnLeftMargin = n; }
    sal_uInt16              GetLeftMargin() const       { return mnLeftMargin; }

    sal_Bool                IsValidPaM( const TextPaM& rPaM );
};

#endif // _TEXTDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
