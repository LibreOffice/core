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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTBLCELL_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTBLCELL_HXX

#include <lwpobj.hxx>
#include <lwpatomholder.hxx>
#include "lwpstory.hxx"

#include <xfilter/xfcell.hxx>

// temporarily added for compile
class LwpObject;

class LwpContent;
class LwpTableLayout;
/**
 * @brief
 * VO_CELLLIST object
 */
class LwpCellList : public LwpDLVList
{
public:
    LwpCellList(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    virtual void Parse(IXFStream* pOutputStream) override;
    LwpObjectID const & GetNextID(){return GetNext();}
    sal_uInt8 GetColumnID(){return cColumn;}

    virtual void Convert(XFCell * pCell, LwpTableLayout* pCellsMap=nullptr);
protected:
    virtual ~LwpCellList() override;

    sal_uInt8 cColumn;
    LwpObjectID cParent;

    void Read() override;
    LwpObjectID cValue;
};
/**
 * @brief
 * VO_ROWLIST object
 */
class LwpRowList final : public LwpDLVList
{
public:
    LwpRowList(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) override;
    LwpObjectID const & GetChildHeadID(){return cChild.GetHead();}
    LwpObjectID const & GetNextID(){return GetNext();}
    sal_uInt16 GetRowID(){return cRowID;}
private:
    void Read() override;
    virtual ~LwpRowList() override;

    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    sal_uInt16 cRowID;
};
/**
 * @brief
 * VO_NUMERICVALUE object
 */
class LwpNumericValue final : public LwpObject
{
public:
    LwpNumericValue(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    double GetValue(){return cNumber;}
    void Parse(IXFStream* pOutputStream) override;
private:
    void Read() override;
    virtual ~LwpNumericValue() override;

    double cNumber;
};

/**
 * @brief
 * VO_TABLERANGE object
 */
class LwpTableRange final : public LwpDLVList
{
public:
    LwpTableRange(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) override;
    const LwpObjectID& GetCellRangeID(){return cpCellRange;}
    const LwpObjectID& GetTableID(){ return cqTable;}
    LwpTableRange* GetNext() { return dynamic_cast<LwpTableRange*>(LwpDLVList::GetNext().obj().get());}
private:
    void Read() override;
    virtual ~LwpTableRange() override;

    LwpObjectID cqTable;
    LwpObjectID cpCellRange;
};
/**
 * @brief
 * VO_CELLRANGE object
 */
class LwpCellRange final : public LwpObject
{
public:
    LwpCellRange(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) override;
    const LwpObjectID& GetFolderID(){return cpFolder;}
private:
    void Read() override;
    virtual ~LwpCellRange() override;

    LwpObjectID cpFolder;
};
/**
 * @brief
 * VO_FOLDER object
 */
class LwpFolder final : public LwpDLVList
{
public:
    LwpFolder(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) override;
    LwpObjectID const & GetChildHeadID(){ return cChild.GetHead();}
private:
    void Read() override;
    virtual ~LwpFolder() override;

    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    LwpObjectID cqTable;
};
/**
 * @brief
 * VO_DEPENDENT object
 */
class LwpDependent final : public LwpDLVList
{
public:
    LwpDependent(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) override;
private:
    void Read() override;
    virtual ~LwpDependent() override;

    LwpObjectID cFormulaInfo;
    sal_uInt16 cReferenceOffset;    // Used to fix dependent formula when we're
                                //  dropped, sorted.
    // Flags:
    enum
    {
        START_CELL  = 0x01,
        END_CELL    = 0x02,
        REGISTERED  = 0x04
    };
    sal_uInt8 cFlags;                // Used to fix dependent formula when we're
                                //  dropped, sorted.
};

/**
 * @brief
 * row or column id
 */
class LwpRowColumnQualifier
{
public:
    LwpRowColumnQualifier();

    bool IsAbsolute();

    bool IsAfter();

    bool IsBad();

    void QuickRead(LwpObjectStream *pStrm);

private:
    enum    // cFlags bit definitions
    {
        REF_ABSOLUTE    = 0x01,
        REF_AFTER       = 0x02,
        REF_BAD         = 0x04
    };
    sal_uInt8 cFlags;
};

inline
LwpRowColumnQualifier::LwpRowColumnQualifier()
{
    cFlags = 0;
}

inline bool
LwpRowColumnQualifier::IsAfter()
{
    return (cFlags & REF_AFTER) != 0;
}

inline bool
LwpRowColumnQualifier::IsBad()
{
    return (cFlags & REF_BAD) != 0;
}

inline bool
LwpRowColumnQualifier::IsAbsolute()
{
    return (cFlags & REF_ABSOLUTE) != 0;
}
/**
 * @brief
 * row id
 */
class LwpRowSpecifier
{
public:
    LwpRowSpecifier()
        : cRow(0)
        {}

    void QuickRead(LwpObjectStream *pStrm);

    sal_uInt16 RowID(sal_uInt16 FormulaRow);

private:
    sal_uInt16 cRow;
    LwpRowColumnQualifier cQualifier;
};

inline sal_uInt16
LwpRowSpecifier::RowID(sal_uInt16 FormulaRow)
{
    if (cQualifier.IsBad())
    {
        return 0xffff;
    }
    if (cQualifier.IsAbsolute())
        return cRow;

    if (cQualifier.IsAfter())
        return FormulaRow + cRow;
    return FormulaRow - cRow;
}

/**
 * @brief
 * column id
 */
class LwpColumnSpecifier
{
public:
    LwpColumnSpecifier()
        : cColumn(0)
        {}

    void QuickRead(LwpObjectStream *pStrm);

    sal_uInt8 ColumnID(sal_uInt8 FormulaColumn);

private:
    sal_uInt8 cColumn;
    LwpRowColumnQualifier cQualifier;
};

inline sal_uInt8
LwpColumnSpecifier::ColumnID(sal_uInt8 FormulaColumn)
{
    if (cQualifier.IsBad())
    {
        return 0xff;
    }
    if (cQualifier.IsAbsolute())
        return cColumn;
    if (cQualifier.IsAfter())
        return FormulaColumn + cColumn;
    return FormulaColumn - cColumn;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
