/*************************************************************************
 *
 *  $RCSfile: export.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 12:39:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXPORT_HXX
#define _EXPORT_HXX

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <tools/isolang.hxx>

#include <hash_map> /* std::hashmap*/
#include <iterator> /* std::iterator*/
#include <set>      /* std::set*/
#include <vector>   /* std::vector*/
#include <queue>

#define NO_TRANSLATE_ISO        "x-no-translate"

// Achtung !!! merge.cxx
#define JAPANESE_ISO "ja"


struct eqstr{
  BOOL operator()(const char* s1, const char* s2) const{
    return strcmp(s1,s2)==0;
  }
};

struct equalByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
    }
};
struct lessByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
    }
};

struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
                std::hash< const char* > myHash;
                return myHash( rName.GetBuffer() );
    }
};

class PFormEntrys;
class MergeData;
typedef std::set<ByteString , lessByteString > ByteStringSet;

typedef std::hash_map<ByteString , ByteString , hashByteString,equalByteString>
                                ByteStringHashMap;

typedef std::hash_map<ByteString , bool , hashByteString,equalByteString>
                                ByteStringBoolHashMap;

typedef std::hash_map<ByteString , PFormEntrys* , hashByteString,equalByteString>
                                PFormEntrysHashMap;

typedef std::hash_map<ByteString , MergeData* , hashByteString,equalByteString>
                                MergeDataHashMap;

#define GERMAN_LIST_LINE_INDEX "GERMAN_LIST_LINE_INDEX"
#define LIST_REFID  "LIST_REFID"

typedef ByteStringHashMap ExportListEntry;

DECLARE_LIST( ExportListBase, ExportListEntry * );

//
// class ExportList
//

class ExportList : public ExportListBase
{
private:
    ULONG nGermanEntryCount;

public:
    ExportList() : ExportListBase() { nGermanEntryCount = 0; }
    ULONG GetGermanEntryCount() { return nGermanEntryCount; }
    void NewGermanEntry() { nGermanEntryCount++; }
};

#define REFID_NONE 0xFFFF

//
// struct ResData
//

/******************************************************************************
* Purpose: holds mandatory data to export a single res (used with ResStack)
******************************************************************************/

#define ID_LEVEL_NULL       0x0000
#define ID_LEVEL_AUTOID     0x0001
#define ID_LEVEL_TEXT       0x0002
#define ID_LEVEL_FIELDNAME  0x0003
#define ID_LEVEL_ACCESSPATH 0x0004
#define ID_LEVEL_IDENTIFIER 0x0005
#define ID_LEVEL_LISTINDEX  0x0006

class ResData
{
public:
    ResData( const ByteString &rPF, const ByteString &rGId )
            : nWidth( 0 ),
            sPForm( rPF ),
            pStringList( NULL ),
            pFilterList( NULL ),
            pItemList( NULL ),
            pUIEntries( NULL ),
            nChildIndex( 0 ),
            nIdLevel( ID_LEVEL_NULL ),
            sGId( rGId ),
            sTextTyp( "Text" ),
            bText( FALSE ),
            bList( FALSE ),
            bQuickHelpText( FALSE ),
            bHelpText( FALSE ),
            bTitle( FALSE ),
            bChild( FALSE ),
            bChildWithText( FALSE ),
            nTextRefId( REFID_NONE ),
            nHelpTextRefId( REFID_NONE ),
            nQuickHelpTextRefId( REFID_NONE ),
            nTitleRefId( REFID_NONE ),
            bRestMerged( FALSE )
    {
        sGId.EraseAllChars( '\r' );
        sPForm.EraseAllChars( '\r' );
    };

    ~ResData();
    BOOL SetId( const ByteString &rId, USHORT nLevel );

    USHORT nChildIndex;
    USHORT nIdLevel;
    BOOL bChild;
    BOOL bChildWithText;

    BOOL bText;
    BOOL bHelpText;
    BOOL bQuickHelpText;
    BOOL bTitle;
    BOOL bList;

    BOOL bRestMerged;

    ByteString sPForm;
    ByteString sResTyp;
    ByteString sId;
    ByteString sGId;
    ByteString sHelpId;
    USHORT nWidth;

    ByteStringHashMap sText;
    USHORT nTextRefId;

    ByteStringHashMap sHelpText;
    USHORT nHelpTextRefId;

    ByteStringHashMap sQuickHelpText;
    USHORT nQuickHelpTextRefId;

    ByteStringHashMap sTitle;
    USHORT nTitleRefId;

    ByteString sTextTyp;

    ExportList  *pStringList;
    ExportList  *pUIEntries;
    ExportList  *pFilterList;
    ExportList  *pItemList;
};


