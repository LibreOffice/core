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

#ifndef _PRJ_HXX
#define _PRJ_HXX

#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <soldep/listmacr.hxx>
#include <vos/mutex.hxx>
#include <tools/string.hxx>

#define XML_EXT "xlist"

#define OS_NONE             0x0000
#define OS_WIN16            0x0001
#define OS_WIN32            0x0002
#define OS_OS2              0x0004
#define OS_UNX              0x0008
#define OS_MAC              0x0010
#define OS_ALL              ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX | OS_MAC )

#define COMMAND_PROJECTROOT 0x0000
#define COMMAND_NMAKE       0x0001
#define COMMAND_GET         0x0002
#define COMMAND_USER_START  0x0003
#define COMMAND_USER_END    0xFFFE
#define COMMAND_ALLDIRS     0xFFFF

class SByteStringList;
class GenericInformationList;

/*
// Pfade auf Konfigurationsdateien des Build-Servers

#define REQUEST_DIR         \\src\data4\source\b_server\server\newjob
*/
/*********************************************************************
*
*   Die Klasse CommandData haelte alle Informationen, die fuer die
*   Abarbeitung eines Kommandos (nmake, get) noetig sind
*
*********************************************************************/

class CommandData
{
    ByteString      aPrj;
    ByteString      aLogFileName;
    ByteString      aInpath;
    ByteString      aUpd;
    ByteString      aUpdMinor;
    ByteString      aProduct;
    ByteString      aCommand;
    ByteString      aPath;
    ByteString      aPrePath;
    ByteString      aPreFix;
    ByteString      aCommandPara;
    ByteString      aComment;
    ByteString      sClientRestriction;
    SByteStringList *pDepList;
    SByteStringList *pCommandList;
    sal_uInt16      nOSType;
    sal_uInt16      nCommand;

    sal_uIntPtr     nDepth;             // Tiefe der Abhaenigkeit

public:
                CommandData();
                ~CommandData();
    ByteString      GetProjectName(){return aPrj;}
    void        SetProjectName( ByteString aName ){aPrj = aName;}
    ByteString      GetLogFile(){return aLogFileName;}
    void        SetLogFile( ByteString aName ){aLogFileName = aName;}
    ByteString      GetInpath(){return aInpath;}
    void        SetInpath( ByteString aName ){aInpath = aName;}
    ByteString      GetUpd(){return aUpd;}
    void        SetUpd( ByteString aName ){aUpd = aName;}
    ByteString      GetUpdMinor(){return aUpdMinor;}
    void        SetUpdMinor( ByteString aName ){aUpdMinor = aName;}
    ByteString      GetProduct(){return aProduct;}
    void        SetProduct( ByteString aName ){aProduct = aName;}
    ByteString      GetCommand(){return aCommand;}
    void        SetCommand ( ByteString aName ){aCommand = aName;}
    ByteString      GetCommandPara(){return aCommandPara;}
    void        SetCommandPara ( ByteString aName ){aCommandPara = aName;}
    ByteString      GetComment(){return aComment;}
    void        SetComment ( ByteString aCommentString ){aComment = aCommentString;}
    ByteString      GetPath(){return aPath;}
    void        SetPath( ByteString aName ){aPath = aName;}
    ByteString      GetPrePath(){return aPrePath;}
    void        SetPrePath( ByteString aName ){aPrePath = aName;}
    sal_uInt16      GetOSType(){return nOSType;}
    ByteString      GetOSTypeString();
    void        SetOSType( sal_uInt16 nType ){nOSType = nType;}
    sal_uInt16      GetCommandType(){return nCommand;}
    ByteString      GetCommandTypeString();
    void        SetCommandType( sal_uInt16 nCommandType ){nCommand = nCommandType;}
    SByteStringList* GetDependencies(){return pDepList;}
    void        SetDependencies( SByteStringList *pList ){pDepList = pList;}
    ByteString      GetClientRestriction() { return sClientRestriction; }
    void        SetClientRestriction( ByteString sRestriction ) { sClientRestriction = sRestriction; }

    void        AddDepth(){nDepth++;}
    sal_uIntPtr     GetDepth(){return nDepth;}

    void AddCommand(ByteString* pCommand);
    SByteStringList* GetCommandList() {return pCommandList;}

    CommandData& operator<<  ( SvStream& rStream );
    CommandData& operator>>  ( SvStream& rStream );
};

/*********************************************************************
*
*   Die Klasse SimpleConfig kann benutzt werden, um aus einer Textdatei
*   alle Tokens zu lesen
*
*********************************************************************/

