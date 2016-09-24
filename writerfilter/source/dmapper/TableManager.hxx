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

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TABLEMANAGER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TABLEMANAGER_HXX

#include <dmapper/resourcemodel.hxx>

#include <ooxml/resourceids.hxx>

#include <memory>
#include <stack>
#include "TagLogger.hxx"

#include <rtl/strbuf.hxx>
#include <PropertyMap.hxx>
#include <TableData.hxx>

namespace writerfilter
{
namespace dmapper
{

class DomainMapperTableHandler;

/**
   The table manager.

   This class gets forwarded events from the tokenizer. It gathers the
   table data and after ending the table generates events for the
   table structure. The events have to be handles by a TableDataHandler.

 */
class TableManager
{
    class TableManagerState
    {
        /**
         properties of the current cell
         */
        TablePropertyMapPtr mpCellProps;

        /**
         properties of the current row
         */
        TablePropertyMapPtr mpRowProps;

        /**
         properties of the current table
         */
        std::stack<TablePropertyMapPtr> mTableProps;

        /**
         true if at the end of a row
         */
        bool mbRowEnd;

        /**
         true when in a cell
         */
        bool mbInCell;

        /**
         true when at the end of a cell
         */
        bool mbCellEnd;

    public:
        /**
         Constructor
         */
        TableManagerState()
        : mbRowEnd(false), mbInCell(false), mbCellEnd(false)
        {
        }

        virtual ~TableManagerState()
        {
        }

        void startLevel()
        {
            TablePropertyMapPtr pProps;
            mTableProps.push(pProps);
        }

        void endLevel()
        {
            mTableProps.pop();
        }

        /**
         Reset to initial state at beginning of row.
         */
        void resetCellSpecifics()
        {
            mbRowEnd = false;
            mbInCell = false;
            mbCellEnd = false;
        }

        void resetCellProps()
        {
            mpCellProps.reset();
        }

        void setCellProps(TablePropertyMapPtr pProps)
        {
            mpCellProps = pProps;
        }

        const TablePropertyMapPtr& getCellProps()
        {
            return mpCellProps;
        }

        void resetRowProps()
        {
            mpRowProps.reset();
        }

        void setRowProps(TablePropertyMapPtr pProps)
        {
            mpRowProps = pProps;
        }

        const TablePropertyMapPtr& getRowProps()
        {
            return mpRowProps;
        }

        void resetTableProps()
        {
            if (mTableProps.size() > 0)
                mTableProps.top().reset();
        }

        void setTableProps(TablePropertyMapPtr pProps)
        {
            if (mTableProps.size() > 0)
                mTableProps.top() = pProps;
        }

        TablePropertyMapPtr getTableProps()
        {
            TablePropertyMapPtr pResult;

            if (mTableProps.size() > 0)
                pResult = mTableProps.top();

            return pResult;
        }

        void setInCell(bool bInCell)
        {
            mbInCell = bInCell;
        }

        bool isInCell() const
        {
            return mbInCell;
        }

        void setCellEnd(bool bCellEnd)
        {
            mbCellEnd = bCellEnd;
        }

        bool isCellEnd() const
        {
            return mbCellEnd;
        }

        void setRowEnd(bool bRowEnd)
        {
            mbRowEnd = bRowEnd;
        }

        bool isRowEnd() const
        {
            return mbRowEnd;
        }
    };

    /**
     handle for the current position in document
     */
    css::uno::Reference<css::text::XTextRange> mCurHandle;

    TableManagerState mState;

protected:
    TablePropertyMapPtr getCellProps()
    {
        return mState.getCellProps();
    }

    void setCellProps(TablePropertyMapPtr pProps)
    {
        mState.setCellProps(pProps);
    }

    void resetCellProps()
    {
        mState.resetCellProps();
    }

public:
    TablePropertyMapPtr getRowProps()
    {
        return mState.getRowProps();
    }

    void setRowProps(TablePropertyMapPtr pProps)
    {
        mState.setRowProps(pProps);
    }

    void resetRowProps()
    {
        mState.resetRowProps();
    }

protected:
    void setInCell(bool bInCell)
    {
        mState.setInCell(bInCell);
    }

    bool isInCell() const
    {
        return mState.isInCell();
    }

    void setCellEnd(bool bCellEnd)
    {
        mState.setCellEnd(bCellEnd);
    }

    bool isCellEnd() const
    {
        return mState.isCellEnd();
    }

    void setRowEnd(bool bRowEnd)
    {
        mState.setRowEnd(bRowEnd);
    }

    bool isRowEnd() const
    {
        return mState.isRowEnd();
    }

    TablePropertyMapPtr getTableProps()
    {
        return mState.getTableProps();
    }

    void setTableProps(TablePropertyMapPtr pProps)
    {
        mState.setTableProps(pProps);
    }

    void resetTableProps()
    {
        mState.resetTableProps();
    }

    const css::uno::Reference<css::text::XTextRange>& getHandle()
    {
        return mCurHandle;
    }

