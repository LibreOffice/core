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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDODELETE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDODELETE_HXX

#include <undobj.hxx>
#include <rtl/ustring.hxx>
#include <memory>
#include <optional>

class SwRedlineSaveDatas;
class SwTextNode;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sfx2 {
    class MetadatableUndo;
}

class SwUndoDelete
    : public SwUndo
    , private SwUndRng
    , private SwUndoSaveContent
{
    std::unique_ptr<SwNodeIndex> m_pMvStt;            // Position of Nodes in UndoNodes-Array
    std::optional<OUString> m_aSttStr, m_aEndStr;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlSaveData;
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

    OUString m_sTableName;

    sal_uLong m_nNode;
    sal_uLong m_nNdDiff;           // difference of Nodes before/after Delete
    sal_uLong m_nSectDiff;         // diff. of Nodes before/after Move w/ SectionNodes
    sal_uLong m_nReplaceDummy;     // diff. to a temporary dummy object
    sal_uInt16 m_nSetPos;

    bool m_bGroup : 1;         // TRUE: is already Grouped; see CanGrouping()
    bool m_bBackSp : 1;        // TRUE: if Grouped and preceding content deleted
    bool m_bJoinNext: 1;       // TRUE: if range is selected forwards
    bool m_bTableDelLastNd : 1;  // TRUE: TextNode following Table inserted/deleted
    bool m_bDelFullPara : 1;   // TRUE: entire Nodes were deleted
    bool m_bResetPgDesc : 1;   // TRUE: reset PgDsc on following node
    bool m_bResetPgBrk : 1;    // TRUE: reset PgBreak on following node
    bool m_bFromTableCopy : 1; // TRUE: called by SwUndoTableCpyTable

    bool SaveContent( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTextNode* pSttTextNd, SwTextNode* pEndTextNd );

public:
    SwUndoDelete(
        SwPaM&,
        bool bFullPara = false,
        bool bCalledByTableCpy = false );
    virtual ~SwUndoDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
    /**
       Returns rewriter for this undo object.

       The rewriter consists of the following rule:

           $1 -> '<deleted text>'

       <deleted text> is shortened to nUndoStringLength characters.

       @return rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const override;

    bool CanGrouping( SwDoc&, const SwPaM& );

    void SetTableDelLastNd()      { m_bTableDelLastNd = true; }

    // for PageDesc/PageBreak Attributes of a table
    void SetPgBrkFlags( bool bPageBreak, bool bPageDesc )
        { m_bResetPgDesc = bPageDesc; m_bResetPgBrk = bPageBreak; }

    void SetTableName(const OUString & rName);

    // SwUndoTableCpyTable needs this information:
    bool IsDelFullPara() const { return m_bDelFullPara; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDODELETE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
