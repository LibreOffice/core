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
#include <svx/svdtypes.hxx>
#include <rtl/ustring.hxx>
#include <swtypes.hxx>
#include <IDocumentContentOperations.hxx>
#include <optional>

class Graphic;
class SwGrfNode;
class SwUndoDelete;
class SwUndoFormatAttr;
class SwDoc;
namespace sw { class DocumentContentOperationsManager; }
enum class MirrorGraph;

class SwUndoInsert final : public SwUndo, private SwUndoSaveContent
{
    /// start of Content in UndoNodes for Redo
    std::unique_ptr<SwNodeIndex> m_pUndoNodeIndex;
    std::optional<OUString> maText;
    std::optional<OUString> maUndoText;
    std::unique_ptr<SwRedlineData> m_pRedlData;
    SwNodeOffset m_nNode;
    sal_Int32 m_nContent, m_nLen;
    bool m_bIsWordDelim : 1;
    bool m_bIsAppend : 1;
    bool m_bWithRsid : 1;

    const SwInsertFlags m_nInsertFlags;

    friend class ::sw::DocumentContentOperationsManager;     // actually only DocumentContentOperationsManager::InsertString, because it uses CanGrouping
    bool CanGrouping( sal_Unicode cIns );
    bool CanGrouping( const SwPosition& rPos );

    SwDoc * m_pDoc;

    void Init(const SwNodeIndex & rNode);
    std::optional<OUString> GetTextFromDoc() const;

public:
    SwUndoInsert( const SwNodeIndex& rNode, sal_Int32 nContent, sal_Int32 nLen,
                  const SwInsertFlags nInsertFlags,
                  bool bWDelim = true );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert() override;

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
};

SwRewriter
MakeUndoReplaceRewriter(sal_uLong const occurrences,
    OUString const& sOld, OUString const& sNew);

class SwUndoReplace final
    : public SwUndo
{
public:
    SwUndoReplace(SwPaM const& rPam,
            OUString const& rInsert, bool const bRegExp);

    virtual ~SwUndoReplace() override;

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
    std::unique_ptr<Impl> m_pImpl;
};

class SwUndoReRead final : public SwUndo
{
    std::unique_ptr<Graphic> mpGraphic;
    std::optional<OUString> maNm;
    std::optional<OUString> maFltr;
    SwNodeOffset mnPosition;
    MirrorGraph mnMirror;

    void SaveGraphicData( const SwGrfNode& );
    void SetAndSave( ::sw::UndoRedoContext & );

public:
    SwUndoReRead( const SwPaM& rPam, const SwGrfNode& pGrfNd );

    virtual ~SwUndoReRead() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoInsertLabel final : public SwUndo
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
            SwNodeOffset nNode;
        } NODE;
    };

    OUString m_sText;
    // #i39983# the separator is drawn with a character style
    OUString m_sSeparator;
    OUString m_sNumberSeparator;
    OUString m_sCharacterStyle;
    // #i26791# - re-store of drawing object position no longer needed
    sal_uInt16 m_nFieldId;
    SwLabelType m_eType;
    SdrLayerID m_nLayerId;              // for character objects
    bool m_bBefore        :1;
    bool m_bUndoKeep      :1;
    bool m_bCopyBorder        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const OUString &rText,
    // #i39983# the separator is drawn with a character style
                        const OUString& rSeparator,
                        const OUString& rNumberSeparator, //#i61007# order of captions
                        const bool bBefore, const sal_uInt16 nId,
                        const OUString& rCharacterStyle,
                        const bool bCpyBrd,
                        const SwDoc* pDoc );
    virtual ~SwUndoInsertLabel() override;

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

    void SetNodePos( SwNodeOffset nNd )
        { if( SwLabelType::Object != m_eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { m_bUndoKeep = true; }
    void SetFlys( SwFrameFormat& rOldFly, SfxItemSet const & rChgSet, SwFrameFormat& rNewFly );
    void SetDrawObj( SdrLayerID nLayerId );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOINSERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
