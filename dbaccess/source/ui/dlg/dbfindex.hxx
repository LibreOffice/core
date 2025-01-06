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

#include <utility>
#include <vcl/weld.hxx>
#include <deque>

namespace dbaui
{

// OTableIndex
/// represents a single dbf index
class OTableIndex
{
private:
    OUString aIndexFileName;

public:
    OTableIndex() { }
    explicit OTableIndex( OUString aFileName ) : aIndexFileName(std::move( aFileName )) { }

    const OUString& GetIndexFileName() const { return aIndexFileName; }
};

typedef std::deque< OTableIndex >  TableIndexList;

// OTableInfo
/** holds the INF file of a table
*/
class OTableInfo
{
    friend class ODbaseIndexDialog;
private:
    OUString aTableName;
    TableIndexList aIndexList;

public:
    explicit OTableInfo( OUString aName ) : aTableName(std::move(aName)) { }

    void WriteInfFile( const OUString& rDSN ) const;
};

typedef std::deque< OTableInfo >   TableInfoList;

// IndexDialog
class ODbaseIndexDialog : public weld::GenericDialogController
{
    OUString            m_aDSN;
    TableInfoList       m_aTableInfoList;
    TableIndexList      m_aFreeIndexList;

    std::unique_ptr<weld::Button> m_xPB_OK;
    std::unique_ptr<weld::ComboBox> m_xCB_Tables;
    std::unique_ptr<weld::Widget> m_xIndexes;
    std::unique_ptr<weld::TreeView> m_xLB_TableIndexes;
    std::unique_ptr<weld::TreeView> m_xLB_FreeIndexes;

    std::unique_ptr<weld::Button> m_xAdd;
    std::unique_ptr<weld::Button> m_xRemove;
    std::unique_ptr<weld::Button> m_xAddAll;
    std::unique_ptr<weld::Button> m_xRemoveAll;

    DECL_LINK( TableSelectHdl, weld::ComboBox&, void );
    DECL_LINK( AddClickHdl, weld::Button&, void );
    DECL_LINK( RemoveClickHdl, weld::Button&, void );
    DECL_LINK( AddAllClickHdl, weld::Button&, void );
    DECL_LINK( RemoveAllClickHdl, weld::Button&, void );
    DECL_LINK( OKClickHdl, weld::Button&, void );
    DECL_LINK( OnListEntrySelected, weld::TreeView&, void );

protected:
    void        Init();
    void        SetCtrls();

    static OTableIndex implRemoveIndex(const OUString& _rName, TableIndexList& _rList, weld::TreeView& _rDisplay, bool _bMustExist);
    static void implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, weld::TreeView& _rDisplay);

    OTableIndex RemoveFreeIndex( const OUString& _rName, bool _bMustExist ) { return implRemoveIndex(_rName, m_aFreeIndexList, *m_xLB_FreeIndexes, _bMustExist); }
    void        InsertFreeIndex( const OTableIndex& _rIndex ) { implInsertIndex(_rIndex, m_aFreeIndexList, *m_xLB_FreeIndexes); }
    OTableIndex RemoveTableIndex( std::u16string_view _rTableName, const OUString& _rIndexName );
    void        InsertTableIndex( std::u16string_view _rTableName, const OTableIndex& _rIndex );

    void checkButtons();

public:
    ODbaseIndexDialog(weld::Window * pParent, OUString aDataSrcName);
    virtual ~ODbaseIndexDialog() override;
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
