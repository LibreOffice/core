/*************************************************************************
 *
 *  $RCSfile: dbfindex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-07 16:44:40 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_DBFINDEX_HXX_
#define _DBAUI_DBFINDEX_HXX_

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
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

    GroupBox        m_GB_Indexes;

    FixedText       m_FT_TableIndexes;
    ListBox         aLB_TableIndexes;

    FixedText       m_FT_AllIndexes;
    ListBox         aLB_FreeIndexes;

    PushButton      aPB_Add;
    PushButton      aPB_Remove;
    PushButton      aPB_AddAll;
    PushButton      aPB_RemoveAll;

    DECL_LINK( TableSelectHdl, ComboBox* );
    DECL_LINK( AddClickHdl, PushButton* );
    DECL_LINK( RemoveClickHdl, PushButton* );
    DECL_LINK( AddAllClickHdl, PushButton* );
    DECL_LINK( RemoveAllClickHdl, PushButton* );
    DECL_LINK( OKClickHdl, PushButton* );

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

public:
    ODbaseIndexDialog( Window * pParent, String aDataSrcName );
    virtual ~ODbaseIndexDialog();
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DBFINDEX_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2000/11/23 01:59:15  svesik
 *  Remove stl/ from #include statement
 *
 *  Revision 1.2  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.1  2000/10/05 10:05:38  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 15:49:08  fs
 ************************************************************************/

