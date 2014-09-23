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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DBFINDEX_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DBFINDEX_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <list>

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
    OTableIndex( const OTableIndex& _rSource) : aIndexFileName(_rSource.aIndexFileName) { }
    OTableIndex( const OUString& rFileName ) : aIndexFileName( rFileName ) { }

    void SetIndexFileName( const OUString& rFileName ) { aIndexFileName = rFileName; }
    OUString GetIndexFileName() const { return aIndexFileName; }
};

typedef ::std::list< OTableIndex >  TableIndexList;

// OTableInfo
class ODbaseIndexDialog;
/** holds the INF file of a table
*/
class OTableInfo
{
    friend class ODbaseIndexDialog;
private:
    OUString aTableName;
    TableIndexList aIndexList;

public:
    OTableInfo() { }
    OTableInfo( const OUString& rName ) : aTableName(rName) { }

    void WriteInfFile( const OUString& rDSN ) const;
};

typedef ::std::list< OTableInfo >   TableInfoList;

// IndexDialog
class ODbaseIndexDialog : public ModalDialog
{
protected:
    OKButton*       m_pPB_OK;
    ComboBox*       m_pCB_Tables;
    VclContainer*   m_pIndexes;
    ListBox*        m_pLB_TableIndexes;
    ListBox*        m_pLB_FreeIndexes;

    PushButton*     m_pAdd;
    PushButton*     m_pRemove;
    PushButton*     m_pAddAll;
    PushButton*     m_pRemoveAll;

    DECL_LINK( TableSelectHdl, ComboBox* );
    DECL_LINK( AddClickHdl, PushButton* );
    DECL_LINK( RemoveClickHdl, PushButton* );
    DECL_LINK( AddAllClickHdl, PushButton* );
    DECL_LINK( RemoveAllClickHdl, PushButton* );
    DECL_LINK( OKClickHdl, PushButton* );
    DECL_LINK( OnListEntrySelected, ListBox* );

    OUString            m_aDSN;
    TableInfoList       m_aTableInfoList;
    TableIndexList      m_aFreeIndexList;
    bool                m_bCaseSensitiv;

    void        Init();
    void        SetCtrls();
    bool    GetTable(const OUString& rName, TableInfoList::iterator& _rPosition);

    OTableIndex implRemoveIndex(const OUString& _rName, TableIndexList& _rList, ListBox& _rDisplay, bool _bMustExist);
    void        implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, ListBox& _rDisplay);

    OTableIndex RemoveFreeIndex( const OUString& _rName, bool _bMustExist ) { return implRemoveIndex(_rName, m_aFreeIndexList, *m_pLB_FreeIndexes, _bMustExist); }
    void        InsertFreeIndex( const OTableIndex& _rIndex ) { implInsertIndex(_rIndex, m_aFreeIndexList, *m_pLB_FreeIndexes); }
    OTableIndex RemoveTableIndex( const OUString& _rTableName, const OUString& _rIndexName, bool _bMustExist );
    void        InsertTableIndex( const OUString& _rTableName, const OTableIndex& _rIndex );

    void checkButtons();

public:
    ODbaseIndexDialog( vcl::Window * pParent, const OUString& aDataSrcName );
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DBFINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