class SimpleConfig
{
    long            nLine;
    String          aFileName;
    SvFileStream    aFileStream;
    ByteString          aTmpStr;
    ByteString          aStringBuffer;

    ByteString          GetNextLine();
public:
                    SimpleConfig(String aSimpleConfigFileName);
                    SimpleConfig(DirEntry& rDirEntry);
                    ~SimpleConfig();
    ByteString          GetNext();
    ByteString          GetCleanedNextLine( sal_Bool bReadComments = sal_False );
};

#define ENV_GUI     0x00000000
#define ENV_OS      0x00000001
#define ENV_UPD     0x00000002
#define ENV_UPDMIN  0x00000004
#define ENV_INPATH  0x00000008
#define ENV_OUTPATH 0x00000010
#define ENV_GUIBASE 0x00000020
#define ENV_CVER    0x00000040
#define ENV_GVER    0x00000080
#define ENV_GUIENV  0x00000100
#define ENV_CPU     0x00000200
#define ENV_CPUNAME 0x00000400
#define ENV_DLLSUFF 0x00000800
#define ENV_COMEX   0x00001000
#define ENV_COMPATH 0x00002000
#define ENV_INCLUDE 0x00004000
#define ENV_LIB     0x00008000
#define ENV_PATH    0x00010000
#define ENV_SOLVER  0x00020000
#define ENV_SOLENV  0x00040000
#define ENV_SOLROOT 0x00080000
#define ENV_DEVROOT 0x00100000
#define ENV_EMERG   0x00200000
#define ENV_STAND   0x00400000

DECL_DEST_LIST ( PrjList_tmp, PrjList, CommandData * )

/*********************************************************************
*
*   class DepInfo
*   Struct mit Modeinfos
*
*********************************************************************/

class DepInfo
{
    private:
        ByteString*         pProject;
        SByteStringList*    pModeList;
        sal_Bool                bAllModes;

        void                RemoveProject ();
        void                RemoveModeList ();
    public:
                            DepInfo() : pProject(0), pModeList(0), bAllModes(sal_False) {}
                            DepInfo(ByteString* pString) : pProject(0), pModeList(0), bAllModes(sal_False) {pProject = pString;}
                            DepInfo(ByteString* pString, SByteStringList* pList) : pProject(0), pModeList(0), bAllModes(sal_False) {pProject = pString; pModeList = pList;}
                            ~DepInfo();

        void                SetProject (ByteString* pStr);
        ByteString*         GetProject() {return pProject;}
        void                PutModeString (ByteString* pStr);
        SByteStringList*    GetModeList() {return pModeList;}
        void                SetModeList (SByteStringList* pList) {pModeList = pList;}
        sal_Bool                IsAllModes() {return bAllModes;}
        void                SetAllModes(sal_Bool bModes=sal_True) {bAllModes = bModes;}

        DepInfo&            operator<<  ( SvStream& rStream );
        DepInfo&            operator>>  ( SvStream& rStream );
};

DECLARE_LIST( DepInfoList, DepInfo* )

/*********************************************************************
*
*   class SDepInfoList
*   Struct mit Modeinfos + Listen Methoden
*
*********************************************************************/

class SDepInfoList : public DepInfoList
{
    SByteStringList*    pAllModeList; // only pointer to strings

    public:
                SDepInfoList();
                ~SDepInfoList();

                // neuen ByteString in Liste einfuegen
    sal_uIntPtr     PutString( ByteString*, ByteString*);
    sal_uIntPtr     PutString( ByteString*);
    void        PutModeString( DepInfo* pInfoList, ByteString* pStr );

    ByteString*     RemoveString( const ByteString& rName );

                // Position des ByteString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    sal_uIntPtr     IsString( ByteString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    sal_uIntPtr     GetPrevString( ByteString* );
    SByteStringList*    GetAllDepModes();

    SDepInfoList& operator<< ( SvStream& rStream );
    SDepInfoList& operator>> ( SvStream& rStream );
};


/*********************************************************************
*
*   class Prj
*   alle Daten eines Projektes werden hier gehalten
*
*********************************************************************/

class Star;
class Prj : public PrjList
{
friend class Star;
private:

