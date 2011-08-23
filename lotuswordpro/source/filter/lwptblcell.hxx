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
 Mar 2005		 	Created
 ************************************************************************/
#ifndef _LWPLAYOUTNUMERICSOVERRIDE_HXX_
#define _LWPLAYOUTNUMERICSOVERRIDE_HXX_

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
    virtual ~LwpCellList();

    virtual void Parse(IXFStream* pOutputStream);
    LwpObjectID GetNextID(){return *GetNext();}
    sal_uInt8 GetColumnID(){return cColumn;}
    virtual sal_Bool IsFormula(){return sal_False;}
    LwpObjectID GetValueID(){return cValue;}

    virtual void Convert(XFCell * pCell, LwpTableLayout* pCellsMap=NULL);
protected:
    sal_uInt8 cColumn;
    LwpObjectID cParent;

    void Read();
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
    ~LwpRowList();

    void Parse(IXFStream* pOutputStream);
    LwpObjectID GetChildHeadID(){return *cChild.GetHead();}
    LwpObjectID GetNextID(){return *GetNext();}
    sal_uInt16 GetRowID(){return cRowID;}
protected:
    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    sal_uInt16 cRowID;
    void Read();
};
/**
 * @brief
 * VO_NUMERICVALUE object
 */
class LwpNumericValue : public LwpObject
{
public:
    LwpNumericValue(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpNumericValue();

    double GetValue(){return cNumber;}
    void Parse(IXFStream* pOutputStream);
protected:
    double cNumber;
    //LwpContent m_TheContent;
    void Read();
};

////////////////////////////////////////////////////////////////////////
/**
 * @brief
 * VO_TABLERANGE object
 */
class LwpTableRange: public LwpDLVList
{
public:
    LwpTableRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpTableRange();

    void Parse(IXFStream* pOutputStream);
    LwpObjectID GetCellRangeID(){return cpCellRange;}
    LwpObjectID GetTableID(){ return cqTable;}
    LwpTableRange* GetNext() { return (LwpTableRange*)(LwpDLVList::GetNext()->obj());}
protected:
    LwpObjectID cqTable;
    LwpObjectID cpCellRange;
    void Read();
};
/**
 * @brief
 * VO_CELLRANGE object
 */
class LwpCellRange: public LwpObject
{
public:
    LwpCellRange(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpCellRange();

    void Parse(IXFStream* pOutputStream);
    LwpObjectID GetFolderID(){return cpFolder;}
protected:
    LwpObjectID cpFolder;
    void Read();
};
/**
 * @brief
 * VO_FOLDER object
 */
class LwpFolder: public LwpDLVList
{
public:
    LwpFolder(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFolder();

    void Parse(IXFStream* pOutputStream);
    LwpObjectID GetChildHeadID(){ return *cChild.GetHead();}
protected:
    LwpDLVListHeadTail cChild;
    LwpObjectID cParent;
    LwpObjectID cqTable;
    void Read();
};
/**
 * @brief
 * VO_DEPENDENT object
 */
class LwpDependent: public LwpDLVList
{
public:
    LwpDependent(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpDependent();

    void Parse(IXFStream* pOutputStream);
protected:
    void Read();
    LwpObjectID cFormulaInfo;
    sal_uInt16 cReferenceOffset;	// Used to fix dependent formula when we're
                                //  dropped, sorted.
    // Flags:
    enum
    {
        START_CELL	= 0x01,
        END_CELL	= 0x02,
        REGISTERED	= 0x04
    };
    sal_uInt8 cFlags;				 // Used to fix dependent formula when we're
                                //  dropped, sorted.
};


///////////////////////////////////////////////////////////////
/**
 * @brief
 * row or column id
 */
class LwpRowColumnQualifier
{
public:
    LwpRowColumnQualifier(void);
    ~LwpRowColumnQualifier(){}

    inline void SetAbsolute(void);
    inline void ClearAbsolute(void);
    sal_Bool IsAbsolute(void);

    inline void SetAfter(void);
    inline void ClearAfter(void);
    sal_Bool IsAfter(void);

    void SetBad(sal_Bool Bad);
    sal_Bool IsBad(void);

