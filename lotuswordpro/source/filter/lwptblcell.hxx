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
/*************************************************************************
 * Change History
 Mar 2005           Created
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTBLCELL_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTBLCELL_HXX

#include "lwpobj.hxx"
#include "lwpatomholder.hxx"
#include "lwpstory.hxx"

#include "xfilter/xfcell.hxx"

// temporily added for compile
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
    LwpCellList(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    virtual void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
    LwpObjectID GetNextID(){return GetNext();}
    sal_uInt8 GetColumnID(){return cColumn;}
    virtual bool IsFormula(){return false;}
    LwpObjectID GetValueID(){return cValue;}

    virtual void Convert(XFCell * pCell, LwpTableLayout* pCellsMap=NULL);
protected:
    virtual ~LwpCellList();

    sal_uInt8 cColumn;
    LwpObjectID cParent;

    void Read() SAL_OVERRIDE;
    LwpObjectID cValue;
};
/**
 * @brief
 * VO_ROWLIST object
 */
class LwpRowList : public LwpDLVList
{
public:
    LwpRowList(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
    LwpObjectID GetChildHeadID(){return cChild.GetHead();}
    LwpObjectID GetNextID(){return GetNext();}
    sal_uInt16 GetRowID(){return cRowID;}
protected:
    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    sal_uInt16 cRowID;
    void Read() SAL_OVERRIDE;
private:
    virtual ~LwpRowList();
};
/**
 * @brief
 * VO_NUMERICVALUE object
 */
class LwpNumericValue : public LwpObject
{
public:
    LwpNumericValue(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    double GetValue(){return cNumber;}
    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
protected:
    double cNumber;
    //LwpContent m_TheContent;
    void Read() SAL_OVERRIDE;
private:
    virtual ~LwpNumericValue();
};

/**
 * @brief
 * VO_TABLERANGE object
 */
class LwpTableRange: public LwpDLVList
{
public:
    LwpTableRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
    LwpObjectID GetCellRangeID(){return cpCellRange;}
    LwpObjectID GetTableID(){ return cqTable;}
    LwpTableRange* GetNext() { return static_cast<LwpTableRange*>(LwpDLVList::GetNext().obj().get());}
protected:
    LwpObjectID cqTable;
    LwpObjectID cpCellRange;
    void Read() SAL_OVERRIDE;
private:
    virtual ~LwpTableRange();
};
/**
 * @brief
 * VO_CELLRANGE object
 */
class LwpCellRange: public LwpObject
{
public:
    LwpCellRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
    LwpObjectID GetFolderID(){return cpFolder;}
protected:
    LwpObjectID cpFolder;
    void Read() SAL_OVERRIDE;
private:
    virtual ~LwpCellRange();
};
/**
 * @brief
 * VO_FOLDER object
 */
class LwpFolder: public LwpDLVList
{
public:
    LwpFolder(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
    LwpObjectID GetChildHeadID(){ return cChild.GetHead();}
protected:
    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    LwpObjectID cqTable;
    void Read() SAL_OVERRIDE;
private:
    virtual ~LwpFolder();
};
/**
 * @brief
 * VO_DEPENDENT object
 */
class LwpDependent: public LwpDLVList
{
public:
    LwpDependent(LwpObjectHeader &objHdr, LwpSvStream* pStrm);

    void Parse(IXFStream* pOutputStream) SAL_OVERRIDE;
protected:
    void Read() SAL_OVERRIDE;
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
private:
    virtual ~LwpDependent();
};

/**
 * @brief
 * row or column id
 */
class LwpRowColumnQualifier
{
public:
    LwpRowColumnQualifier();
    ~LwpRowColumnQualifier(){}

    inline void SetAbsolute();
    inline void ClearAbsolute();
    bool IsAbsolute();

    inline void SetAfter();
    inline void ClearAfter();
    bool IsAfter();

