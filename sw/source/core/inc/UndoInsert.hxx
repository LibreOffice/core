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

#ifndef SW_UNDO_INSERT_HXX
#define SW_UNDO_INSERT_HXX

#include <undobj.hxx>
#include <tools/mempool.hxx>
#include <swtypes.hxx>
#include <IDocumentContentOperations.hxx>

class Graphic;
class SwGrfNode;
class SwUndoDelete;
class SwUndoFmtAttr;

class SwUndoInsert: public SwUndo, private SwUndoSaveCntnt
{
    /// start of Content in UndoNodes for Redo
    ::boost::scoped_ptr<SwNodeIndex> m_pUndoNodeIndex;
    String *pTxt, *pUndoTxt;
    SwRedlineData* pRedlData;
    sal_uLong nNode;
    xub_StrLen nCntnt, nLen;
    sal_Bool bIsWordDelim : 1;
    sal_Bool bIsAppend : 1;
    sal_Bool m_bWithRsid : 1;

    const IDocumentContentOperations::InsertFlags m_nInsertFlags;

    friend class SwDoc;     // eigentlich nur SwDoc::Insert( String )
    sal_Bool CanGrouping( sal_Unicode cIns );
    sal_Bool CanGrouping( const SwPosition& rPos );

    SwDoc * pDoc;

    void Init(const SwNodeIndex & rNode);
    String * GetTxtFromDoc() const;

public:
    SwUndoInsert( const SwNodeIndex& rNode, xub_StrLen nCntnt, xub_StrLen nLen,
                  const IDocumentContentOperations::InsertFlags nInsertFlags,
                  sal_Bool bWDelim = sal_True );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    /**
       Returns rewriter for this undo object.

       The returned rewriter has the following rule:

           $1 -> '<inserted text>'

       <inserted text> is shortened to a length of nUndoStringLength.

       @return rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const;

    void SetWithRsid() { m_bWithRsid = true; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoInsert)
};

SwRewriter SW_DLLPRIVATE
MakeUndoReplaceRewriter(sal_uLong const ocurrences,
    OUString const& sOld, OUString const& sNew);

class SwUndoReplace
    : public SwUndo
{
public:
    SwUndoReplace(SwPaM const& rPam,
            OUString const& rInsert, bool const bRegExp);

    virtual ~SwUndoReplace();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    /**
       Returns the rewriter of this undo object.

       If this undo object represents several replacements the
       rewriter contains the following rules:

           $1 -> <number of replacements>
           $2 -> occurrences of
           $3 -> <replaced text>

       If this undo object represents one replacement the rewriter
       contains these rules:

           $1 -> <replaced text>
           $2 -> "->"                   (STR_YIELDS)
           $3 -> <replacing text>

       @return the rewriter of this undo object
    */
    virtual SwRewriter GetRewriter() const;

    void SetEnd( const SwPaM& rPam );

private:
    class Impl;
    ::std::auto_ptr<Impl> m_pImpl;
};

class SwUndoReRead : public SwUndo
{
    Graphic *pGrf;
    OUString *pNm;
    OUString *pFltr;
    sal_uLong nPos;
    sal_uInt16 nMirr;

    void SaveGraphicData( const SwGrfNode& );
    void SetAndSave( ::sw::UndoRedoContext & );

public:
    SwUndoReRead( const SwPaM& rPam, const SwGrfNode& pGrfNd );

    virtual ~SwUndoReRead();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

class SwUndoInsertLabel : public SwUndo
{
    union {
        struct {
            // for NoTxtFrms
            SwUndoInsLayFmt* pUndoFly;
            SwUndoFmtAttr* pUndoAttr;
        } OBJECT;
        struct {
            // for tables or TxtFrms
            SwUndoDelete* pUndoInsNd;
            sal_uLong nNode;
        } NODE;
    };

    String sText;
    // #i39983# the separator is drawn with a character style
    String sSeparator;
    String sNumberSeparator;
    String sCharacterStyle;
    // #i26791# - re-store of drawing object position no longer needed
    sal_uInt16 nFldId;
    SwLabelType eType;
    sal_uInt8 nLayerId;              // for character objects
    sal_Bool bBefore        :1;
    sal_Bool bUndoKeep      :1;
    sal_Bool bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const String &rText,
    // #i39983# the separator is drawn with a character style
                        const String& rSeparator,
                        const String& rNumberSeparator, //#i61007# order of captions
                        const sal_Bool bBefore, const sal_uInt16 nId,
                        const String& rCharacterStyle,
                        const sal_Bool bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    /**
       Returns the rewriter of this undo object.

       The rewriter contains this rule:

           $1 -> '<text of inserted label>'

       <text of inserted label> is shortened to nUndoStringLength
       characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    void SetNodePos( sal_uLong nNd )
        { if( LTYPE_OBJECT != eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { bUndoKeep = sal_True; }
    void SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet, SwFrmFmt& rNewFly );
    void SetDrawObj( sal_uInt8 nLayerId );
};

#endif // SW_UNDO_INSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
