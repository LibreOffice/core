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

#pragma once

#include "EditLineList.hxx"
#include "TextPortionList.hxx"

struct ScriptTypePosInfo
{
    short nScriptType;
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;

    ScriptTypePosInfo(short Type, sal_Int32 Start, sal_Int32 End)
        : nScriptType(Type)
        , nStartPos(Start)
        , nEndPos(End)
    {
    }
};

typedef std::vector<ScriptTypePosInfo> ScriptTypePosInfos;

struct WritingDirectionInfo
{
    sal_uInt8 nType;
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;

    WritingDirectionInfo(sal_uInt8 Type, sal_Int32 Start, sal_Int32 End)
        : nType(Type)
        , nStartPos(Start)
        , nEndPos(End)
    {
    }
};

typedef std::vector<WritingDirectionInfo> WritingDirectionInfos;

class ParaPortion
{
    friend class ImpEditEngine; // to adjust the height
private:
    EditLineList aLineList;
    TextPortionList aTextPortionList;
    ContentNode* pNode;
    tools::Long nHeight;

    ScriptTypePosInfos aScriptInfos;
    WritingDirectionInfos aWritingDirectionInfos;

    sal_Int32 nInvalidPosStart;
    sal_Int32 nFirstLineOffset; // For Writer-LineSpacing-Interpretation
    sal_Int32 nBulletX;
    sal_Int32 nInvalidDiff;

    bool bInvalid : 1;
    bool bSimple : 1; // only linear Tap
    bool bVisible : 1; // Belongs to the node!
    bool bForceRepaint : 1;

    ParaPortion(const ParaPortion&) = delete;

public:
    ParaPortion(ContentNode* pNode);
    ~ParaPortion();

    sal_Int32 GetLineNumber(sal_Int32 nIndex) const;

    EditLineList& GetLines() { return aLineList; }
    const EditLineList& GetLines() const { return aLineList; }

    bool IsInvalid() const { return bInvalid; }
    bool IsSimpleInvalid() const { return bSimple; }
    void SetValid()
    {
        bInvalid = false;
        bSimple = true;
    }

    bool MustRepaint() const { return bForceRepaint; }
    void SetMustRepaint(bool bRP) { bForceRepaint = bRP; }

    sal_Int32 GetBulletX() const { return nBulletX; }
    void SetBulletX(sal_Int32 n) { nBulletX = n; }

    void MarkInvalid(sal_Int32 nStart, sal_Int32 nDiff);
    void MarkSelectionInvalid(sal_Int32 nStart);

    void SetVisible(bool bVisible);
    bool IsVisible() const { return bVisible; }

    bool IsEmpty() { return GetTextPortions().Count() == 1 && GetTextPortions()[0].GetLen() == 0; }

    tools::Long GetHeight() const { return (bVisible ? nHeight : 0); }
    sal_Int32 GetFirstLineOffset() const { return (bVisible ? nFirstLineOffset : 0); }
    void ResetHeight()
    {
        nHeight = 0;
        nFirstLineOffset = 0;
    }

    ContentNode* GetNode() const { return pNode; }
    TextPortionList& GetTextPortions() { return aTextPortionList; }
    const TextPortionList& GetTextPortions() const { return aTextPortionList; }

    sal_Int32 GetInvalidPosStart() const { return nInvalidPosStart; }
    short GetInvalidDiff() const { return nInvalidDiff; }

    void CorrectValuesBehindLastFormattedLine(sal_Int32 nLastFormattedLine);
#if OSL_DEBUG_LEVEL > 0
    static bool DbgCheckTextPortions(ParaPortion const&);
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
