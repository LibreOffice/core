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
    EditLineList maLineList;
    TextPortionList maTextPortionList;
    ContentNode* mpNode = nullptr;
    tools::Long mnHeight = 0;

    ScriptTypePosInfos maScriptInfos;
    WritingDirectionInfos maWritingDirectionInfos;

    sal_Int32 mnInvalidPosStart = 0;
    sal_Int32 mnFirstLineOffset = 0; // For Writer-LineSpacing-Interpretation
    sal_Int32 mnBulletX = 0;
    sal_Int32 mnInvalidDiff = 0;

    bool mbInvalid : 1 = true;
    bool mbSimple : 1 = false; // only linear Tap
    bool mbVisible : 1 = true; // Belongs to the node!
    bool mbForceRepaint : 1 = false;

    ParaPortion(const ParaPortion&) = delete;

public:
    ParaPortion(ContentNode* pNode)
        : mpNode(pNode)
    {
    }

    sal_Int32 GetLineNumber(sal_Int32 nIndex) const;

    EditLineList& GetLines() { return maLineList; }
    const EditLineList& GetLines() const { return maLineList; }

    bool IsInvalid() const { return mbInvalid; }
    bool IsSimpleInvalid() const { return mbSimple; }
    void SetValid()
    {
        mbInvalid = false;
        mbSimple = true;
    }

    bool MustRepaint() const { return mbForceRepaint; }
    void SetMustRepaint(bool bRP) { mbForceRepaint = bRP; }

    sal_Int32 GetBulletX() const { return mnBulletX; }
    void SetBulletX(sal_Int32 nBulletX) { mnBulletX = nBulletX; }

    void MarkInvalid(sal_Int32 nStart, sal_Int32 nDiff);
    void MarkSelectionInvalid(sal_Int32 nStart);

    void SetVisible(bool bVisible) { mbVisible = bVisible; }
    bool IsVisible() const { return mbVisible; }

    bool IsEmpty() { return GetTextPortions().isEmpty(); }

    tools::Long GetHeight() const { return mbVisible ? mnHeight : 0; }
    sal_Int32 GetFirstLineOffset() const { return mbVisible ? mnFirstLineOffset : 0; }
    void ResetHeight()
    {
        mnHeight = 0;
        mnFirstLineOffset = 0;
    }

    ScriptTypePosInfos& getScriptTypePosInfos() { return maScriptInfos; }
    ScriptTypePosInfos const& getScriptTypePosInfos() const { return maScriptInfos; }

    WritingDirectionInfos& getWritingDirectionInfos() { return maWritingDirectionInfos; }

    ContentNode* GetNode() const { return mpNode; }
    TextPortionList& GetTextPortions() { return maTextPortionList; }
    const TextPortionList& GetTextPortions() const { return maTextPortionList; }

    sal_Int32 GetInvalidPosStart() const { return mnInvalidPosStart; }
    short GetInvalidDiff() const { return mnInvalidDiff; }

    void CorrectValuesBehindLastFormattedLine(sal_Int32 nLastFormattedLine);
#if OSL_DEBUG_LEVEL > 0
    static bool DbgCheckTextPortions(ParaPortion const&);
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