//
// class Export
//

/******************************************************************************
* Purpose: syntax check and export of *.src, called from lexer
******************************************************************************/

#define LIST_NON                    0x0000
#define LIST_STRING                 0x0001
#define LIST_FILTER                 0x0002
#define LIST_ITEM                   0x0004
#define LIST_UIENTRIES              0x0008
#define STRING_TYP_TEXT             0x0010
#define STRING_TYP_HELPTEXT         0x0020
#define STRING_TYP_QUICKHELPTEXT    0x0040
#define STRING_TYP_TITLE            0x0080

#define MERGE_MODE_NORMAL           0x0000
#define MERGE_MODE_LIST             0x0001

DECLARE_LIST( ResStack, ResData * );
// forwards
class WordTransformer;
class ParserQueue;

class Export
{
private:
    WordTransformer *pWordTransformer;

    CharSet aCharSet;                   // used charset in src

    SvFileStream aOutput;

    ResStack aResStack;                 // stack for parsing recursive

    ByteString sActPForm;               // hold cur. system

    BOOL bDefine;                       // cur. res. in a define?
    BOOL bNextMustBeDefineEOL;          // define but no \ at lineend
    ULONG nLevel;                       // res. recursiv? how deep?
    USHORT nList;                       // cur. res. is String- or FilterList

    ByteString nListLang;
    ULONG nListIndex;
    ULONG nListLevel;

    bool bSkipFile;

    ByteString sProject;
    ByteString sRoot;
    //ByteString sFile;

    BOOL bEnableExport;
    BOOL bMergeMode;

    ByteString sMergeSrc;
    ByteString sLastListLine;

    BOOL bError;                        // any errors while export?
    BOOL bReadOver;
    BOOL bDontWriteOutput;

    ByteString sLastTextTyp;

    static bool isInitialized;



public:
    ParserQueue* pParseQueue; // public ?

    static ByteString sLanguages;

    static ByteString sIsoCode99;
    static void InitLanguages( bool bMergeMode = false );
    static std::vector<ByteString> GetLanguages();
    static void SetLanguages( std::vector<ByteString> val );
    static USHORT GetLangIndex( USHORT nLangId );
    static CharSet GetCharSet( USHORT nLangId );
    static USHORT GetLangByIsoLang( const ByteString &rIsoLang );
    static ByteString GetIsoLangByIndex( USHORT nIndex );
    static void QuotHTML( ByteString &rString );
    static void UnquotHTML( ByteString &rString );

    static bool LanguageAllowed( const ByteString &nLanguage );
    static void Languages( std::vector<ByteString>::const_iterator& begin , std::vector<ByteString>::const_iterator& end );


    static ByteString GetFallbackLanguage( const ByteString nLanguage );
    static void FillInFallbacks( ResData *pResData );
    static void FillInListFallbacks( ExportList *pList, const ByteString &nSource, const ByteString &nFallback );
    static ByteString GetTimeStamp();
    static BOOL ConvertLineEnds( ByteString sSource, ByteString sDestination );
    static ByteString GetNativeFile( ByteString sSource );
    static DirEntry GetTempFile();

private:
    static std::vector<ByteString> aLanguages;

    BOOL CreateRefIds( ResData *pResData ) { /* Dummy !!! */ return TRUE; }
    BOOL ListExists( ResData *pResData, USHORT nLst );

    BOOL WriteData( ResData *pResData, BOOL bCreateNew = FALSE );// called befor dest. cur ResData
    BOOL WriteExportList( ResData *pResData, ExportList *pExportList,
                        const ByteString &rTyp, BOOL bCreateNew = FALSE );

    ByteString FullId();                    // creates cur. GID

    void InsertListEntry( const ByteString &rText, const ByteString &rLine );
    void CleanValue( ByteString &rValue );
    ByteString GetText( const ByteString &rSource, USHORT nToken );

    BOOL PrepareTextToMerge( ByteString &rText, USHORT nTyp,
        ByteString &nLangIndex, ResData *pResData );

    void MergeRest( ResData *pResData, USHORT nMode = MERGE_MODE_NORMAL );
    void ConvertMergeContent( ByteString &rText, USHORT nTyp );

      void WriteToMerged( const ByteString &rText , bool bSDFContent );
    void SetChildWithText();

    void CutComment( ByteString &rText );

public:
    Export( const ByteString &rOutput, BOOL bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot , const ByteString& rFile );
    Export( const ByteString &rOutput, BOOL bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot,
            const ByteString &rMergeSource , const ByteString& rFile );
    ~Export();

