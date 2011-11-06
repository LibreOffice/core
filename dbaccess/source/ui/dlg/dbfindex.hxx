/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DBAUI_DBFINDEX_HXX_
#define _DBAUI_DBFINDEX_HXX_

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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

