/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DBAUI_DBFINDEX_HXX_
#define _DBAUI_DBFINDEX_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#ifndef __SGI_STL_LIST
#include <list>
#endif
#include <comphelper/stl_types.hxx>

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OTableIndex
//=========================================================================
/// represents a single dbf index
class OTableIndex
{
private:
    String aIndexFileName;

public:
    OTableIndex() { }
    OTableIndex( const OTableIndex& _rSource) : aIndexFileName(_rSource.aIndexFileName) { }
    OTableIndex( const String& rFileName ) : aIndexFileName( rFileName ) { }

    void SetIndexFileName( const String& rFileName ) { aIndexFileName = rFileName; }
    String GetIndexFileName() const { return aIndexFileName; }
};

//-------------------------------------------------------------------------
typedef ::std::list< OTableIndex >  TableIndexList;
DECLARE_STL_ITERATORS(TableIndexList);

//=========================================================================
//= OTableInfo
//=========================================================================
class ODbaseIndexDialog;
/** holds the INF file of a table
*/
class OTableInfo
{
    friend class ODbaseIndexDialog;
private:
    String aTableName;
    TableIndexList aIndexList;

public:
    OTableInfo() { }
    OTableInfo( const String& rName ) : aTableName(rName) { }

    void WriteInfFile( const String& rDSN ) const;
};

//-------------------------------------------------------------------------
typedef ::std::list< OTableInfo >   TableInfoList;
DECLARE_STL_ITERATORS(TableInfoList);

//////////////////////////////////////////////////////////////////////////
// IndexDialog
class ODbaseIndexDialog : public ModalDialog
{
protected:
    OKButton        aPB_OK;
    CancelButton    aPB_CANCEL;
    HelpButton      aPB_HELP;

    FixedText       m_FT_Tables;
    ComboBox        aCB_Tables;

    FixedLine       m_FL_Indexes;

    FixedText       m_FT_TableIndexes;
    ListBox         aLB_TableIndexes;

    FixedText       m_FT_AllIndexes;
    ListBox         aLB_FreeIndexes;

    ImageButton      aIB_Add;
    ImageButton      aIB_Remove;
    ImageButton      aIB_AddAll;
    ImageButton      aIB_RemoveAll;

    DECL_LINK( TableSelectHdl, ComboBox* );
    DECL_LINK( AddClickHdl, PushButton* );
    DECL_LINK( RemoveClickHdl, PushButton* );
    DECL_LINK( AddAllClickHdl, PushButton* );
    DECL_LINK( RemoveAllClickHdl, PushButton* );
    DECL_LINK( OKClickHdl, PushButton* );
    DECL_LINK( OnListEntrySelected, ListBox* );

    String              m_aDSN;
    TableInfoList       m_aTableInfoList;
    TableIndexList      m_aFreeIndexList;
    sal_Bool                m_bCaseSensitiv;

    void        Init();
    void        SetCtrls();
    sal_Bool    GetTable(const String& rName, TableInfoListIterator& _rPosition);

    OTableIndex implRemoveIndex(const String& _rName, TableIndexList& _rList, ListBox& _rDisplay, sal_Bool _bMustExist);
    void        implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, ListBox& _rDisplay);

    OTableIndex RemoveFreeIndex( const String& _rName, sal_Bool _bMustExist ) { return implRemoveIndex(_rName, m_aFreeIndexList, aLB_FreeIndexes, _bMustExist); }
    void        InsertFreeIndex( const OTableIndex& _rIndex ) { implInsertIndex(_rIndex, m_aFreeIndexList, aLB_FreeIndexes); }
    OTableIndex RemoveTableIndex( const String& _rTableName, const String& _rIndexName, sal_Bool _bMustExist );
    void        InsertTableIndex( const String& _rTableName, const OTableIndex& _rIndex );

    void checkButtons();

public:
    ODbaseIndexDialog( Window * pParent, String aDataSrcName );
    virtual ~ODbaseIndexDialog();
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DBFINDEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
