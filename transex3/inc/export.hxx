/*************************************************************************
 *
 *  $RCSfile: export.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:10:43 $
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

#define COMMENT                 0
#define COMMENT_ISO             "x-comment"
#define COMMENT_INDEX               0

#define ENGLISH_US              1
#define ENGLISH_US_ISO          "en-US"
#define ENGLISH_US_PROPERTY     "en_US"
#define ENGLISH_US_INDEX            1

#define PORTUGUESE              3
#define PORTUGUESE_ISO          "pt"
#define PORTUGUESE_INDEX            2

#define GERMAN_DE               4
#define GERMAN_DE_ISO           "x-german"
#define GERMAN_DE_INDEX             3

#define RUSSIAN                 7
#define RUSSIAN_ISO             "ru"
#define RUSSIAN_INDEX               4

#define GREEK                   30
#define GREEK_ISO               "el"
#define GREEK_INDEX                 5

#define DUTCH                   31
#define DUTCH_ISO               "nl"
#define DUTCH_INDEX                 6

#define FRENCH                  33
#define FRENCH_ISO              "fr"
#define FRENCH_INDEX                7

#define SPANISH                 34
#define SPANISH_ISO             "es"
#define SPANISH_INDEX               8

#define FINNISH                 35
#define FINNISH_ISO             "fi"
#define FINNISH_INDEX               9

#define HUNGARIAN               36
#define HUNGARIAN_ISO           "hu"
#define HUNGARIAN_INDEX             10

#define ITALIAN                 39
#define ITALIAN_ISO             "it"
#define ITALIAN_INDEX               11

#define CZECH                   42
#define CZECH_ISO               "cs"
#define CZECH_INDEX                 12

#define SLOVAK                  43
#define SLOVAK_ISO              "sk"
#define SLOVAK_INDEX                13

#define ENGLISH                 44
#define ENGLISH_ISO             "x-translate"
#define ENGLISH_INDEX               14

#define DANISH                  45
#define DANISH_ISO              "da"
#define DANISH_INDEX                15

#define SWEDISH                 46
#define SWEDISH_ISO             "sv"
#define SWEDISH_INDEX               16

#define NORWEGIAN               47
#define NORWEGIAN_ISO           "no"
#define NORWEGIAN_INDEX             17

#define POLISH                  48
#define POLISH_ISO              "pl"
#define POLISH_INDEX                18

#define GERMAN                  49
#define GERMAN_ISO              "de"
#define GERMAN_INDEX                19

#define PORTUGUESE_BRAZILIAN    55
#define PORTUGUESE_BRAZILIAN_ISO "pt-BR"
#define PORTUGUESE_BRAZILIAN_PROPERTY "pt_BR"
#define PORTUGUESE_BRAZILIAN_INDEX  20

#define JAPANESE                81
#define JAPANESE_ISO            "ja"
#define JAPANESE_INDEX              21

#define KOREAN                  82
#define KOREAN_ISO              "ko"
#define KOREAN_INDEX                22

#define CHINESE_SIMPLIFIED      86
#define CHINESE_SIMPLIFIED_ISO  "zh-CN"
#define CHINESE_SIMPLIFIED_PROPERTY "zh_CN"
#define CHINESE_SIMPLIFIED_INDEX    23

#define CHINESE_TRADITIONAL     88
#define CHINESE_TRADITIONAL_ISO "zh-TW"
#define CHINESE_TRADITIONAL_PROPERTY    "zh_TW"
#define CHINESE_TRADITIONAL_INDEX   24

#define TURKISH                 90
#define TURKISH_ISO             "tr"
#define TURKISH_INDEX               25

#define ARABIC                  96
#define ARABIC_ISO              "ar"
#define ARABIC_INDEX                26

#define HEBREW                  97
#define HEBREW_ISO              "he"
#define HEBREW_INDEX                27

#define CATALAN                 37
#define CATALAN_ISO             "ca"
#define CATALAN_INDEX               28

#define THAI                    66
#define THAI_ISO                "th"
#define THAI_INDEX                  29

#define HINDI                   91
#define HINDI_ISO               "hi-IN"
#define HINDI_INDEX                 30

#define ESTONIAN                    77
#define ESTONIAN_ISO                "et"
#define ESTONIAN_INDEX                  31

// special language for l10n framework
#define EXTERN                  99
#define EXTERN_INDEX                32

#define LANGUAGES                   33

#define LANGUAGE_ALLOWED( n )  (( n != 0xFFFF ) && ( Export::LanguageAllowed( Export::LangId[ n ] )) &&                                         \
                               (( Export::LangId[ n ] == 01 ) || ( Export::LangId[ n ] == 03 ) || ( Export::LangId[ n ] == 07 ) ||  \
                                ( Export::LangId[ n ] == 30 ) || ( Export::LangId[ n ] == 31 ) || ( Export::LangId[ n ] == 33 ) ||  \
                                ( Export::LangId[ n ] == 34 ) || ( Export::LangId[ n ] == 35 ) || ( Export::LangId[ n ] == 37 ) ||  \
                                ( Export::LangId[ n ] == 39 ) || ( Export::LangId[ n ] == 45 ) || ( Export::LangId[ n ] == 46 ) ||  \
                                ( Export::LangId[ n ] == 48 ) || ( Export::LangId[ n ] == 49 ) || ( Export::LangId[ n ] == 55 ) ||  \
                                ( Export::LangId[ n ] == 81 ) || ( Export::LangId[ n ] == 82 ) || ( Export::LangId[ n ] == 86 ) ||  \
                                ( Export::LangId[ n ] == 88 ) || ( Export::LangId[ n ] == 90 ) || ( Export::LangId[ n ] == 96 ) ||  \
                                ( Export::LangId[ n ] == 42 ) || ( Export::LangId[ n ] == 43 ) || ( Export::LangId[ n ] == 97 ) ||  \
                                ( Export::LangId[ n ] == 66 ) || ( Export::LangId[ n ] == 91 ) || ( Export::LangId[ n ] == 00 ) ||  \
                                ( Export::LangId[ n ] == 99 ) || ( Export::LangId[ n ] == 77 )))

#define GERMAN_LIST_LINE_INDEX  LANGUAGES
#define LIST_REFID              (GERMAN_LIST_LINE_INDEX+1)
typedef ByteString ExportListEntry[ LANGUAGES + 2 ];
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

    ByteString sText[ LANGUAGES ];
    USHORT nTextRefId;

    ByteString sHelpText[ LANGUAGES ];
    USHORT nHelpTextRefId;

    ByteString sQuickHelpText[ LANGUAGES ];
    USHORT nQuickHelpTextRefId;

    ByteString sTitle[ LANGUAGES ];
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
    USHORT nListLang;
    ULONG nListIndex;
    ULONG nListLevel;

    ByteString sProject;
    ByteString sRoot;

    BOOL bEnableExport;
    BOOL bMergeMode;
    ByteString sMergeSrc;
    ByteString sLastListLine;

    BOOL bError;                        // any errors while export?
    BOOL bReadOver;
    BOOL bDontWriteOutput;

    ByteString sLastTextTyp;

public:
    static ByteString sLanguages;
    static ByteString sIsoCode99;
    static USHORT LangId[ LANGUAGES ];  // table to handle country codes
    static const ByteString LangName[ LANGUAGES ];
    static USHORT GetLangIndex( const ByteString &rLang );  // string to LangId-Index
    static USHORT GetLangIndex( USHORT nLangId );
    static CharSet GetCharSet( USHORT nLangId );
    static USHORT GetLangByIsoLang( const ByteString &rIsoLang );
    static ByteString GetIsoLangByIndex( USHORT nIndex );
    static void QuotHTML( ByteString &rString );
    static void UnquotHTML( ByteString &rString );
    static BOOL LanguageAllowed( USHORT nLanguage );
    static USHORT GetFallbackLanguage( USHORT nLanguage );
    static void FillInFallbacks( ResData *pResData );
    static void FillInListFallbacks( ExportList *pList, USHORT nSource, USHORT nFallback );
    static ByteString GetTimeStamp();
    static BOOL ConvertLineEnds( ByteString sSource, ByteString sDestination );
    static ByteString GetNativeFile( ByteString sSource );
    static DirEntry GetTempFile();

private:

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
                USHORT nLangIndex, ResData *pResData );
    void MergeRest( ResData *pResData, USHORT nMode = MERGE_MODE_NORMAL );
    void ConvertMergeContent( ByteString &rText, USHORT nTyp );

      void WriteToMerged( const ByteString &rText );
    void SetChildWithText();

    void CutComment( ByteString &rText );

public:
    Export( const ByteString &rOutput, BOOL bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot );
    Export( const ByteString &rOutput, BOOL bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot,
            const ByteString &rMergeSource );
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
    ByteString sText[ LANGUAGES ];
    BOOL bTextFirst[ LANGUAGES ];
    ByteString sQuickHelpText[ LANGUAGES ];
    BOOL bQuickHelpTextFirst[ LANGUAGES ];
    ByteString sTitle[ LANGUAGES ];
    BOOL bTitleFirst[ LANGUAGES ];
public:
    PFormEntrys( const ByteString &rPForm ) : ByteString( rPForm ) {};
    void InsertEntry( USHORT nId, const ByteString &rText,
                    const ByteString &rQuickHelpText,
                    const ByteString &rTitle )
        {
            sText[ nId ] = rText;
            bTextFirst[ nId ] = TRUE;
            sQuickHelpText[ nId ] = rQuickHelpText;
            bQuickHelpTextFirst[ nId ] = TRUE;
            sTitle[ nId ] = rTitle;
            bTitleFirst[ nId ] = TRUE;
        }
    BOOL GetText( ByteString &rReturn, USHORT nTyp, USHORT nLangIndex, BOOL bDel = FALSE );
};

//
// class MergeData
//

/******************************************************************************
* Purpose: holds information of data to merge (one ressource)
******************************************************************************/

