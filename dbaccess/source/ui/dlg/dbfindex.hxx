/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbfindex.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:56:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
    BOOL                m_bCaseSensitiv;

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

