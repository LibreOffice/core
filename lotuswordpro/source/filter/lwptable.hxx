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
 *  For LWP filter architecture prototype - table object
 */

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABLE_HXX

#include <lwpobj.hxx>
#include <lwpobjhdr.hxx>
#include <lwpobjid.hxx>
#include "lwpstory.hxx"
#include "lwppara.hxx"
#include "lwppagehint.hxx"
#include <lwptools.hxx>
#include "lwptablelayout.hxx"

class LwpTableLayout;
/**
 * @brief
 * VO_SUPERTABLE object
 */
class LwpSuperTable: public LwpContent
{
public:
    LwpSuperTable(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpSuperTable() override;

    virtual void Parse(IXFStream* pOutputStream) override;
    virtual void XFConvert(XFContentContainer* pCont) override;
protected:
    void Read() override;
};
/**
 * @brief
 * for VO_TABLE object reading
 */
class LwpForkedNotifyList final
{
public:
    LwpForkedNotifyList(){}
    void Read(LwpObjectStream* pObjStrm)
    {
        m_PersistentList.Read(pObjStrm);
    };

private:
    LwpNotifyListPersistent m_PersistentList;
};
/**
 * @brief
 * VO_TABLE object
 */
class LwpTable: public LwpContent
{
public:
    LwpTable(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpTable() override;

    virtual void Parse(IXFStream* pOutputStream) override;
    double GetWidth() {return LwpTools::ConvertFromUnitsToMetric(m_nWidth);}
    double GetHeight() {return LwpTools::ConvertFromUnitsToMetric(m_nHeight);}
    LwpObjectID& GetDefaultCellStyle() {return m_DefaultCellStyle;}
    sal_uInt16 GetRow() {return m_nRow;}
    sal_uInt16 GetColumn() {return m_nColumn;}
    rtl::Reference<LwpTableLayout> GetTableLayout()
    {
        return rtl::Reference<LwpTableLayout>(dynamic_cast<LwpTableLayout*>(GetLayout(nullptr).get()));
    }
    bool IsNumberDown();
    virtual bool IsTable() override { return true;}
    LwpSuperTableLayout* GetSuperTableLayout();
protected:
    LwpForkedNotifyList m_CPNotifyList;

    sal_uInt16      m_nRow, m_nColumn;
    sal_Int32           m_nHeight;
    sal_Int32           m_nWidth;
    sal_Int32           m_nDefaultAutoGrowRowHeight;
    sal_uInt16      m_nAttributes;
    enum lTableAttributes
    {
        NUMBER_DOWN                 = 0x01,
        SIZING_VIA_MOUSE_ENABLED    = 0x02,
        NUMBER_RESET                = 0x04
    };

    LwpObjectID     m_Layout;
    LwpObjectID     m_DefaultCellStyle; // gCVirtualLayout
    void Read() override;
};

/**
 * @brief
 * VO_TABLEHEADING object
 */
class LwpTableHeading : public LwpTable
{
public:
    LwpTableHeading(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpTableHeading() override;

    virtual void Parse(IXFStream* pOutputStream) override;
protected:
    void Read() override;

};
class LwpParallelColumns : public LwpTable
{
public:
    LwpParallelColumns(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpParallelColumns() override;
protected:
    void Read() override;
private:
    LwpObjectID     cDefaultLeftColumnStyle;
    LwpObjectID cDefaultRightColumnStyle;
};

#define MAX_NUM_ROWS 8192
#define MAX_NUM_COLS 255

class LwpGlossary final : public LwpParallelColumns
{
public:
    LwpGlossary(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpGlossary() override;
private:
    void Read() override;
    sal_uInt16 GetNumIndexRows();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
