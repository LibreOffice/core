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
    FILE *         fOutput;
    OString        aOutputRc;
    OString        aOutputSysList;
    RscCmdLine*    pCmdLine;
};

class RscTypCont
{
    rtl_TextEncoding    nSourceCharSet;
    RSCBYTEORDER_TYPE   nByteOrder;         // Intel or
    OString             aLanguage;          // output language
    std::vector< sal_uInt32 > aLangFallbacks;   // language fallback list (entry 0 is language itself)
    sal_uLong           nFilePos;           // position in file (MTF)
    sal_uInt32          nPMId;              // unique id for PR-resource file
                                            // must be greater that RSC_VERSIONCONTROL_ID
    RscTop  *           pRoot;              // pointer to the root of type tree
    RSCINST             aVersion;           // version control instance

    ::std::vector< RscTop* >
                        aBaseLst;           // list of simple resource class

    void        Init();         // initializes classes and tables
    void        SETCONST( RscEnum *, const char *, sal_uInt32 );
    RscEnum *   InitFieldUnitsType();
    RscTupel *  InitStringLongTupel();
    static RscCont  *  InitStringLongTupelList( RscTupel * pStringLongTupel );
    RscArray *  InitLangStringLongTupelList( RscCont * pStrLongTupelLst );

    RscTop *    InitClassMgr();
    RscTop *    InitClassString( RscTop * pSuper );

public:
    RscLongEnumRange    aEnumLong;
    RscString           aString;
    RscLangEnum         aLangType;
    RscLangArray        aLangString;

    RscError*           pEH;        // error handler
    RscNameTable        aNmTb;      // name table
    RscFileTab          aFileTab;   // file name table
    CommandFlags        nFlags;
    std::map<sal_uInt64, sal_uLong> aIdTranslator; // map resources types and ids to an id (under PM9 or to a file position (MTF)

    RscTypCont( RscError *, RSCBYTEORDER_TYPE, CommandFlags nFlags );
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
                      // deletes all resource objects of this file
    void              Delete( RscFileTab::Index lFileKey );
    ERRTYPE           WriteRc( WriteRcContext& rContext );
    void              WriteSrc( FILE * fOutput, RscFileTab::Index nFileIndex );
    void              PutTranslatorKey( sal_uInt64 nKey );
    void              IncFilePos( sal_uLong nOffset ){ nFilePos += nOffset; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