    ByteString          aPrjPath;
    ByteString          aProjectName;
    ByteString          aProjectPrefix;     // max. 2-buchstabige Abk.
    SByteStringList*    pPrjInitialDepList;
    SByteStringList*    pPrjDepList;
    SDepInfoList*       pPrjDepInfoList;
    sal_Bool            bSorted;
    sal_Bool            bHardDependencies;
    sal_Bool            bFixedDependencies;
    sal_Bool            bVisited;
    sal_Bool            bIsAvailable;
    SByteStringList* RemoveStringList(SByteStringList* pStringList );
    SDepInfoList*   RemoveDepInfoList(SDepInfoList* pInfoList );
    PrjList*        pTempCommandDataList;
    sal_Bool            bTempCommandDataListPermanent;
    sal_Bool            bError;
public:
                    Prj();
                    Prj( ByteString aName );
                    ~Prj();
    void            SetPreFix( ByteString aPre ){aProjectPrefix = aPre;}
    ByteString          GetPreFix(){return aProjectPrefix;}
    ByteString          GetProjectName()
                            {return aProjectName;}
    void            SetProjectName(ByteString aName)
                            {aProjectName = aName;}
    sal_Bool            InsertDirectory( ByteString aDirName , sal_uInt16 aWhat,
                                    sal_uInt16 aWhatOS, ByteString aLogFileName,
                                    const ByteString &rClientRestriction );
    CommandData*    RemoveDirectory( ByteString aLogFileName );
    CommandData*    GetDirectoryList ( sal_uInt16 nWhatOs, sal_uInt16 nCommand );
    CommandData*    GetDirectoryData( ByteString aLogFileName );
    inline CommandData* GetData( ByteString aLogFileName )
                            { return GetDirectoryData( aLogFileName ); };

    SByteStringList*    GetDependencies( sal_Bool bExpanded = sal_True );
    SDepInfoList*   GetModeAndDependencies() {return pPrjDepInfoList;}
    void            AddDependencies( ByteString aStr );
    void            AddDependencies( ByteString aStr, ByteString aModeStr );
    void            SetMode(SByteStringList* pModeList);
    void            HasHardDependencies( sal_Bool bHard ) { bHardDependencies = bHard; }
    sal_Bool            HasHardDependencies() { return bHardDependencies; }
    void            HasFixedDependencies( sal_Bool bFixed ) { bFixedDependencies = bFixed; }
    sal_Bool            HasFixedDependencies() { return bFixedDependencies; }

    sal_Bool            IsAvailable() { return bIsAvailable; }
    void            IsAvailable( sal_Bool bAvailable ) { bIsAvailable=bAvailable; }

    void            ExtractDependencies();

    PrjList*        GetCommandDataList ();
    void            RemoveTempCommandDataList();
    void            GenerateTempCommandDataList();
    void            GenerateEmptyTempCommandDataList();
    sal_Bool            HasTempCommandDataList() {return pTempCommandDataList != NULL;}
    void            SetTempCommandDataListPermanent (sal_Bool bVar = sal_True) {bTempCommandDataListPermanent = bVar;}
    sal_Bool            IsTempCommandDataListPermanent() {return bTempCommandDataListPermanent;}

    void            SetError (sal_Bool bVar = sal_True) {bError = bVar;}
    sal_Bool            HasError () {return bError;}

    Prj&            operator<<  ( SvStream& rStream );
    Prj&            operator>>  ( SvStream& rStream );
};

/*********************************************************************
*
*   class Star
*   Diese Klasse liest die Projectstruktur aller StarDivision Projekte
*   aus \\dev\data1\upenv\data\config\solar.lst aus
*
*********************************************************************/

DECL_DEST_LIST ( StarList_tmp, StarList, Prj* )
DECLARE_LIST ( SolarFileList, String* )

class StarFile
{
private:
    String aFileName;
    Date aDate;
    Time aTime;
    Date aDateCreated;
    Time aTimeCreated;

    sal_Bool bExists;

public:
    StarFile( const String &rFile );
    const String &GetName() { return aFileName; }
    Date GetDate() { return aDate; }
    Time GetTime() { return aTime; }

    sal_Bool NeedsUpdate();
    sal_Bool Exists() { return bExists; }
};

DECLARE_LIST( StarFileList, StarFile * )

#define STAR_MODE_SINGLE_PARSE          0x0000
//#define STAR_MODE_RECURSIVE_PARSE     0x0001      it dosen't work anymore
#define STAR_MODE_MULTIPLE_PARSE        0x0002

class Star : public StarList
{
private:
    Link aDBNotFoundHdl;
    ByteString      aStarName; // no idee what this should be

protected:
    vos:: OMutex  aMutex;