    void setHandle(const css::uno::Reference<css::text::XTextRange>& rHandle)
    {
        mCurHandle = rHandle;
    }

private:
    typedef std::shared_ptr< css::uno::Reference<css::text::XTextRange> > T_p;

    /**
       depth of the current cell
    */
    sal_uInt32 mnTableDepthNew;

    /**
        depth of the previous cell
    */
    sal_uInt32 mnTableDepth;

    /**
       stack of table data

       for each level of nested tables there is one frame in the stack
     */
    std::stack<TableData::Pointer_t> mTableDataStack;
    RowData::Pointer_t mpUnfinishedRow;
    bool mbKeepUnfinishedRow;

    /**
       handler for resolveCurrentTable
     */
    std::shared_ptr<DomainMapperTableHandler> mpTableDataHandler;

    /**
       Set flag which indicates the current handle is in a cell.
     */
    void inCell();

    /**
       Set flag which indicate the current handle is at the end of a cell.
    */
    void endCell();

    /**
       Set the table depth of the current cell.

       @param nDepth     the cell depth
     */
    void cellDepth(sal_uInt32 nDepth);

    /**
       Set flag indication the current handle is at the end of a row.
    */
    void endRow();

    /**
       Resolve the current table to the TableDataHandler.
     */
    void resolveCurrentTable();

    /**
     Open a cell at current level.
     */

    void openCell(const css::uno::Reference<css::text::XTextRange>& handle, const TablePropertyMapPtr& pProps);

    /**
     Close a cell at current level.
     */
    void closeCell(const css::uno::Reference<css::text::XTextRange>& handle);

    /**
     Ensure a cell is open at the current level.
    */
    void ensureOpenCell(const TablePropertyMapPtr& pProps);

protected:
    /**
       Return the current table difference, i.e. 1 if we are in the first cell of a new table, etc.
     */
    sal_uInt32 getTableDepthDifference() { return mnTableDepthNew - mnTableDepth; }

    /**
       Action to be carried out at the end of the last paragraph of a
       cell.
     */
    virtual void endOfCellAction();

    /**
       Action to be carried out at the end of the "table row"
       paragraph.
     */
    virtual void endOfRowAction();
    /** let the derived class clear their table related data
     */
    virtual void clearData();

    /** Should we keep the unfinished row in endLevel to initialize the table
        data in the following startLevel.
      */
    void setKeepUnfinishedRow(bool bKeep)
    {
        mbKeepUnfinishedRow = bKeep;
    }


public:
    TableManager();
    virtual ~TableManager(){}

    /**
       Set handler for resolveCurrentTable.

       @param pTableDataHandler     the handler
     */
    void setHandler(const std::shared_ptr<DomainMapperTableHandler>& pTableDataHandler);

    /**
       Set the current handle.

       @param rHandle     the handle
     */
    void handle(const css::uno::Reference<css::text::XTextRange>& rHandle);

    /**
       Start a new table level.

       A new context is pushed onto the table data stack,
     */
    virtual void startLevel();

    /**
       End a table level.

       The current table is resolved and the context is popped from
       the stack.
     */
    virtual void endLevel();

    /**
     * Signal that the next paragraph definitely won't be part of any table.
     */
    void endTable()
    {
        setRowEnd(false);
    }

    /**
       Tells whether a table has been started or not
      */
    bool isInTable();

    /**
       Handle the start of a paragraph group.
     */
    void startParagraphGroup();

    /**
       Handle the end of a paragraph group.
    */
    void endParagraphGroup();

    /**
       Handle an SPRM at current handle.

       @param rSprm   the SPRM
     */
    virtual bool sprm(Sprm & rSprm);

    /**
       Handle occurrence of character 0x7.
     */
    void handle0x7();

    /**
       Handle 8 bit text at current handle.

       @param data    array of characters
       @param len     number of characters to handle
     */
    void text(const sal_uInt8 * data, size_t len);

    /**
       Handle 16 bit text at current handle.

       @param data    array of characters
       @param len     number of characters to handle
     */
    void utext(const sal_uInt8 * data, size_t len);

    /**
       Handle properties of the current cell.

       @param pProps   the properties
     */
    virtual void cellProps(const TablePropertyMapPtr& pProps);

    /**
       Handle properties of a certain cell in the current row.

       @paran i        index of the cell in the current row
       @param pProps   the properties
     */
    virtual void cellPropsByCell(unsigned int i, const TablePropertyMapPtr& pProps);

    /**
       Handle properties of the current row.

       @param pProps   the properties
     */
    virtual void insertRowProps(const TablePropertyMapPtr& pProps);

    /**
       Handle properties of the current table.

       @param pProps   the properties
     */
    virtual void insertTableProps(const TablePropertyMapPtr& pProps);

    /**
       Return if table manager has detected paragraph to ignore.

       If this function returns true the current paragraph contains
       only control information, e.g. end of row.
     */
    bool isIgnore() const;


};

}

}

#endif // INCLUDED_WRITERFILTER_INC_RESOURCEMODEL_TABLEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
