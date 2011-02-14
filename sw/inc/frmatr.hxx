/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FRMATR_HXX
#define _FRMATR_HXX

#include <hintids.hxx>      //die Ids der Attribute, vor frmitems damit die
#include <format.hxx>       //fuer Implementierung der inlines

//------------------------ Inlines ---------------------------------


/******************************************************************************
 *  Implementierung der FrameAttribut Methoden vom SwAttrSet
 ******************************************************************************/

inline const SvxPaperBinItem &SwAttrSet::GetPaperBin(sal_Bool bInP) const
    { return (const SvxPaperBinItem&)Get( RES_PAPER_BIN,bInP); }
inline const SvxLRSpaceItem &SwAttrSet::GetLRSpace(sal_Bool bInP) const
    { return (const SvxLRSpaceItem&)Get( RES_LR_SPACE,bInP); }
inline const SvxULSpaceItem &SwAttrSet::GetULSpace(sal_Bool bInP) const
    { return (const SvxULSpaceItem&)Get( RES_UL_SPACE,bInP); }
inline const SvxPrintItem &SwAttrSet::GetPrint(sal_Bool bInP) const
    { return (const SvxPrintItem&)Get( RES_PRINT,bInP); }
inline const SvxOpaqueItem &SwAttrSet::GetOpaque(sal_Bool bInP) const
    { return (const SvxOpaqueItem&)Get( RES_OPAQUE,bInP); }
inline const SvxProtectItem &SwAttrSet::GetProtect(sal_Bool bInP) const
    { return (const SvxProtectItem&)Get( RES_PROTECT,bInP); }
inline const SvxBoxItem &SwAttrSet::GetBox(sal_Bool bInP) const
    { return (const SvxBoxItem&)Get( RES_BOX,bInP); }
inline const SvxFmtKeepItem &SwAttrSet::GetKeep(sal_Bool bInP) const
    { return (const SvxFmtKeepItem&)Get( RES_KEEP,bInP); }
inline const SvxBrushItem &SwAttrSet::GetBackground(sal_Bool bInP) const
    { return (const SvxBrushItem&)Get( RES_BACKGROUND,bInP); }
inline const SvxShadowItem &SwAttrSet::GetShadow(sal_Bool bInP) const
    { return (const SvxShadowItem&)Get( RES_SHADOW,bInP); }
inline const SvxFmtBreakItem &SwAttrSet::GetBreak(sal_Bool bInP) const
    { return (const SvxFmtBreakItem&)Get( RES_BREAK,bInP); }
inline const SvxMacroItem &SwAttrSet::GetMacro(sal_Bool bInP) const
    { return (const SvxMacroItem&)Get( RES_FRMMACRO,bInP); }
inline const SvxFrameDirectionItem &SwAttrSet::GetFrmDir(sal_Bool bInP) const
    { return (const SvxFrameDirectionItem&)Get( RES_FRAMEDIR,bInP); }


/******************************************************************************
 *  Implementierung der FrameAttribut Methoden vom SwFmt
 ******************************************************************************/

inline const SvxPaperBinItem &SwFmt::GetPaperBin(sal_Bool bInP) const
    { return aSet.GetPaperBin(bInP); }
inline const SvxLRSpaceItem &SwFmt::GetLRSpace(sal_Bool bInP) const
    { return aSet.GetLRSpace(bInP); }
inline const SvxULSpaceItem &SwFmt::GetULSpace(sal_Bool bInP) const
    { return aSet.GetULSpace(bInP); }
inline const SvxPrintItem &SwFmt::GetPrint(sal_Bool bInP) const
    { return aSet.GetPrint(bInP); }
inline const SvxOpaqueItem &SwFmt::GetOpaque(sal_Bool bInP) const
    { return aSet.GetOpaque(bInP); }
inline const SvxProtectItem &SwFmt::GetProtect(sal_Bool bInP) const
    { return aSet.GetProtect(bInP); }
inline const SvxBoxItem &SwFmt::GetBox(sal_Bool bInP) const
    { return aSet.GetBox(bInP); }
inline const SvxFmtKeepItem &SwFmt::GetKeep(sal_Bool bInP) const
    { return aSet.GetKeep(bInP); }
inline const SvxBrushItem &SwFmt::GetBackground(sal_Bool bInP) const
    { return aSet.GetBackground(bInP); }
inline const SvxShadowItem &SwFmt::GetShadow(sal_Bool bInP) const
    { return aSet.GetShadow(bInP); }
inline const SvxFmtBreakItem &SwFmt::GetBreak(sal_Bool bInP) const
    { return aSet.GetBreak(bInP); }
inline const SvxMacroItem &SwFmt::GetMacro(sal_Bool bInP) const
    { return aSet.GetMacro(bInP); }
inline const SvxFrameDirectionItem  &SwFmt::GetFrmDir(sal_Bool bInP) const
    { return aSet.GetFrmDir(bInP); }

#endif //_FRMATR_HXX
