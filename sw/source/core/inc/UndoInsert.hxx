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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOINSERT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOINSERT_HXX

#include <memory>

#include <undobj.hxx>
#include <rtl/ustring.hxx>
#include <tools/mempool.hxx>
#include <swtypes.hxx>
#include <IDocumentContentOperations.hxx>

class Graphic;
class SwGrfNode;
class SwUndoDelete;
class SwUndoFormatAttr;
namespace sw { class DocumentContentOperationsManager; }

class SwUndoInsert: public SwUndo, private SwUndoSaveContent
{
    /// start of Content in UndoNodes for Redo
    std::unique_ptr<SwNodeIndex> m_pUndoNodeIndex;
    OUString *pText, *pUndoText;
    SwRedlineData* pRedlData;
    sal_uLong nNode;
    sal_Int32 nContent, nLen;
    bool bIsWordDelim : 1;
    bool bIsAppend : 1;
    bool m_bWithRsid : 1;

    const SwInsertFlags m_nInsertFlags;

    friend class ::sw::DocumentContentOperationsManager;     // actually only DocumentContentOperationsManager::InsertString, because it uses CanGrouping
    bool CanGrouping( sal_Unicode cIns );
    bool CanGrouping( const SwPosition& rPos );

    SwDoc * pDoc;

    void Init(const SwNodeIndex & rNode);
    OUString * GetTextFromDoc() const;

public:
    SwUndoInsert( const SwNodeIndex& rNode, sal_Int32 nContent, sal_Int32 nLen,
                  const SwInsertFlags nInsertFlags,
                  bool bWDelim = true );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    /**
       Returns rewriter for this undo object.

       The returned rewriter has the following rule:

           $1 -> '<inserted text>'

       <inserted text> is shortened to a length of nUndoStringLength.

       @return rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const override;

    void SetWithRsid() { m_bWithRsid = true; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoInsert)
};

SwRewriter
MakeUndoReplaceRewriter(sal_uLong const ocurrences,
    OUString const& sOld, OUString const& sNew);

class SwUndoReplace
    : public SwUndo
{
public:
    SwUndoReplace(SwPaM const& rPam,
            OUString const& rInsert, bool const bRegExp);

    virtual ~SwUndoReplace();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

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
    virtual SwRewriter GetRewriter() const override;

    void SetEnd( const SwPaM& rPam );

private:
    class Impl;
    ::std::unique_ptr<Impl> m_pImpl;
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

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoInsertLabel : public SwUndo
{
    union {
        struct {
            // for NoTextFrames
            SwUndoInsLayFormat* pUndoFly;
            SwUndoFormatAttr* pUndoAttr;
        } OBJECT;
        struct {
            // for tables or TextFrames
            SwUndoDelete* pUndoInsNd;
            sal_uLong nNode;
        } NODE;
    };

    OUString sText;
    // #i39983# the separator is drawn with a character style
    OUString sSeparator;
    OUString sNumberSeparator;
    OUString sCharacterStyle;
    // #i26791# - re-store of drawing object position no longer needed
    sal_uInt16 nFieldId;
    SwLabelType eType;
    sal_uInt8 nLayerId;              // for character objects
    bool bBefore        :1;
    bool bUndoKeep      :1;
    bool bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const OUString &rText,
    // #i39983# the separator is drawn with a character style
                        const OUString& rSeparator,
                        const OUString& rNumberSeparator, //#i61007# order of captions
                        const bool bBefore, const sal_uInt16 nId,
                        const OUString& rCharacterStyle,
                        const bool bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    /**
       Returns the rewriter of this undo object.

       The rewriter contains this rule:

           $1 -> '<text of inserted label>'

       <text of inserted label> is shortened to nUndoStringLength
       characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const override;
    static SwRewriter CreateRewriter(const OUString &rStr);

    void SetNodePos( sal_uLong nNd )
        { if( LTYPE_OBJECT != eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { bUndoKeep = true; }
    void SetFlys( SwFrameFormat& rOldFly, SfxItemSet& rChgSet, SwFrameFormat& rNewFly );
    void SetDrawObj( sal_uInt8 nLayerId );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOINSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
