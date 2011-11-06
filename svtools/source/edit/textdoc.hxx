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



#ifndef _TEXTDOC_HXX
#define _TEXTDOC_HXX

#include <svl/svarray.hxx>
#include <svtools/textdata.hxx>
#include <svtools/txtattr.hxx>

#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>

typedef TextCharAttrib* TextCharAttribPtr;
SV_DECL_PTRARR_DEL( TextCharAttribs, TextCharAttribPtr, 0, 4 )

class TextCharAttribList : private TextCharAttribs
{
private:
    sal_Bool            mbHasEmptyAttribs;

                    TextCharAttribList( const TextCharAttribList& ) : TextCharAttribs() {}

public:
                    TextCharAttribList();
                    ~TextCharAttribList();

    void            Clear( sal_Bool bDestroyAttribs );
    sal_uInt16          Count() const               { return TextCharAttribs::Count(); }

    TextCharAttrib* GetAttrib( sal_uInt16 n ) const { return TextCharAttribs::GetObject( n ); }
    void            RemoveAttrib( sal_uInt16 n )    { TextCharAttribs::Remove( n, 1 ); }

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

#ifdef  DBG_UTIL
    sal_Bool            DbgCheckAttribs();
#endif
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

//  sal_Bool                RemoveAttribs( TextNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd ), sal_uInt16 nWhich = 0 );
//  sal_Bool                RemoveAttribs( TextNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, TextCharAttrib*& rpStarting, TextCharAttrib*& rpEnding, sal_uInt16 nWhich = 0 );
//  void                InsertAttrib( const EditCharAttrib* pAttr );
//  void                InsertAttribInSelection( const EditCharAttrib* pAttr );
//  void                FindAttribs( TextNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, SfxItemSet& rCurSet );

    sal_Bool                IsValidPaM( const TextPaM& rPaM );
};

#endif // _TEXTDOC_HXX
