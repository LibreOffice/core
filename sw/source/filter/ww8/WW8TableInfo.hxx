/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WW8TableInfo.hxx,v $
 * $Revision: 1.1.2.4 $
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

#ifndef WW8_TABLE_INFO_HXX
#define WW8_TABLE_INFO_HXX
#include <hash_map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <sal/types.h>

class SwTable;
class SwTableLine;
class SwTableBox;
class SwNode;
class SwWW8Writer;

namespace ww8
{
using namespace ::std;

class WW8TableNodeInfo
{
    sal_uInt32 mnDepth;
    sal_uInt32 mnCell;
    sal_uInt32 mnRow;
    bool mbEndOfLine;
    bool mbEndOfCell;
    const SwNode * mpNode;
    const SwTableBox * mpTableBox;
    const SwTable * mpTable;

 public:
    typedef boost::shared_ptr<WW8TableNodeInfo> Pointer_t;

    WW8TableNodeInfo(const SwNode * pTxtNode,
                         const SwTableBox * pTableBox,
                         const SwTable * pTable);
    virtual ~WW8TableNodeInfo();

    void setDepth(sal_uInt32 nDepth);
    void setEndOfLine(bool bEndOfLine);
    void setEndOfCell(bool bEndOfCell);
    void setTableBox(const SwTableBox *pTableBox);
    void setTable(const SwTable * pTable);
    void setCell(sal_uInt32 nCell);
    void setRow(sal_uInt32 nRow);

    sal_uInt32 getDepth() const;
    const SwNode * getNode() const;
    const SwTableBox * getTableBox() const;
    const SwTable * getTable() const;
    bool isEndOfLine() const;
    bool isEndOfCell() const;
    sal_uInt32 getCell() const;
    sal_uInt32 getRow() const;

    ::std::string toString() const;
};

struct hashNode
{
    size_t operator()(const SwNode * pNode) const
    { return reinterpret_cast<size_t>(pNode); }
};

class WW8TableInfo
{
    typedef hash_map<const SwNode *, WW8TableNodeInfo::Pointer_t, hashNode > Map_t;
    Map_t mMap;

    void processTableLine(const SwTable * pTable,
                          const SwTableLine * pTableLine,
                          sal_uInt32 nRow,
                          sal_uInt32 nDepth);

    WW8TableNodeInfo::Pointer_t
    processTableBox(const SwTable * pTable,
                    const SwTableBox * pTableBox,
                    sal_uInt32 nRow,
                    sal_uInt32 nCell,
                    sal_uInt32 nDepth);

    WW8TableNodeInfo::Pointer_t
    insertTableNodeInfo(const SwNode * pNode,
                        const SwTable * pTable,
                        const SwTableBox * pTableBox,
                        sal_uInt32 nRow,
                        sal_uInt32 nCell,
                        sal_uInt32 nDepth);

public:
    typedef boost::shared_ptr<WW8TableInfo> Pointer_t;

    WW8TableInfo();
    virtual ~WW8TableInfo();

    void processSwTable(const SwTable * pTable);
    WW8TableNodeInfo::Pointer_t getTableNodeInfo(const SwNode * pNode);
};

}
#endif // WW8_TABLE_INFO_HXX