    void Init();
    int Execute( int nToken, char * pToken );   // called from lexer
    void SetError() { bError = TRUE; }
    BOOL GetError() { return bError; }
};

//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//
//      classes used to merge data back into src
//
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

//
// class PFormEntrys
//

/******************************************************************************
* Purpose: holds information of data to merge (one pform)
******************************************************************************/

class PFormEntrys : public ByteString
{
friend class MergeDataFile;
private:
    ByteString sHelpText; // empty string
    ByteStringHashMap sText;
    ByteStringBoolHashMap bTextFirst;
    ByteStringHashMap sQuickHelpText;
    ByteStringBoolHashMap bQuickHelpTextFirst;
    ByteStringHashMap sTitle;
    ByteStringBoolHashMap bTitleFirst;

public:
    PFormEntrys( const ByteString &rPForm ) : ByteString( rPForm ) {};
    void InsertEntry(
                    const ByteString &nId ,
                    const ByteString &rText,
                    const ByteString &rQuickHelpText,
                    const ByteString &rTitle )
        {
            sText[ nId ] = rText;
            bTextFirst[ nId ] = true;
            sQuickHelpText[ nId ] = rQuickHelpText;
            bQuickHelpTextFirst[ nId ] = true;
            sTitle[ nId ] = rTitle;
            bTitleFirst[ nId ] = true;
        }
     BOOL GetText( ByteString &rReturn, USHORT nTyp, const ByteString &nLangIndex, BOOL bDel = FALSE );
};

//
// class MergeData
//

/******************************************************************************
* Purpose: holds information of data to merge (one ressource)
******************************************************************************/

class MergeDataFile;
//DECLARE_LIST( MergeStrings, PFormEntrys * );
class MergeData //: public MergeStrings
{
friend class MergeDataFile;
private:
    ByteString sTyp;
    ByteString sGID;
    ByteString sLID;
    PFormEntrysHashMap aMap;
public:
    MergeData( const ByteString &rTyp, const ByteString &rGID, const ByteString &rLID )
            : sTyp( rTyp ), sGID( rGID ), sLID( rLID ) {};
    ~MergeData();
    PFormEntrys* InsertEntry( const ByteString &rPForm );
    PFormEntrys* GetPFormEntrys( ResData *pResData );

    void Insert( const ByteString& rPFO , PFormEntrys* pfEntrys );
    PFormEntrys* GetPFObject( const ByteString& rPFO );

    BOOL operator==( ResData *pData );
};

//
// class MergeDataFile
//

/******************************************************************************
* Purpose: holds information of data to merge
******************************************************************************/

class MergeDataFile
{
private:
    BOOL bErrorLog;
    ByteString sErrorLog;
    SvFileStream aErrLog;
    ByteStringSet aLanguageSet;
    MergeDataHashMap aMap;
    std::vector<ByteString> aLanguages;
    inline ByteString CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID );

public:
    MergeDataFile( const ByteString &rFileName, const ByteString& rFile , BOOL bErrLog, CharSet aCharSet, BOOL bUTF8 );
    ~MergeDataFile();

    std::vector<ByteString> GetLanguages();
    MergeData *GetMergeData( ResData *pResData );

    PFormEntrys *GetPFormEntrys( ResData *pResData );
    void InsertEntry( const ByteString &rTYP, const ByteString &rGID, const ByteString &rLID,
                const ByteString &rPFO, //USHORT nLANG
                const ByteString &nLang , const ByteString &rTEXT,
                const ByteString &rQHTEXT, const ByteString &rTITLE );
    static USHORT GetLangIndex( USHORT nId );


    void WriteErrorLog( const ByteString &rFileName );
    void WriteError( const ByteString &rLine );
};


class QueueEntry
{
public:
    QueueEntry( int nTypVal , ByteString sLineVal ): nTyp( nTypVal ) , sLine( sLineVal ){};
    int nTyp;
    ByteString sLine;
};

class ParserQueue
{
public:

    ParserQueue( Export& aExportObj );
    ~ParserQueue();

    inline void Push( const QueueEntry& aEntry );
    bool bCurrentIsM;  // public ?
    bool bNextIsM;   // public ?
    bool bLastWasM;   // public ?
    bool bMflag;   // public ?

    void Close();
private:
    // Future / Next
    std::queue<QueueEntry>* aQueueNext;
    // Current
    std::queue<QueueEntry>* aQueueCur;
    // Ref
    std::queue<QueueEntry>* aQref;

    Export& aExport;
    bool bStart;
    bool bStartNext;

    inline void Pop( std::queue<QueueEntry>& aQueue );

};
#endif