    void SetBad(bool Bad);
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

inline void
LwpRowColumnQualifier::SetAbsolute()
{
    cFlags |= REF_ABSOLUTE;
}

inline void
LwpRowColumnQualifier::ClearAbsolute()
{
    cFlags &= ~REF_ABSOLUTE;
}

inline void
LwpRowColumnQualifier::SetAfter()
{
    cFlags |= REF_AFTER;
}

inline void
LwpRowColumnQualifier::ClearAfter()
{
    cFlags &= ~REF_AFTER;
}

inline
LwpRowColumnQualifier::LwpRowColumnQualifier()
{
    cFlags = 0;
}

inline bool
LwpRowColumnQualifier::IsAfter()
{
    return cFlags & REF_AFTER ? sal_True : sal_False;
}

inline bool
LwpRowColumnQualifier::IsBad()
{
    return cFlags & REF_BAD ? sal_True : sal_False;
}

inline bool
LwpRowColumnQualifier::IsAbsolute()
{
    return cFlags & REF_ABSOLUTE ? sal_True : sal_False;
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
    ~LwpRowSpecifier(){}

    void QuickRead(LwpObjectStream *pStrm);
    OUString ToString(sal_uInt16 nFormulaRow);

    sal_uInt16 RowID(sal_uInt16 FormulaRow);
    sal_uInt16 &Row();

    void SetAbsolute();
    void ClearAbsolute();
    bool IsAbsolute();

    void SetAfter();
    void ClearAfter();
    bool IsAfter();

    void SetBad(bool Bad);
    bool IsBad();
    void SetRowDelta(sal_uInt16 ReferenceRowID, sal_uInt16 FormulaRowID);

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

inline sal_uInt16 &
LwpRowSpecifier::Row()
{
    return cRow;
}

inline void
LwpRowSpecifier::SetAbsolute()
{
    cQualifier.SetAbsolute();
}

inline void
LwpRowSpecifier::ClearAbsolute()
{
    cQualifier.ClearAbsolute();
}

inline bool
LwpRowSpecifier::IsAbsolute()
{
    return cQualifier.IsAbsolute();
}

inline void
LwpRowSpecifier::SetAfter()
{
    cQualifier.SetAfter();
}

inline void
LwpRowSpecifier::ClearAfter()
{
    cQualifier.ClearAfter();
}

inline bool
LwpRowSpecifier::IsAfter()
{
    return cQualifier.IsAfter();
}

inline void
LwpRowSpecifier::SetBad(bool Bad)
{
    cQualifier.SetBad(Bad);
}

inline bool
LwpRowSpecifier::IsBad()
{
    return cQualifier.IsBad();
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
    ~LwpColumnSpecifier()
        {}

    void QuickRead(LwpObjectStream *pStrm);
    sal_uInt8 Column(){return cColumn;}
    OUString ToString(sal_uInt8 nFormulaCol);

    sal_uInt8 ColumnID(sal_uInt8 FormulaColumn);
    void SetAbsolute();
    void ClearAbsolute();
    bool IsAbsolute();
    void SetAfter();
    void ClearAfter();
    bool IsAfter();
    void SetBad(bool Bad);
    bool IsBad();
    void SetColumnDelta(sal_uInt8 ReferenceColumnID, sal_uInt8 FormulaColumnID);

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

inline void
LwpColumnSpecifier::SetAbsolute()
{
    cQualifier.SetAbsolute();
}

inline void
LwpColumnSpecifier::ClearAbsolute()
{
    cQualifier.ClearAbsolute();
}

inline bool
LwpColumnSpecifier::IsAbsolute()
{
    return cQualifier.IsAbsolute();
}

inline void
LwpColumnSpecifier::SetAfter()
{
    cQualifier.SetAfter();
}

inline void
LwpColumnSpecifier::ClearAfter()
{
    cQualifier.ClearAfter();
}

inline bool
LwpColumnSpecifier::IsAfter()
{
    return cQualifier.IsAfter();
}

inline void
LwpColumnSpecifier::SetBad(bool Bad)
{
    cQualifier.SetBad(Bad);
}

inline bool
LwpColumnSpecifier::IsBad()
{
    return cQualifier.IsBad();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
