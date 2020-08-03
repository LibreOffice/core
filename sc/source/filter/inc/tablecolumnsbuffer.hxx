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

#pragma once

#include <oox/helper/refvector.hxx>
#include "workbookhelper.hxx"

namespace oox { class AttributeList; }
namespace oox { class SequenceInputStream; }
class ScDBData;

namespace oox::xls {

/** A column in a table (database range).
 */
class TableColumn : public WorkbookHelper
{
public:
    explicit            TableColumn( const WorkbookHelper& rHelper );

    /** Imports table column settings from the tableColumn element. */
    void                importTableColumn( const AttributeList& rAttribs );
    /** Imports table column settings from the TABLECOLUMN (?) record. */
    void                importTableColumn( SequenceInputStream& rStrm );
    /** Gets the name of this column. */
    const OUString&     getName() const;

private:
    OUString            maName;
    sal_Int32           mnId;
    sal_Int32           mnDataDxfId;
};

class TableColumns : public WorkbookHelper
{
public:
    explicit            TableColumns( const WorkbookHelper& rHelper );

    /** Imports settings from the tableColumns element. */
    void                importTableColumns( const AttributeList& rAttribs );
    /** Imports settings from the TABLECOLUMNS (?) record. */
    void                importTableColumns( SequenceInputStream& rStrm );

    /** Creates a new table column and stores it internally. */
    TableColumn&        createTableColumn();

    /** Applies the columns to the passed database range. */
    bool                finalizeImport( ScDBData* pDBData );

private:
    typedef RefVector< TableColumn > TableColumnVector;

    TableColumnVector   maTableColumnVector;
    sal_Int32           mnCount;
};

class TableColumnsBuffer : public WorkbookHelper
{
public:
    explicit            TableColumnsBuffer( const WorkbookHelper& rHelper );

    /** Creates a new table columns object and stores it internally. */
    TableColumns&       createTableColumns();

    /** Applies the table columns to the passed database range. */
    void                finalizeImport( ScDBData* pDBData );

private:
    /** Returns the table columns object used. */
    TableColumns*       getActiveTableColumns();

private:
    typedef RefVector< TableColumns > TableColumnsVector;
    TableColumnsVector  maTableColumnsVector;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
