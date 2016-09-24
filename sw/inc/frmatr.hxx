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
#ifndef INCLUDED_SW_INC_FRMATR_HXX
#define INCLUDED_SW_INC_FRMATR_HXX

#include <hintids.hxx>
#include <format.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/macitem.hxx>
#include <svx/sdtaitm.hxx>

// Inlines

// Implementation of FrameAttribute methods of SwAttrSet.
inline const SvxPaperBinItem &SwAttrSet::GetPaperBin(bool bInP) const
    { return static_cast<const SvxPaperBinItem&>(Get( RES_PAPER_BIN,bInP)); }
inline const SvxLRSpaceItem &SwAttrSet::GetLRSpace(bool bInP) const
    { return static_cast<const SvxLRSpaceItem&>(Get( RES_LR_SPACE,bInP)); }
inline const SvxULSpaceItem &SwAttrSet::GetULSpace(bool bInP) const
    { return static_cast<const SvxULSpaceItem&>(Get( RES_UL_SPACE,bInP)); }
inline const SvxPrintItem &SwAttrSet::GetPrint(bool bInP) const
    { return static_cast<const SvxPrintItem&>(Get( RES_PRINT,bInP)); }
inline const SvxOpaqueItem &SwAttrSet::GetOpaque(bool bInP) const
    { return static_cast<const SvxOpaqueItem&>(Get( RES_OPAQUE,bInP)); }
inline const SvxProtectItem &SwAttrSet::GetProtect(bool bInP) const
    { return static_cast<const SvxProtectItem&>(Get( RES_PROTECT,bInP)); }
inline const SvxBoxItem &SwAttrSet::GetBox(bool bInP) const
    { return static_cast<const SvxBoxItem&>(Get( RES_BOX,bInP)); }
inline const SvxFormatKeepItem &SwAttrSet::GetKeep(bool bInP) const
    { return static_cast<const SvxFormatKeepItem&>(Get( RES_KEEP,bInP)); }
inline const SvxBrushItem &SwAttrSet::GetBackground(bool bInP) const
    { return static_cast<const SvxBrushItem&>(Get( RES_BACKGROUND,bInP)); }
inline const SvxShadowItem &SwAttrSet::GetShadow(bool bInP) const
    { return static_cast<const SvxShadowItem&>(Get( RES_SHADOW,bInP)); }
inline const SvxFormatBreakItem &SwAttrSet::GetBreak(bool bInP) const
    { return static_cast<const SvxFormatBreakItem&>(Get( RES_BREAK,bInP)); }
inline const SvxMacroItem &SwAttrSet::GetMacro(bool bInP) const
    { return static_cast<const SvxMacroItem&>(Get( RES_FRMMACRO,bInP)); }
inline const SvxFrameDirectionItem &SwAttrSet::GetFrameDir(bool bInP) const
    { return static_cast<const SvxFrameDirectionItem&>(Get( RES_FRAMEDIR,bInP)); }
inline const SdrTextVertAdjustItem &SwAttrSet::GetTextVertAdjust(bool bInP) const
    { return static_cast<const SdrTextVertAdjustItem&>(Get( RES_TEXT_VERT_ADJUST,bInP)); }

// Implementation of FrameAttribute methods of SwFormat.
inline const SvxPaperBinItem &SwFormat::GetPaperBin(bool bInP) const
    { return m_aSet.GetPaperBin(bInP); }
inline const SvxLRSpaceItem &SwFormat::GetLRSpace(bool bInP) const
    { return m_aSet.GetLRSpace(bInP); }
inline const SvxULSpaceItem &SwFormat::GetULSpace(bool bInP) const
    { return m_aSet.GetULSpace(bInP); }
inline const SvxPrintItem &SwFormat::GetPrint(bool bInP) const
    { return m_aSet.GetPrint(bInP); }
inline const SvxOpaqueItem &SwFormat::GetOpaque(bool bInP) const
    { return m_aSet.GetOpaque(bInP); }
inline const SvxProtectItem &SwFormat::GetProtect(bool bInP) const
    { return m_aSet.GetProtect(bInP); }
inline const SvxBoxItem &SwFormat::GetBox(bool bInP) const
    { return m_aSet.GetBox(bInP); }
inline const SvxFormatKeepItem &SwFormat::GetKeep(bool bInP) const
    { return m_aSet.GetKeep(bInP); }
inline const SvxShadowItem &SwFormat::GetShadow(bool bInP) const
    { return m_aSet.GetShadow(bInP); }
inline const SvxFormatBreakItem &SwFormat::GetBreak(bool bInP) const
    { return m_aSet.GetBreak(bInP); }
inline const SvxMacroItem &SwFormat::GetMacro(bool bInP) const
    { return m_aSet.GetMacro(bInP); }
inline const SvxFrameDirectionItem  &SwFormat::GetFrameDir(bool bInP) const
    { return m_aSet.GetFrameDir(bInP); }
inline const SdrTextVertAdjustItem &SwFormat::GetTextVertAdjust(bool bInP) const
    { return m_aSet.GetTextVertAdjust(bInP); }

#endif // INCLUDED_SW_INC_FRMATR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
