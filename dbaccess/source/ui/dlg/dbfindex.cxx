/*************************************************************************
 *
 *  $RCSfile: dbfindex.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:45:01 $
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

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_DBF_INDEXES_HRC_
#include "dbfindex.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif


//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::svt;

const ByteString aGroupIdent("dBase III");

//////////////////////////////////////////////////////////////////////////
// Klasse ODbaseIndexDialog
//-------------------------------------------------------------------------
ODbaseIndexDialog::ODbaseIndexDialog( Window * pParent, String aDataSrcName )
    : ModalDialog( pParent, ModuleRes(DLG_DBASE_INDEXES) ),
    aPB_OK(             this, ResId( PB_OK ) ),
    aPB_CANCEL(         this, ResId( PB_CANCEL ) ),
    aPB_HELP(           this, ResId( PB_HELP ) ),
    m_FT_Tables(        this, ResId( FT_TABLES ) ),
    aCB_Tables(         this, ResId( CB_TABLES ) ),
    m_FL_Indexes(       this, ResId( FL_INDEXES ) ),
    m_FT_TableIndexes(  this, ResId( FT_TABLEINDEXES ) ),
    aLB_TableIndexes(   this, ResId( LB_TABLEINDEXES ) ),
    m_FT_AllIndexes(    this, ResId( FT_ALLINDEXES ) ),
    aLB_FreeIndexes(    this, ResId( LB_FREEINDEXES ) ),
    aIB_Add(            this, ResId( IB_ADD ) ),
    aIB_Remove(         this, ResId( IB_REMOVE ) ),
    aIB_AddAll(         this, ResId( IB_ADDALL ) ),
    aIB_RemoveAll(      this, ResId( IB_REMOVEALL ) ),
    m_aDSN(aDataSrcName),
    m_bCaseSensitiv(sal_True)
{
    aCB_Tables.SetSelectHdl( LINK(this, ODbaseIndexDialog, TableSelectHdl) );
    aIB_Add.SetClickHdl( LINK(this, ODbaseIndexDialog, AddClickHdl) );
    aIB_Remove.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveClickHdl) );
    aIB_AddAll.SetClickHdl( LINK(this, ODbaseIndexDialog, AddAllClickHdl) );
    aIB_RemoveAll.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveAllClickHdl) );
    aPB_OK.SetClickHdl( LINK(this, ODbaseIndexDialog, OKClickHdl) );

    aLB_FreeIndexes.SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );
    aLB_TableIndexes.SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );

    aCB_Tables.SetDropDownLineCount(8);
    Init();
    SetCtrls();
    FreeResource();

    // set Hi contrast bitmaps
    aIB_Add.SetModeImage(       ModuleRes(IMG_ONE_LEFT_H),BMP_COLOR_HIGHCONTRAST);
    aIB_AddAll.SetModeImage(    ModuleRes(IMG_ALL_LEFT_H),BMP_COLOR_HIGHCONTRAST);
    aIB_Remove.SetModeImage(    ModuleRes(IMG_ONE_RIGHT_H),BMP_COLOR_HIGHCONTRAST);
    aIB_RemoveAll.SetModeImage( ModuleRes(IMG_ALL_RIGHT_H),BMP_COLOR_HIGHCONTRAST);
}

//-------------------------------------------------------------------------
ODbaseIndexDialog::~ODbaseIndexDialog()
{
}

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
void ODbaseIndexDialog::checkButtons()
{
    aIB_Add.Enable(0 != aLB_FreeIndexes.GetSelectEntryCount());
    aIB_AddAll.Enable(0 != aLB_FreeIndexes.GetEntryCount());

    aIB_Remove.Enable(0 != aLB_TableIndexes.GetSelectEntryCount());
    aIB_RemoveAll.Enable(0 != aLB_TableIndexes.GetEntryCount());
}

//-------------------------------------------------------------------------
OTableIndex ODbaseIndexDialog::implRemoveIndex(const String& _rName, TableIndexList& _rList, ListBox& _rDisplay, sal_Bool _bMustExist)
{
    OTableIndex aReturn;

    sal_Int32 nPos = 0;

    TableIndexListIterator aSearch;
    for (   aSearch = _rList.begin();
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
            if ((sal_uInt32)nPos == _rList.size())
                _rDisplay.SelectEntryPos((sal_uInt16)nPos-1);
            else
                _rDisplay.SelectEntryPos((sal_uInt16)nPos);

            break;
        }
    }

    DBG_ASSERT(!_bMustExist || (aSearch != _rList.end()), "ODbaseIndexDialog::implRemoveIndex : did not find the index!");
    return aReturn;
}

//-------------------------------------------------------------------------
void ODbaseIndexDialog::implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, ListBox& _rDisplay)
{
    _rList.push_front( _rIndex );
    _rDisplay.InsertEntry( _rIndex.GetIndexFileName() );
    _rDisplay.SelectEntryPos(0);
}

//-------------------------------------------------------------------------
OTableIndex ODbaseIndexDialog::RemoveTableIndex( const String& _rTableName, const String& _rIndexName, sal_Bool _bMustExist )
{
    OTableIndex aReturn;

    // does the table exist ?
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return aReturn;

    return implRemoveIndex(_rIndexName, aTablePos->aIndexList, aLB_TableIndexes, _bMustExist);
}

//-------------------------------------------------------------------------
void ODbaseIndexDialog::InsertTableIndex( const String& _rTableName, const OTableIndex& _rIndex)
{
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return;

    implInsertIndex(_rIndex, aTablePos->aIndexList, aLB_TableIndexes);
}

//-------------------------------------------------------------------------
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

//-------------------------------------------------------------------------
IMPL_LINK( ODbaseIndexDialog, AddClickHdl, PushButton*, pButton )
{
    String aSelection = aLB_FreeIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveFreeIndex( aSelection, sal_True );
    InsertTableIndex( aTableName, aIndex );

    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
IMPL_LINK( ODbaseIndexDialog, RemoveClickHdl, PushButton*, pButton )
{
    String aSelection = aLB_TableIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveTableIndex( aTableName, aSelection, sal_True );
    InsertFreeIndex( aIndex );

    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
IMPL_LINK( ODbaseIndexDialog, AddAllClickHdl, PushButton*, pButton )
{
    sal_uInt16 nCnt = aLB_FreeIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();
    String aEntry;

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertTableIndex( aTableName, RemoveFreeIndex( aLB_FreeIndexes.GetEntry(0), sal_True ) );

    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
IMPL_LINK( ODbaseIndexDialog, RemoveAllClickHdl, PushButton*, pButton )
{
    sal_uInt16 nCnt = aLB_TableIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();
    String aEntry;

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertFreeIndex( RemoveTableIndex( aTableName, aLB_TableIndexes.GetEntry(0), sal_True ) );

    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
IMPL_LINK( ODbaseIndexDialog, OnListEntrySelected, ListBox*, NOTINTERESTEDIN )
{
    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
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

    checkButtons();
    return 0;
}

//-------------------------------------------------------------------------
void ODbaseIndexDialog::Init()
{
    aPB_OK.Disable();
    m_FL_Indexes.Disable();
    m_FT_TableIndexes.Disable();
    aLB_TableIndexes.Disable();
    m_FT_AllIndexes.Disable();
    aLB_FreeIndexes.Disable();
    aIB_Add.Disable();
    aIB_Remove.Disable();
    aIB_AddAll.Disable();
    aIB_RemoveAll.Disable();

    ///////////////////////////////////////////////////////////////////////////
    // Alle Indizes werden erst einmal zur Liste der freien Indizes hinzugefuegt.
    // Dann wird fuer jede Tabelle in der Inf-Datei nachgeschaut, welche Indizes sie besitzt.
    // Diese Indizes werden aus der Liste der freien Indizes entfernt
    // und in die Indexliste der Tabelle eingetragen

    ///////////////////////////////////////////////////////////////////////////
    // if the string does not contain a path, cut the string
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    {
        SvtPathOptions aPathOptions;
        m_aDSN = aPathOptions.SubstituteVariable(m_aDSN);
    }
    aURL.SetSmartURL(m_aDSN);


    //  String aFileName = aURL.PathToFileName();
    m_aDSN = aURL.GetMainURL(INetURLObject::NO_DECODE);
    ::ucb::Content aFile;
    sal_Bool bFolder=sal_True;
    try
    {
        aFile = ::ucb::Content(m_aDSN,Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        bFolder = aFile.isFolder();
    }
    catch(Exception&)
    {
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    // first assume for all indexes they're free

    Sequence< ::rtl::OUString> aFolderContent( ::utl::LocalFileHelper::GetFolderContents(m_aDSN,bFolder));

    ::rtl::OUString aIndexExt = ::rtl::OUString::createFromAscii("ndx");
    ::rtl::OUString aTableExt = ::rtl::OUString::createFromAscii("dbf");

    ::std::vector< String > aUsedIndexes;

    String aExt;
    const ::rtl::OUString *pBegin = aFolderContent.getConstArray();
    const ::rtl::OUString *pEnd   = pBegin + aFolderContent.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
        String aName;
        ::utl::LocalFileHelper::ConvertURLToPhysicalName(pBegin->getStr(),aName);
        aURL.SetSmartURL(aName);
        aExt = aURL.getExtension();
        if(aExt == aIndexExt.getStr())
        {
            m_aFreeIndexList.push_back( OTableIndex(aURL.getName()) );
        }
        else if(aExt == aTableExt.getStr())
        {
            m_aTableInfoList.push_back( OTableInfo(aURL.getName()) );
            OTableInfo& rTabInfo = m_aTableInfoList.back();

            // open the INF file
            aURL.setExtension(String::CreateFromAscii("inf"));
            OFileNotation aTransformer(aURL.GetURLNoPass(), OFileNotation::N_URL);
            Config aInfFile( aTransformer.get(OFileNotation::N_SYSTEM) );
            aInfFile.SetGroup( aGroupIdent );

            ///////////////////////////////////////////////////////////////////////////
            // fill the indexes list
            ByteString aNDX;
            sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
            ByteString aKeyName;
            String aEntry;

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
                    aUsedIndexes.push_back(aEntry);
                        // do this later below. We may not have encountered the index file, yet, thus we may not
                        // know the index as beeing free, yet
                }

            }
        }
    }

    for (   ::std::vector< String >::const_iterator aUsedIndex = aUsedIndexes.begin();
            aUsedIndex != aUsedIndexes.end();
            ++aUsedIndex
        )
        RemoveFreeIndex( *aUsedIndex, sal_False );

    if (m_aTableInfoList.size())
    {
        aPB_OK.Enable();
        m_FL_Indexes.Enable();
        m_FT_TableIndexes.Enable();
        aLB_TableIndexes.Enable();
        m_FT_AllIndexes.Enable();
        aLB_FreeIndexes.Enable();
    }

    checkButtons();
}

//-------------------------------------------------------------------------
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
                aIndex != rTabInfo.aIndexList.end();
                ++aIndex
            )
            aLB_TableIndexes.InsertEntry( aIndex->GetIndexFileName() );

        if( rTabInfo.aIndexList.size() )
            aLB_TableIndexes.SelectEntryPos( 0 );

    }

    // ListBox freie Indizes
    for (   ConstTableIndexListIterator aFree = m_aFreeIndexList.begin();
            aFree != m_aFreeIndexList.end();
            ++aFree
        )
        aLB_FreeIndexes.InsertEntry( aFree->GetIndexFileName() );

    if( m_aFreeIndexList.size() )
        aLB_FreeIndexes.SelectEntryPos( 0 );


    TableSelectHdl(&aCB_Tables);
    checkButtons();
}

//////////////////////////////////////////////////////////////////////////
// Klasse OTableInfo
//-------------------------------------------------------------------------
void OTableInfo::WriteInfFile( const String& rDSN ) const
{
    // INF-Datei oeffnen
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    String aDsn = rDSN;
    {
        SvtPathOptions aPathOptions;
        aDsn = aPathOptions.SubstituteVariable(aDsn);
    }
    aURL.SetSmartURL(aDsn);
    aURL.Append(aTableName);
    aURL.setExtension(String::CreateFromAscii("inf"));

    OFileNotation aTransformer(aURL.GetURLNoPass(), OFileNotation::N_URL);
    Config aInfFile( aTransformer.get(OFileNotation::N_SYSTEM) );
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
    if(!nPos)
    {
        try
        {
            ::ucb::Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
            aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),makeAny( sal_Bool( sal_True ) ) );
        }
        catch (const Exception& e )
        {
            e;  // make compiler happy
            // simply silent this. The strange algorithm here does a lot of things even if no files at all were
            // created or access, so it's possible that the file we're trying to delete does not even exist,
            // and this is a valid condition.
            // 2003-05-15 - #109677# - fs@openoffice.org
        }
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

