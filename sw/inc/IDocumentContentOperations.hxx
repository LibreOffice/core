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

#ifndef INCLUDED_SW_INC_IDOCUMENTCONTENTOPERATIONS_HXX
#define INCLUDED_SW_INC_IDOCUMENTCONTENTOPERATIONS_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
using rtl::OUString;

class SwPaM;
struct SwPosition;
class SwNode;
class SwNodeRange;
class Graphic;
class SfxItemSet;
class SfxPoolItem;
class GraphicObject;
class SdrObject;
class SwFrmFmt;
class SwDrawFrmFmt;
class SwFlyFrmFmt;
class SwNodeIndex;
class SwFmtFld;

namespace utl { class TransliterationWrapper; }
namespace svt { class EmbeddedObjectRef; }

/** Text operation/manipulation interface
*/
class IDocumentContentOperations
{
public:
    enum SwMoveFlags
    {
        DOC_MOVEDEFAULT = 0x00,
        DOC_MOVEALLFLYS = 0x01,
        DOC_CREATEUNDOOBJ = 0x02,
        DOC_MOVEREDLINES = 0x04,
        DOC_NO_DELFRMS = 0x08
    };

    // constants for inserting text
    enum InsertFlags
    {   INS_DEFAULT         = 0x00 // no extras
    ,   INS_EMPTYEXPAND     = 0x01 // expand empty hints at insert position
    ,   INS_NOHINTEXPAND    = 0x02 // do not expand any hints at insert pos
    ,   INS_FORCEHINTEXPAND = 0x04 // expand all hints at insert position
    };

public:
    /** Copy a selected content range to a position

        The position can be in the same or in an another document. It can also
        be within the range!

        \warning The range has to include at least two nodes or has to be a
        SwDoc::IsColumnSelection!

        Normally this function should work only with content nodes. But there
        is a special case used by SwDoc::Paste, which starts the SwPaM at the
        content start node. This position doesn't contain any content:

        @code
        SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 1 );
        @endcode

        This is important, because it prevents merging of the first node of
        the range into the node pointed to by \p rPos.
        As a result this keeps all properties of the first real content node,
        which is the 2nd, including the Flys and the page description. In this
        case the first (fake) node is silently dropped and all other nodes are
        just copied.

        @param rPam
        The source node range to copy

        @param rPos
        The target copy destination
     */
    virtual bool CopyRange(SwPaM& rPam, SwPosition& rPos, const bool bCopyAll ) const = 0;

    /** Delete section containing the node.
    */
    virtual void DeleteSection(SwNode* pNode) = 0;

    /** Delete a range SwFlyFrmFmt.
    */
    virtual bool DeleteRange(SwPaM&) = 0;

    /** Delete full paragraphs.
    */
    virtual bool DelFullPara(SwPaM&) = 0;

    /** complete delete of a given PaM

        #i100466#
        Add optional parameter <bForceJoinNext>, default value <false>
        Needed for hiding of deletion redlines
    */
    virtual bool DeleteAndJoin( SwPaM&,
        const bool bForceJoinNext = false ) = 0;

    virtual bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    virtual bool MoveNodeRange(SwNodeRange&, SwNodeIndex&, SwMoveFlags) = 0;

    /** Move a range.
    */
    virtual bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags) = 0;

    /** Overwrite string in an existing text node.
    */
    virtual bool Overwrite(const SwPaM &rRg, const OUString& rStr) = 0;

    /** Insert string into existing text node at position rRg.Point().
     */
    virtual bool InsertString(const SwPaM &rRg, const OUString&,
              const enum InsertFlags nInsertMode = INS_EMPTYEXPAND ) = 0;

    /** change text to Upper/Lower/Hiragana/Katagana/...
     */
    virtual void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&) = 0;

    /** Insert graphic or formula. The XXXX are copied.
     */
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    virtual SwFlyFrmFmt* Insert(const SwPaM& rRg, const GraphicObject& rGrfObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    /** Transpose graphic (with undo)
     */
    virtual void ReRead(SwPaM&, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic, const GraphicObject* pGrfObj) = 0;

    /** Insert a DrawObject. The object must be already registered
        in DrawModel.
    */
    virtual SwDrawFrmFmt* InsertDrawObj( const SwPaM &rRg, SdrObject& rDrawObj, const SfxItemSet& rFlyAttrSet ) = 0;

    /** Insert OLE-objects.
    */
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, const SfxItemSet* pFlyAttrSet,
        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    virtual SwFlyFrmFmt* InsertOLE(const SwPaM &rRg, const OUString& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
                           const SfxItemSet* pGrfAttrSet, SwFrmFmt*) = 0;

    /** Split a node at rPos (implemented only for TxtNode).
    */
    virtual bool SplitNode(const SwPosition &rPos, bool bChkTableStart) = 0;

    virtual bool AppendTxtNode(SwPosition& rPos) = 0;

    /** Replace selected range in a TxtNode with string.
        Intended for search & replace.
        bRegExpRplc - replace tabs (\\t) and insert the found string
        ( not \& ). E.g.: Find: "zzz", Replace: "xx\t\\t..&..\&"
        --> "xx\t<Tab>..zzz..&"
    */
    virtual bool ReplaceRange(SwPaM& rPam, const OUString& rNewStr,
                              const bool bRegExReplace) = 0;

    /** Insert an attribute. If rRg spans several nodes the
        attribute is split, provided it makes sense.
        Nodes, where this attribute does not make sense are ignored.
        In nodes completely enclosed in the selection the attribute
        becomes hard-formated, in all other (text-) nodes the attribute
        is inserted into the attribute array.
        For a character attribute, in cases where no selection exists
        an "empty" hint is inserted. If there is a selection the attribute
        is hard-formated and added to the node at rRg.Start().
        If the attribute could not be inserted, the method returns
        false.
    */
    virtual bool InsertPoolItem(const SwPaM &rRg, const SfxPoolItem&,
                                const sal_uInt16 nFlags,bool bExpandCharToPara=false) = 0;

    virtual bool InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
        const sal_uInt16 nFlags) = 0;

    /** Removes any leading white space from the paragraph
    */
    virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos ) = 0;

protected:
    virtual ~IDocumentContentOperations() {};
};

#endif // INCLUDED_SW_INC_IDOCUMENTCONTENTOPERATIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