    sal_uInt16          nStarMode;
    SolarFileList   aFileList;
    StarFileList    aLoadedFilesList;
    String          sSourceRoot;
    String          sFileName;
    SByteStringList* pDepMode;
    SByteStringList* pAllDepMode;

    Link aFileIOErrorHdl; // called with &String as parameter!!!

    void InsertSolarList( String sProject );
    String CreateFileName( String& rProject, String& rSourceRoot );

    void            Expand_Impl();
    void            ExpandPrj_Impl( Prj *pPrj, Prj *pDepPrj );
    sal_uIntPtr         SearchFileEntry( StarFileList *pStarFiles, StarFile* pFile );
    void            InsertTokenLine (const ByteString& rToken, Prj** ppPrj, const ByteString& rProjectName, const sal_Bool bExtendAlias = sal_True);

public:
                    Star();
                    Star( String aFileName, sal_uInt16 nMode = STAR_MODE_SINGLE_PARSE );
                    Star( SolarFileList *pSolarFiles );
                    Star( GenericInformationList *pStandLst, ByteString &rVersion );

                    ~Star();

    void            SetDBNotFoundHdl( const Link &rLink ) { aDBNotFoundHdl = rLink; }
    void            SetFileIOErrorHdl( const Link &rLink ) { aFileIOErrorHdl = rLink; }

    ByteString      GetName(){ return aStarName; }; // dummy function of VG
    void            Read( String &rFileName );
    void            Read( SolarFileList *pSOlarFiles );
//  void            ReadXmlBuildList(const ByteString& sBuildLstPath);


    sal_Bool            HasProject( ByteString aProjectName );
    Prj*            GetPrj( ByteString aProjectName );
    ByteString          GetPrjName( DirEntry &rPath );
    sal_Bool            RemovePrj ( Prj* pPrj );
    void            RemoveAllPrj ();

    StarFile*       ReadBuildlist (const String& rFilename, sal_Bool bReadComments = sal_False, sal_Bool bExtendAlias = sal_True);
    sal_Bool            NeedsUpdate();
    SolarFileList*  NeedsFilesForUpdate();
    void            ReplaceFileEntry( StarFileList *pStarFiles, StarFile* pFile );
    void            UpdateFileList( GenericInformationList *pStandLst, ByteString &rVersion, sal_Bool bRead = sal_False );
    void            FullReload( GenericInformationList *pStandLst, ByteString &rVersion, sal_Bool bRead = sal_False );
    void            GenerateFileLoadList( SolarFileList *pSolarFiles );
    sal_Bool            CheckFileLoadList(SolarFileList *pSolarFiles);

    sal_uInt16          GetMode() { return nStarMode; }
    String          GetFileName(){ return sFileName; };
    String          GetSourceRoot(){ return sSourceRoot; };
    SByteStringList* GetAvailableDeps ();
    void            ClearAvailableDeps ();
    SByteStringList* GetCurrentDeps () {return pDepMode;}
    void            SetCurrentDeps (SByteStringList* pDepList);
    void            ClearCurrentDeps ();

    int             GetOSType ( ByteString& aWhatOS );
    int             GetJobType ( ByteString& JobType );

    void            ClearLoadedFilesList ();

    void            PutPrjIntoStream (SByteStringList* pPrjNameList, SvStream* pStream);
    Star&           operator<<  ( SvStream& rStream );
    Star&           operator>>  ( SvStream& rStream );
};

class StarWriter : public Star
{
private:
    sal_uInt16          WritePrj( Prj *pPrj, SvFileStream& rStream );

public:
                    StarWriter( String aFileName, sal_Bool bReadComments = sal_False, sal_uInt16 nMode = STAR_MODE_SINGLE_PARSE );
                    StarWriter( SolarFileList *pSolarFiles, sal_Bool bReadComments = sal_False );
                    StarWriter( GenericInformationList *pStandLst, ByteString &rVersion, ByteString &rMinor,
                        sal_Bool bReadComments = sal_False );

    void            CleanUp();

    sal_Bool            InsertProject ( Prj* pNewPrj );
    Prj*            RemoveProject ( ByteString aProjectName );

    sal_uInt16          Read( String aFileName, sal_Bool bReadComments = sal_False, sal_uInt16 nMode = STAR_MODE_SINGLE_PARSE  );
       sal_uInt16           Read( SolarFileList *pSolarFiles, sal_Bool bReadComments = sal_False );
    sal_uInt16          Write( String aFileName );
    sal_uInt16          WriteMultiple( String rSourceRoot );

    void            InsertTokenLine ( const ByteString& rTokenLine );
};

#endif


