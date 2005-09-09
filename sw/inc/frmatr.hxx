/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmatr.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:54:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FRMATR_HXX
#define _FRMATR_HXX

#ifndef _HINTIDS_HXX
#include <hintids.hxx>      //die Ids der Attribute, vor frmitems damit die
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>       //fuer Implementierung der inlines
#endif

//------------------------ Inlines ---------------------------------


/******************************************************************************
 *  Implementierung der FrameAttribut Methoden vom SwAttrSet
 ******************************************************************************/

inline const SvxPaperBinItem &SwAttrSet::GetPaperBin(BOOL bInP) const
    { return (const SvxPaperBinItem&)Get( RES_PAPER_BIN,bInP); }
inline const SvxLRSpaceItem &SwAttrSet::GetLRSpace(BOOL bInP) const
    { return (const SvxLRSpaceItem&)Get( RES_LR_SPACE,bInP); }
inline const SvxULSpaceItem &SwAttrSet::GetULSpace(BOOL bInP) const
    { return (const SvxULSpaceItem&)Get( RES_UL_SPACE,bInP); }
inline const SvxPrintItem &SwAttrSet::GetPrint(BOOL bInP) const
    { return (const SvxPrintItem&)Get( RES_PRINT,bInP); }
inline const SvxOpaqueItem &SwAttrSet::GetOpaque(BOOL bInP) const
    { return (const SvxOpaqueItem&)Get( RES_OPAQUE,bInP); }
inline const SvxProtectItem &SwAttrSet::GetProtect(BOOL bInP) const
    { return (const SvxProtectItem&)Get( RES_PROTECT,bInP); }
inline const SvxBoxItem &SwAttrSet::GetBox(BOOL bInP) const
    { return (const SvxBoxItem&)Get( RES_BOX,bInP); }
inline const SvxFmtKeepItem &SwAttrSet::GetKeep(BOOL bInP) const
    { return (const SvxFmtKeepItem&)Get( RES_KEEP,bInP); }
inline const SvxBrushItem &SwAttrSet::GetBackground(BOOL bInP) const
    { return (const SvxBrushItem&)Get( RES_BACKGROUND,bInP); }
inline const SvxShadowItem &SwAttrSet::GetShadow(BOOL bInP) const
    { return (const SvxShadowItem&)Get( RES_SHADOW,bInP); }
inline const SvxFmtBreakItem &SwAttrSet::GetBreak(BOOL bInP) const
    { return (const SvxFmtBreakItem&)Get( RES_BREAK,bInP); }
inline const SvxMacroItem &SwAttrSet::GetMacro(BOOL bInP) const
    { return (const SvxMacroItem&)Get( RES_FRMMACRO,bInP); }
inline const SvxFrameDirectionItem &SwAttrSet::GetFrmDir(BOOL bInP) const
    { return (const SvxFrameDirectionItem&)Get( RES_FRAMEDIR,bInP); }


/******************************************************************************
 *  Implementierung der FrameAttribut Methoden vom SwFmt
 ******************************************************************************/

inline const SvxPaperBinItem &SwFmt::GetPaperBin(BOOL bInP) const
    { return aSet.GetPaperBin(bInP); }
inline const SvxLRSpaceItem &SwFmt::GetLRSpace(BOOL bInP) const
    { return aSet.GetLRSpace(bInP); }
inline const SvxULSpaceItem &SwFmt::GetULSpace(BOOL bInP) const
    { return aSet.GetULSpace(bInP); }
inline const SvxPrintItem &SwFmt::GetPrint(BOOL bInP) const
    { return aSet.GetPrint(bInP); }
inline const SvxOpaqueItem &SwFmt::GetOpaque(BOOL bInP) const
    { return aSet.GetOpaque(bInP); }
inline const SvxProtectItem &SwFmt::GetProtect(BOOL bInP) const
    { return aSet.GetProtect(bInP); }
inline const SvxBoxItem &SwFmt::GetBox(BOOL bInP) const
    { return aSet.GetBox(bInP); }
inline const SvxFmtKeepItem &SwFmt::GetKeep(BOOL bInP) const
    { return aSet.GetKeep(bInP); }
inline const SvxBrushItem &SwFmt::GetBackground(BOOL bInP) const
    { return aSet.GetBackground(bInP); }
inline const SvxShadowItem &SwFmt::GetShadow(BOOL bInP) const
    { return aSet.GetShadow(bInP); }
inline const SvxFmtBreakItem &SwFmt::GetBreak(BOOL bInP) const
    { return aSet.GetBreak(bInP); }
inline const SvxMacroItem &SwFmt::GetMacro(BOOL bInP) const
    { return aSet.GetMacro(bInP); }
inline const SvxFrameDirectionItem  &SwFmt::GetFrmDir(BOOL bInP) const
    { return aSet.GetFrmDir(bInP); }

#endif //_FRMATR_HXX
