/*************************************************************************
 *
 *  $RCSfile: textdoc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TEXTDOC_HXX
#define _TEXTDOC_HXX

#ifndef _SVARRAY_HXX
#include <svarray.hxx>
#endif

#ifndef _TEXTDATA_HXX
#include <textdata.hxx>
#endif

#ifndef _TXTATTR_HXX
#include <txtattr.hxx>
#endif

#include <tools/debug.hxx>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif


typedef TextCharAttrib* TextCharAttribPtr;
SV_DECL_PTRARR_DEL( TextCharAttribs, TextCharAttribPtr, 0, 4 );

class TextCharAttribList : private TextCharAttribs
{
private:
    BOOL            mbHasEmptyAttribs;

                    TextCharAttribList( const TextCharAttribList& ) {;}

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
//  String&                     GetText()               { return maText; }

    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( USHORT nPos, const String& rText );
    void                InsertText( USHORT nPos, sal_Unicode c );
    void                RemoveText( USHORT nPos, USHORT nChars );

    TextNode*           Split( USHORT nPos, BOOL bKeepEndigAttribs );
    void                Append( const TextNode& rNode );
};



DECLARE_LIST( TextNodeList, TextNode* );

class TextDoc
{
private:
    TextNodeList        maTextNodes;
    USHORT              mnLeftMargin;

protected:
    void                DestroyTextNodes();

public:
                        TextDoc();
                        ~TextDoc();

    void                Clear();

    TextNodeList&       GetNodes()              { return maTextNodes; }
    const TextNodeList& GetNodes() const        { return maTextNodes; }

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
