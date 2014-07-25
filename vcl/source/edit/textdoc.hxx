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
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class TextCharAttribList : boost::noncopyable
{
private:
    typedef boost::ptr_vector<TextCharAttrib> TextCharAttribs;
    TextCharAttribs maAttribs;
    bool            mbHasEmptyAttribs;

public:
                    TextCharAttribList();
                    ~TextCharAttribList();

    void            Clear();
    sal_uInt16          Count() const               { return maAttribs.size(); }

    const TextCharAttrib& GetAttrib( sal_uInt16 n ) const { return maAttribs[n]; }
    TextCharAttrib* GetAttrib( sal_uInt16 n )       { return &maAttribs[n]; }
    void            RemoveAttrib( sal_uInt16 n )    { maAttribs.release( maAttribs.begin() + n ).release(); }

    void            InsertAttrib( TextCharAttrib* pAttrib );

    void            DeleteEmptyAttribs();
    void            ResortAttribs();

    bool            HasEmptyAttribs() const { return mbHasEmptyAttribs; }
    bool&           HasEmptyAttribs()       { return mbHasEmptyAttribs; }

    TextCharAttrib* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    const TextCharAttrib* FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos, sal_uInt16 nMaxPos = 0xFFFF ) const;
    TextCharAttrib* FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos );
    bool            HasAttrib( sal_uInt16 nWhich ) const;
    bool            HasBoundingAttrib( sal_uInt16 nBound );
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

    const TextCharAttrib&   GetCharAttrib(sal_uInt16 nPos) const  { return maCharAttribs.GetAttrib(nPos); }
    const TextCharAttribList&   GetCharAttribs() const  { return maCharAttribs; }
    TextCharAttribList&         GetCharAttribs()        { return maCharAttribs; }

    void                InsertText( sal_uInt16 nPos, const OUString& rText );
    void                InsertText( sal_uInt16 nPos, sal_Unicode c );
    void                RemoveText( sal_uInt16 nPos, sal_uInt16 nChars );

    TextNode*           Split( sal_uInt16 nPos, bool bKeepEndigAttribs );
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

    TextPaM             InsertParaBreak( const TextPaM& rPaM, bool bKeepEndingAttribs );
    TextPaM             ConnectParagraphs( TextNode* pLeft, TextNode* pRight );

    sal_uLong           GetTextLen( const sal_Unicode* pSep, const TextSelection* pSel = NULL ) const;
    OUString            GetText( const sal_Unicode* pSep ) const;
    OUString            GetText( sal_uLong nPara ) const;

    void                SetLeftMargin( sal_uInt16 n )   { mnLeftMargin = n; }
    sal_uInt16          GetLeftMargin() const       { return mnLeftMargin; }

    bool            IsValidPaM( const TextPaM& rPaM );
};

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
