/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_RSC_INC_RSCDB_HXX
#define INCLUDED_RSC_INC_RSCDB_HXX

#include <rscall.h>
#include <rsckey.hxx>
#include <rscconst.hxx>
#include <rscflag.hxx>
#include <rscrange.hxx>
#include <rscstr.hxx>
#include <rscarray.hxx>
#include <rscdef.hxx>

#include <vector>
#include <map>

class RscError;
class RscTupel;
class RscCont;
class RscCmdLine;
enum class MenuItemBits : sal_Int16;
enum class MapUnit;

struct WriteRcContext
{
    FILE *              fOutput;
    OString        aOutputRc;
    OString        aOutputSysList;
    RscCmdLine*         pCmdLine;
};

// table for system dependent resources
struct RscSysEntry
{
    sal_uInt32      nKey;
    sal_uInt32      nRscTyp;
    OString    aFileName;
    sal_uInt32      nTyp;
    sal_uInt32      nRefId;
};

class RscTypCont
{
    rtl_TextEncoding    nSourceCharSet;
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel oder
    OString             aLanguage;          // output language
    std::vector< sal_uInt32 > aLangFallbacks;   // language fallback list (entry 0 is language itself)
    OString             aSearchPath;        // search path for bitmap, icon and pointer
    OString             aSysSearchPath;     // aSearchPath plus language specific paths
    sal_uInt32          nUniqueId;          // unique id for system resources
    sal_uLong           nFilePos;           // position in file (MTF)
    sal_uInt32          nPMId;              // unique id for PR-resource file
                                            // must be greater that RSC_VERSIONCONTROL_ID
    RscTop  *           pRoot;              // pointer to the root of type tree
    RSCINST             aVersion;           // version control instance

    ::std::vector< RscTop* >
                        aBaseLst;           // list of simple resource class
    ::std::vector< RscSysEntry* >
                        aSysLst;            // list of system resources

    void        Init();         // initializes classes and tables
    void        SETCONST( RscConst *, const char *, sal_uInt32 );
    void        SETCONST( RscConst *, Atom, sal_uInt32 );
    void SETCONST( RscConst *p1, Atom p2, MenuItemBits p3 ) { SETCONST(p1, p2, static_cast<sal_uInt32>(p3)); }
    RscEnum *   InitFieldUnitsType();
    RscTupel *  InitStringLongTupel();
    static RscCont  *  InitStringLongTupelList( RscTupel * pStringLongTupel );
    RscArray *  InitLangStringLongTupelList( RscCont * pStrLongTupelLst );

    RscTop *    InitClassMgr();
    RscTop *    InitClassString( RscTop * pSuper );
    RscTop *    InitClassBitmap( RscTop * pSuper );
    RscTop *    InitClassMenuItem( RscTop * pSuper );
    RscTop *    InitClassMenu( RscTop * pSuper, RscTop * pMenuItem );

public:
    RscBool             aBool;
    RscRange            aShort;
    RscRange            aUShort;
    RscLongEnumRange    aEnumLong;
    RscIdRange          aIdNoZeroUShort;
    RscString           aString;
    RscString           aStringLiteral;
    RscLangEnum         aLangType;
    RscLangArray        aLangString;

    RscError*           pEH;        // error handler
    RscNameTable        aNmTb;      // name table
    RscFileTab          aFileTab;   // file name table
    CommandFlags        nFlags;
    std::map<sal_uInt64, sal_uLong> aIdTranslator; // map resources types and ids to an id (under PM9 or to a file position (MTF)

    RscTypCont( RscError *, RSCBYTEORDER_TYPE, const OString& rSearchPath, CommandFlags nFlags );
    ~RscTypCont();

    Atom AddLanguage( const char* );
    bool              IsSrsDefault() const
                          { return bool(nFlags & CommandFlags::SrsDefault); }
    OString           ChangeLanguage(const OString & rNewLang);
    const std::vector< sal_uInt32 >& GetFallbacks() const
                          { return aLangFallbacks; }

    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    rtl_TextEncoding  GetSourceCharSet() const { return nSourceCharSet; }
    void              SetSourceCharSet( rtl_TextEncoding aCharSet )
                          {
                              nSourceCharSet = aCharSet;
                          }
    const OString&    GetSearchPath() const { return aSearchPath; }
    void              SetSysSearchPath( const OString& rStr ) { aSysSearchPath = rStr; }
                      // deletes all resource objects of this file
    void              Delete( RscFileTab::Index lFileKey );
    sal_uInt32        PutSysName( sal_uInt32 nRscTyp, char * pName );
    void              ClearSysNames();
    ERRTYPE           WriteRc( WriteRcContext& rContext );
    void              WriteSrc( FILE * fOutput, RscFileTab::Index nFileIndex );
    void              PutTranslatorKey( sal_uInt64 nKey );
    void              IncFilePos( sal_uLong nOffset ){ nFilePos += nOffset; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