    void QuickRead(LwpObjectStream *pStrm);

private:
    enum	// cFlags bit definitions
    {
        REF_ABSOLUTE	= 0x01,
        REF_AFTER		= 0x02,
        REF_BAD			= 0x04
    };
    sal_uInt8 cFlags;
};

inline void
LwpRowColumnQualifier::SetAbsolute(void)
{
    cFlags |= REF_ABSOLUTE;
}

inline void
LwpRowColumnQualifier::ClearAbsolute(void)
{
    cFlags &= ~REF_ABSOLUTE;
}

inline void
LwpRowColumnQualifier::SetAfter(void)
{
    cFlags |= REF_AFTER;
}

inline void
LwpRowColumnQualifier::ClearAfter(void)
{
    cFlags &= ~REF_AFTER;
}

inline
LwpRowColumnQualifier::LwpRowColumnQualifier()
{
    cFlags = 0;
}

inline sal_Bool
LwpRowColumnQualifier::IsAfter()
{
    return cFlags & REF_AFTER ? sal_True : sal_False;
}

inline sal_Bool
LwpRowColumnQualifier::IsBad()
{
    return cFlags & REF_BAD ? sal_True : sal_False;
}

inline sal_Bool
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
    LwpRowSpecifier(void){}
    ~LwpRowSpecifier(){}

    void QuickRead(LwpObjectStream *pStrm);
    String ToString(USHORT nFormulaRow);

    USHORT RowID(USHORT FormulaRow);
    USHORT &Row(void);

    void SetAbsolute(void);
    void ClearAbsolute(void);
    sal_Bool IsAbsolute(void);

    void SetAfter(void);
    void ClearAfter(void);
    sal_Bool IsAfter(void);

    void SetBad(sal_Bool Bad);
    sal_Bool IsBad(void);
    void SetRowDelta(USHORT ReferenceRowID, USHORT FormulaRowID);

private:
    sal_uInt16 cRow;
    LwpRowColumnQualifier cQualifier;
};

inline USHORT
LwpRowSpecifier::RowID(USHORT FormulaRow)
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

inline USHORT &
LwpRowSpecifier::Row()
{
    return cRow;
}

inline void
LwpRowSpecifier::SetAbsolute(void)
{
    cQualifier.SetAbsolute();
}

inline void
LwpRowSpecifier::ClearAbsolute(void)
{
    cQualifier.ClearAbsolute();
}

inline sal_Bool
LwpRowSpecifier::IsAbsolute()
{
    return cQualifier.IsAbsolute();
}

inline void
LwpRowSpecifier::SetAfter(void)
{
    cQualifier.SetAfter();
}

inline void
LwpRowSpecifier::ClearAfter(void)
{
    cQualifier.ClearAfter();
}

inline sal_Bool
LwpRowSpecifier::IsAfter()
{
    return cQualifier.IsAfter();
}

inline void
LwpRowSpecifier::SetBad(sal_Bool Bad)
{
    cQualifier.SetBad(Bad);
}

inline sal_Bool
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
    LwpColumnSpecifier(void){};
    ~LwpColumnSpecifier(){};

    void QuickRead(LwpObjectStream *pStrm);
    sal_uInt8 Column(){return cColumn;}
    String ToString(sal_uInt8 nFormulaCol);

    sal_uInt8 ColumnID(sal_uInt8 FormulaColumn);
    void SetAbsolute(void);
    void ClearAbsolute(void);
    sal_Bool IsAbsolute(void);
    void SetAfter(void);
    void ClearAfter(void);
    sal_Bool IsAfter(void);
    void SetBad(sal_Bool Bad);
    sal_Bool IsBad(void);
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
LwpColumnSpecifier::SetAbsolute(void)
{
    cQualifier.SetAbsolute();
}

inline void
LwpColumnSpecifier::ClearAbsolute(void)
{
    cQualifier.ClearAbsolute();
}

inline sal_Bool
LwpColumnSpecifier::IsAbsolute()
{
    return cQualifier.IsAbsolute();
}

inline void
LwpColumnSpecifier::SetAfter(void)
{
    cQualifier.SetAfter();
}

inline void
LwpColumnSpecifier::ClearAfter(void)
{
    cQualifier.ClearAfter();
}

inline sal_Bool
LwpColumnSpecifier::IsAfter()
{
    return cQualifier.IsAfter();
}

inline void
LwpColumnSpecifier::SetBad(sal_Bool Bad)
{
    cQualifier.SetBad(Bad);
}

inline sal_Bool
LwpColumnSpecifier::IsBad()
{
    return cQualifier.IsBad();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
