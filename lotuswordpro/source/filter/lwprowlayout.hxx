/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  For LWP filter architecture prototype - row layouts
 */

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPROWLAYOUT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPROWLAYOUT_HXX

#include "lwplayout.hxx"
#include "lwptablelayout.hxx"
#include "lwpcelllayout.hxx"

class XFRow;
class XFTable;
class LwpTableLayout;
/**
 * @brief
 * VO_ROWLAYOUT obj
 */
class LwpRowLayout : public LwpVirtualLayout
{
public:
    LwpRowLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpRowLayout() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_ROW_LAYOUT;}
    sal_uInt16 GetRowID() { return crowid;}
    virtual void RegisterStyle() override;
    LwpTableLayout * GetParentTableLayout(){return dynamic_cast<LwpTableLayout *>(GetParent().obj().get());}
    void SetRowMap();
protected:
    void Read() override;
    sal_uInt16 crowid;
    sal_Int32 cheight;              // Minimum height if height is automatic.
    sal_uInt8 cLeaderDotCount;  // dfb - # of cells with leader dots
    sal_Int32 cLeaderDotY;
    sal_uInt8 cRowFlags;
    enum // for cRowFlags
    {
        RF_HAS_BORDER       = 0x01,
        RF_VALID_HAS_BORDER = 0x02,
        RF_LEADER_COUNT_VALID = 0x4
    };
private:
    std::vector<LwpConnectedCellLayout*> m_ConnCellList;
    sal_Int32 FindMarkConnCell(sal_uInt8 nStartCol,sal_uInt8 nEndCol);
    sal_Int32 FindNextMarkConnCell(sal_uInt16 nMarkConnCell,sal_uInt8 nEndCol);

public:
    void ConvertRow(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol, sal_uInt8 nEndCol);
    void ConvertCommonRow(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol, sal_uInt8 nEndCol);
    void CollectMergeInfo();
    void SetCellSplit(sal_uInt16 nEffectRows);
    bool GetMergeCellFlag();
    sal_uInt16 GetCurMaxSpannedRows(sal_uInt8 nStartCol,sal_uInt8 nEndCol);
    void RegisterCurRowStyle(XFRow* pXFRow,sal_uInt16 nRowMark);
};

/**
 * @brief
 * VO_ROWHEADINGLAYOUT obj
 */
class LwpRowHeadingLayout : public LwpRowLayout
{
public:
    LwpRowHeadingLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpRowHeadingLayout() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_ROW_HEADING_LAYOUT;}
protected:
    void Read() override;
private:
    LwpObjectID cRowLayout;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
