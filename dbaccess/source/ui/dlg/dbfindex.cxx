/*************************************************************************
 *
 *  $RCSfile: dbfindex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-09 12:55:31 $
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
#include "dbfindex.hxx"
#endif

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_DBF_INDEXES_HRC_
#include "dbfindex.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

const ByteString aGroupIdent("dbase III");

//////////////////////////////////////////////////////////////////////////
// Klasse ODbaseIndexDialog
ODbaseIndexDialog::ODbaseIndexDialog( Window * pParent, String aDataSrcName )
    : ModalDialog( pParent, ModuleRes(DLG_DBASE_INDEXES) ),
    aPB_OK(             this, ResId( PB_OK ) ),
    aPB_CANCEL(         this, ResId( PB_CANCEL ) ),
    aPB_HELP(           this, ResId( PB_HELP ) ),
    m_FT_Tables(        this, ResId( FT_TABLES ) ),
    aCB_Tables(         this, ResId( CB_TABLES ) ),
    m_GB_Indexes(       this, ResId( GB_INDEXES ) ),
    m_FT_TableIndexes(  this, ResId( FT_TABLEINDEXES ) ),
    aLB_TableIndexes(   this, ResId( LB_TABLEINDEXES ) ),
    m_FT_AllIndexes(    this, ResId( FT_ALLINDEXES ) ),
    aLB_FreeIndexes(    this, ResId( LB_FREEINDEXES ) ),
    aPB_Add(            this, ResId( PB_ADD ) ),
    aPB_Remove(         this, ResId( PB_REMOVE ) ),
    aPB_AddAll(         this, ResId( PB_ADDALL ) ),
    aPB_RemoveAll(      this, ResId( PB_REMOVEALL ) ),
    m_aDSN(aDataSrcName)
{
    aCB_Tables.SetSelectHdl( LINK(this, ODbaseIndexDialog, TableSelectHdl) );
    aPB_Add.SetClickHdl( LINK(this, ODbaseIndexDialog, AddClickHdl) );
    aPB_Remove.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveClickHdl) );
    aPB_AddAll.SetClickHdl( LINK(this, ODbaseIndexDialog, AddAllClickHdl) );
    aPB_RemoveAll.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveAllClickHdl) );
    aPB_OK.SetClickHdl( LINK(this, ODbaseIndexDialog, OKClickHdl) );

    Init();
    SetCtrls();
    FreeResource();
}

ODbaseIndexDialog::~ODbaseIndexDialog()
{
}

sal_Bool ODbaseIndexDialog::GetTable(const String& _rName, TableInfoListIterator& _rPosition)
{
    for (   _rPosition = m_aTableInfoList.begin();
            _rPosition != m_aTableInfoList.end();
            ++_rPosition
        )
    {
        if (m_bCaseSensitiv)
        {
            if (_rPosition->aTableName.Equals(_rName))
                return sal_True;
        }
        else
        {
            if (_rPosition->aTableName.EqualsIgnoreCaseAscii(_rName))
                return sal_True;
        }
    }
    return sal_False;
}

OTableIndex ODbaseIndexDialog::implRemoveIndex(const String& _rName, TableIndexList& _rList, ListBox& _rDisplay)
{
    OTableIndex aReturn;

    sal_Int32 nPos = 0;
    for (   TableIndexListIterator aSearch = _rList.begin();
            aSearch != _rList.end();
            ++aSearch, ++nPos
        )
    {
        if ( m_bCaseSensitiv ? aSearch->GetIndexFileName().Equals(_rName) : aSearch->GetIndexFileName().EqualsIgnoreCaseAscii(_rName) )
        {
            aReturn = *aSearch;

            _rList.erase(aSearch);
            _rDisplay.RemoveEntry( _rName );

            // adjust selection if necessary
            if (nPos == _rList.size())
                _rDisplay.SelectEntryPos((sal_uInt16)nPos-1);
            else
                _rDisplay.SelectEntryPos((sal_uInt16)nPos);

            break;
        }
    }

    DBG_ASSERT(aSearch != _rList.end(), "ODbaseIndexDialog::RemoveFreeIndex : did not find the index!");
    return aReturn;
}

void ODbaseIndexDialog::implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, ListBox& _rDisplay)
{
    _rList.push_front( _rIndex );
    _rDisplay.InsertEntry( _rIndex.GetIndexFileName() );
    _rDisplay.SelectEntryPos(0);
}

OTableIndex ODbaseIndexDialog::RemoveTableIndex( const String& _rTableName, const String& _rIndexName )
{
    OTableIndex aReturn;

    // does the table exist ?
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return aReturn;

    return implRemoveIndex(_rIndexName, aTablePos->aIndexList, aLB_TableIndexes);
}

void ODbaseIndexDialog::InsertTableIndex( const String& _rTableName, const OTableIndex& _rIndex )
{
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return;

    implInsertIndex(_rIndex, aTablePos->aIndexList, aLB_TableIndexes);
}

IMPL_LINK( ODbaseIndexDialog, OKClickHdl, PushButton*, pButton )
{
    // let all tables write their INF file

    for (   ConstTableInfoListIterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        aLoop->WriteInfFile(m_aDSN);

    EndDialog();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, AddClickHdl, PushButton*, pButton )
{
    String aSelection = aLB_FreeIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveFreeIndex( aSelection );
    InsertTableIndex( aTableName, aIndex );
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, RemoveClickHdl, PushButton*, pButton )
{
    String aSelection = aLB_TableIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveTableIndex( aTableName, aSelection );
    InsertFreeIndex( aIndex );

    return 0;
}

IMPL_LINK( ODbaseIndexDialog, AddAllClickHdl, PushButton*, pButton )
{
    sal_uInt16 nCnt = aLB_FreeIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();
    String aEntry;

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertTableIndex( aTableName, RemoveFreeIndex( aLB_FreeIndexes.GetEntry(0) ) );

    return 0;
}

IMPL_LINK( ODbaseIndexDialog, RemoveAllClickHdl, PushButton*, pButton )
{
    sal_uInt16 nCnt = aLB_TableIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();
    String aEntry;

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertFreeIndex( RemoveTableIndex( aTableName, aLB_TableIndexes.GetEntry(0) ) );

    return 0;
}

IMPL_LINK( ODbaseIndexDialog, TableSelectHdl, ComboBox*, pComboBox )
{
    // search the table
    TableInfoListIterator aTablePos;
    if (!GetTable(pComboBox->GetText(), aTablePos))
        return 0L;

    // fill the listbox for the indexes
    aLB_TableIndexes.Clear();
    for (   ConstTableIndexListIterator aLoop = aTablePos->aIndexList.begin();
            aLoop != aTablePos->aIndexList.end();
            ++aLoop
        )
        aLB_TableIndexes.InsertEntry( aLoop->GetIndexFileName() );

    if ( aTablePos->aIndexList.size() )
        aLB_TableIndexes.SelectEntryPos(0);

    return 0;
}

void ODbaseIndexDialog::Init()
{
    ///////////////////////////////////////////////////////////////////////////
    // Alle Indizes werden erst einmal zur Liste der freien Indizes hinzugefuegt.
    // Dann wird fuer jede Tabelle in der Inf-Datei nachgeschaut, welche Indizes sie besitzt.
    // Diese Indizes werden aus der Liste der freien Indizes entfernt
    // und in die Indexliste der Tabelle eingetragen

    ///////////////////////////////////////////////////////////////////////////
    // if the string does not contain a path, cut the string
    DirEntry aInitEntry( m_aDSN );
    FileStat aFileStat(aInitEntry);     // do not combine - bug in VC++ 2.0!

    if (aFileStat.IsKind(FSYS_KIND_WILD))
        m_aDSN = (DirEntry(m_aDSN).GetPath()).GetFull();
    else if (aFileStat.IsKind(FSYS_KIND_DIR))
        m_aDSN = DirEntry(m_aDSN).GetFull();
    else if (aFileStat.IsKind(FSYS_KIND_FILE))
        m_aDSN = (DirEntry(m_aDSN).GetPath()).GetFull();

    ///////////////////////////////////////////////////////////////////////////
    // first assume for all indexes they're free
    DirEntry aDirEntry( m_aDSN );

    m_bCaseSensitiv = aDirEntry.IsCaseSensitive();

    aDirEntry += String::CreateFromAscii("*.ndx");
    Dir* pDir = new Dir( aDirEntry, FSYS_KIND_FILE,FSYS_SORT_NAME | FSYS_SORT_ASCENDING | FSYS_SORT_END );

    sal_uInt16 nCount = pDir->Count();
    String aEntry;

    for( sal_uInt16 nCurPos = 0; nCurPos < nCount; nCurPos++ )
    {
        aEntry = (*pDir)[nCurPos].GetName();
        m_aFreeIndexList.push_back( OTableIndex(aEntry) );
    }

    ///////////////////////////////////////////////////////////////////////////
    // fill the table list
    aDirEntry.SetExtension(String::CreateFromAscii("dbf"));
    delete pDir;
    pDir = new Dir( aDirEntry, FSYS_KIND_FILE,FSYS_SORT_NAME | FSYS_SORT_ASCENDING | FSYS_SORT_END );

    nCount = pDir->Count();
    if(!nCount)
    {
        aPB_OK.Disable(sal_True);
        m_GB_Indexes.Disable(sal_True);
        m_FT_TableIndexes.Disable(sal_True);
        aLB_TableIndexes.Disable(sal_True);
        m_FT_AllIndexes.Disable(sal_True);
        aLB_FreeIndexes.Disable(sal_True);
        aPB_Add.Disable(sal_True);
        aPB_Remove.Disable(sal_True);
        aPB_AddAll.Disable(sal_True);
        aPB_RemoveAll.Disable(sal_True);
    }

    for( nCurPos = 0; nCurPos < nCount; nCurPos++ )
    {
        // add the table to the list
        aEntry = (*pDir)[nCurPos].GetName();
        m_aTableInfoList.push_back( OTableInfo(aEntry) );
        OTableInfo& rTabInfo = m_aTableInfoList.back();

        // open the INF file
        String aTableName = (*pDir)[nCurPos].GetBase();
        String aFileName = m_aDSN;
        aFileName += DirEntry::GetAccessDelimiter();
        aFileName += aTableName;
        aFileName.AppendAscii(".inf");
        Config aInfFile( aFileName );
        aInfFile.SetGroup( aGroupIdent );

        ///////////////////////////////////////////////////////////////////////////
        // fill the indexes list
        ByteString aNDX;
        sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
        ByteString aKeyName;

        for( sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++ )
        {
            // does the key point to an index file ?
            aKeyName = aInfFile.GetKeyName( nKey );
            aNDX = aKeyName.Copy(0,3);

            // yes -> add to the tables index list
            if (aNDX == "NDX" )
            {
                aEntry = String(aInfFile.ReadKey(aKeyName), gsl_getSystemTextEncoding());
                rTabInfo.aIndexList.push_back( OTableIndex( aEntry ) );

                // and remove it from the free index list
                RemoveFreeIndex( aEntry );
            }

        }

    }

    delete pDir;
}

void ODbaseIndexDialog::SetCtrls()
{
    // ComboBox Tabellen
    for (   ConstTableInfoListIterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        aCB_Tables.InsertEntry( aLoop->aTableName );

    // Den ersten Datensatz ins Edit stellen
    if( m_aTableInfoList.size() )
    {
        const OTableInfo& rTabInfo = m_aTableInfoList.front();
        aCB_Tables.SetText( rTabInfo.aTableName );

        // ListBox der Tabellenindizes aufbauen
        for (   ConstTableIndexListIterator aIndex = rTabInfo.aIndexList.begin();
                aIndex != rTabInfo.aIndexList.begin();
                ++aIndex
            )
            aLB_TableIndexes.InsertEntry( aIndex->GetIndexFileName() );

        if( rTabInfo.aIndexList.size() )
            aLB_TableIndexes.SelectEntryPos( 0 );

    }

    // ListBox freie Indizes
    for (   ConstTableIndexListIterator aFree = m_aFreeIndexList.begin();
            aFree != m_aFreeIndexList.begin();
            ++aFree
        )
        aLB_FreeIndexes.InsertEntry( aFree->GetIndexFileName() );

    if( m_aFreeIndexList.size() )
        aLB_FreeIndexes.SelectEntryPos( 0 );

}

//////////////////////////////////////////////////////////////////////////
// Klasse OTableInfo
void OTableInfo::WriteInfFile( const String& rDSN ) const
{
    // INF-Datei oeffnen
    xub_StrLen nLen = aTableName.Len();
    String aName = aTableName.Copy( 0,(nLen-4) ); // Extension ausblenden
    String aFileName = rDSN;
    aFileName += '\\';
    aFileName += aName;
    aFileName.AppendAscii(".inf");
    Config aInfFile( aFileName );
    aInfFile.SetGroup( aGroupIdent );

    // Erst einmal alle Tabellenindizes loeschen
    ByteString aNDX;
    sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
    ByteString aKeyName;
    ByteString aEntry;
    sal_uInt16 nKey = 0;

    while( nKey < nKeyCnt )
    {
        // Verweist der Key auf ein Indexfile?...
        aKeyName = aInfFile.GetKeyName( nKey );
        aNDX = aKeyName.Copy(0,3);

        //...wenn ja, Indexfile loeschen, nKey steht dann auf nachfolgendem Key
        if( aNDX == "NDX" )
        {
            aInfFile.DeleteKey(aKeyName);
            nKeyCnt--;
        }
        else
            nKey++;

    }

    // Jetzt alle gespeicherten Indizes hinzufuegen
    sal_uInt16 nPos = 0;
    for (   ConstTableIndexListIterator aIndex = aIndexList.begin();
            aIndex != aIndexList.end();
            ++aIndex, ++nPos
        )
    {
        aKeyName = "NDX";
        if( nPos > 0 )  // Erster Index erhaelt keine Ziffer
            aKeyName += ByteString::CreateFromInt32( nPos );
        aInfFile.WriteKey( aKeyName, ByteString(aIndex->GetIndexFileName(), gsl_getSystemTextEncoding()) );
    }

    aInfFile.Flush();

    // Falls nur noch [dbase] in INF-File steht, Datei loeschen
    DirEntry aDirEntry( aFileName );
    FileStat aFileStat( aDirEntry );
    sal_uInt32 nFileSize = aFileStat.GetSize();
    xub_StrLen nGroupIdLen = aGroupIdent.Len();

    if( (xub_StrLen)nFileSize == (nGroupIdLen+4) )
        aDirEntry.Kill();
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 10:05:22  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 15:54:26  fs
 ************************************************************************/

