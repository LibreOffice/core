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

#include <svl/svarray.hxx>
#include <svtools/textdata.hxx>
#include <txtattr.hxx>

#include <tools/debug.hxx>
#include <tools/string.hxx>

typedef TextCharAttrib* TextCharAttribPtr;
SV_DECL_PTRARR_DEL( TextCharAttribs, TextCharAttribPtr, 0, 4 )

class TextCharAttribList : private TextCharAttribs
{
private:
    BOOL            mbHasEmptyAttribs;

                    TextCharAttribList( const TextCharAttribList& ) : TextCharAttribs() {}

public:
                    TextCharAttribList();
                    ~TextCharAttribList();

    void            Clear( BOOL bDestroyAttribs );
    USHORT          Count() const               { return TextCharAttribs::Count(); }

    TextCharAttrib* GetAttrib( USHORT n ) const { return TextCharAttribs::GetObject( n ); }
    void            RemoveAttrib( USHORT n )    { TextCharAttribs::Remove( n, 1 ); }

    void            InsertAttrib( TextCharAttrib* pAttrib );

    void            DeleteEmptyAttribs();
    void            ResortAttribs();

    BOOL            HasEmptyAttribs() const { return mbHasEmptyAttribs; }
    BOOL&           HasEmptyAttribs()       { return mbHasEmptyAttribs; }

    TextCharAttrib* FindAttrib( USHORT nWhich, USHORT nPos );
    TextCharAttrib* FindNextAttrib( USHORT nWhich, USHORT nFromPos, USHORT nMaxPos = 0xFFFF ) const;
    TextCharAttrib* FindEmptyAttrib( USHORT nWhich, USHORT nPos );
    BOOL            HasAttrib( USHORT nWhich ) const;
    BOOL            HasBoundingAttrib( USHORT nBound );

#ifdef  DBG_UTIL
    BOOL            DbgCheckAttribs();
#endif
};


class TextNode
{
private:
    String              maText;
    TextCharAttribList  maCharAttribs;

                        TextNode( const TextNode& ) {;}
protected:
    void                ExpandAttribs( USHORT nIndex, USHORT nNewChars );
    void                CollapsAttribs( USHORT nIndex, USHORT nDelChars );

public:
                        TextNode( const String& rText );


    const String&               GetText() const         { return maText; }

    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( USHORT nPos, const String& rText );
    void                InsertText( USHORT nPos, sal_Unicode c );
    void                RemoveText( USHORT nPos, USHORT nChars );

    TextNode*           Split( USHORT nPos, BOOL bKeepEndigAttribs );
    void                Append( const TextNode& rNode );
};

class TextDoc
{
private:
    ToolsList<TextNode*> maTextNodes;
    USHORT              mnLeftMargin;

protected:
    void                DestroyTextNodes();

public:
                        TextDoc();
                        ~TextDoc();

    void                Clear();

    ToolsList<TextNode*>&       GetNodes()              { return maTextNodes; }
    const ToolsList<TextNode*>& GetNodes() const        { return maTextNodes; }

    TextPaM             RemoveChars( const TextPaM& rPaM, USHORT nChars );
    TextPaM             InsertText( const TextPaM& rPaM, sal_Unicode c );
    TextPaM             InsertText( const TextPaM& rPaM, const String& rStr );

    TextPaM             InsertParaBreak( const TextPaM& rPaM, BOOL bKeepEndingAttribs );
    TextPaM             ConnectParagraphs( TextNode* pLeft, TextNode* pRight );

    ULONG               GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel = NULL ) const;
    String              GetText( const sal_Unicode* pSep ) const;
    String              GetText( ULONG nPara ) const;

    void                SetLeftMargin( USHORT n )   { mnLeftMargin = n; }
    USHORT              GetLeftMargin() const       { return mnLeftMargin; }

//  BOOL                RemoveAttribs( TextNode* pNode, USHORT nStart, USHORT nEnd ), USHORT nWhich = 0 );
//  BOOL                RemoveAttribs( TextNode* pNode, USHORT nStart, USHORT nEnd, TextCharAttrib*& rpStarting, TextCharAttrib*& rpEnding, USHORT nWhich = 0 );
//  void                InsertAttrib( const EditCharAttrib* pAttr );
//  void                InsertAttribInSelection( const EditCharAttrib* pAttr );
//  void                FindAttribs( TextNode* pNode, USHORT nStartPos, USHORT nEndPos, SfxItemSet& rCurSet );

    BOOL                IsValidPaM( const TextPaM& rPaM );
};

#endif // _TEXTDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
