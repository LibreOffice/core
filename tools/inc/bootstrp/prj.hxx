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
    ByteString          GetCleanedNextLine( BOOL bReadComments = FALSE );
};

<<<<<<< local
=======
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

/*********************************************************************
*
*   class Prj
*   alle Daten eines Projektes werden hier gehalten
*
*********************************************************************/

DECL_DEST_LIST ( PrjList_tmp, PrjList, CommandData * )

class Star;
class Prj : public PrjList
{
friend class Star;
private:
    BOOL            bVisited;

    ByteString          aPrjPath;
    ByteString          aProjectName;
    ByteString          aProjectPrefix;     // max. 2-buchstabige Abk.
    SByteStringList*    pPrjInitialDepList;
    SByteStringList*    pPrjDepList;
    BOOL            bHardDependencies;
    BOOL            bSorted;

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
    BOOL            InsertDirectory( ByteString aDirName , USHORT aWhat,
                                    USHORT aWhatOS, ByteString aLogFileName,
                                    const ByteString &rClientRestriction );
    CommandData*    RemoveDirectory( ByteString aLogFileName );
    CommandData*    GetDirectoryList ( USHORT nWhatOs, USHORT nCommand );
    CommandData*    GetDirectoryData( ByteString aLogFileName );
    inline CommandData* GetData( ByteString aLogFileName )
                            { return GetDirectoryData( aLogFileName ); };

    SByteStringList*    GetDependencies( BOOL bExpanded = TRUE );
    void            AddDependencies( ByteString aStr );
    void            HasHardDependencies( BOOL bHard ) { bHardDependencies = bHard; }
    BOOL            HasHardDependencies() { return bHardDependencies; }
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

    BOOL bExists;

public:
    StarFile( const String &rFile );
    const String &GetName() { return aFileName; }
    Date GetDate() { return aDate; }
    Time GetTime() { return aTime; }

    BOOL NeedsUpdate();
    BOOL Exists() { return bExists; }
};

DECLARE_LIST( StarFileList, StarFile * )

#define STAR_MODE_SINGLE_PARSE          0x0000
#define STAR_MODE_RECURSIVE_PARSE       0x0001
#define STAR_MODE_MULTIPLE_PARSE        0x0002

class Star : public StarList
{
private:
    ByteString      aStarName;

    static Link aDBNotFoundHdl;
protected:
    vos:: OMutex  aMutex;

    USHORT          nStarMode;
    SolarFileList   aFileList;
    StarFileList    aLoadedFilesList;
    String          sSourceRoot;

    void InsertSolarList( String sProject );
    String CreateFileName( String sProject );

    void            Expand_Impl();
    void            ExpandPrj_Impl( Prj *pPrj, Prj *pDepPrj );

private:
    void            Read( String &rFileName );
    void            Read( SolarFileList *pSOlarFiles );

public:
                    Star();
                    Star( String aFileName, USHORT nMode = STAR_MODE_SINGLE_PARSE );
                    Star( SolarFileList *pSolarFiles );
                    Star( GenericInformationList *pStandLst, ByteString &rVersion, BOOL bLocal = FALSE,
                        const char *pSourceRoot = NULL  );

                    ~Star();

    static          void SetDBNotFoundHdl( const Link &rLink ) { aDBNotFoundHdl = rLink; }

    ByteString      GetName(){ return aStarName; };

    BOOL            HasProject( ByteString aProjectName );
    Prj*            GetPrj( ByteString aProjectName );
    ByteString          GetPrjName( DirEntry &rPath );

    void            InsertToken( char *pChar );
    BOOL            NeedsUpdate();

    USHORT          GetMode() { return nStarMode; }
};

class StarWriter : public Star
{
private:
    USHORT          WritePrj( Prj *pPrj, SvFileStream& rStream );

public:
                    StarWriter( String aFileName, BOOL bReadComments = FALSE, USHORT nMode = STAR_MODE_SINGLE_PARSE );
                    StarWriter( SolarFileList *pSolarFiles, BOOL bReadComments = FALSE );
                    StarWriter( GenericInformationList *pStandLst, ByteString &rVersion, BOOL bLocal = FALSE,
                        const char *pSourceRoot = NULL );

    void            CleanUp();

    BOOL            InsertProject ( Prj* pNewPrj );
    Prj*            RemoveProject ( ByteString aProjectName );

    USHORT          Read( String aFileName, BOOL bReadComments = FALSE, USHORT nMode = STAR_MODE_SINGLE_PARSE  );
       USHORT           Read( SolarFileList *pSolarFiles, BOOL bReadComments = FALSE );
    USHORT          Write( String aFileName );
    USHORT          WriteMultiple( String rSourceRoot );

    void            InsertTokenLine( ByteString& rString );
};

>>>>>>> other
#endif