class MergeDataFile;
DECLARE_LIST( MergeStrings, PFormEntrys * );
class MergeData : public MergeStrings
{
friend class MergeDataFile;
private:
    ByteString sTyp;
    ByteString sGID;
    ByteString sLID;

public:
    MergeData( const ByteString &rTyp, const ByteString &rGID, const ByteString &rLID )
            : sTyp( rTyp ), sGID( rGID ), sLID( rLID ) {};
    ~MergeData();
    PFormEntrys *InsertEntry( const ByteString &rPForm );
    PFormEntrys *GetPFormEntrys( ResData *pResData );

    BOOL operator==( ResData *pData );
};

//
// class MergeDataFile
//

/******************************************************************************
* Purpose: holds information of data to merge
******************************************************************************/

DECLARE_LIST( MergeDataList, MergeData * )
class MergeDataFile  : public MergeDataList
{
private:
    BOOL bErrorLog;
    ByteString sErrorLog;
    SvFileStream aErrLog;
public:
    MergeDataFile( const ByteString &rFileName, BOOL bErrLog, CharSet aCharSet, BOOL bUTF8 );
    ~MergeDataFile();

    MergeData *GetMergeData( ResData *pResData );
    PFormEntrys *GetPFormEntrys( ResData *pResData );
    void InsertEntry( const ByteString &rTYP, const ByteString &rGID, const ByteString &rLID,
                const ByteString &rPFO, USHORT nLANG, const ByteString &rTEXT,
                const ByteString &rQHTEXT, const ByteString &rTITLE );
    static USHORT GetLangIndex( USHORT nId );

    void WriteErrorLog( const ByteString &rFileName );
    void WriteError( const ByteString &rLine );
};

#endif
