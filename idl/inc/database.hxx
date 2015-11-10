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

#ifndef INCLUDED_IDL_INC_DATABASE_HXX
#define INCLUDED_IDL_INC_DATABASE_HXX

#include <module.hxx>
#include <hash.hxx>
#include <lex.hxx>
#include <rtl/string.hxx>
#include <tools/pstm.hxx>

#include <rtl/ustring.hxx>
#include <set>

class SvCommand;

class SvIdlError
{
    OString aText;
public:
    sal_uInt32  nLine, nColumn;

            SvIdlError() : nLine(0), nColumn(0) {}
            SvIdlError( sal_uInt32 nL, sal_uInt32 nC )
                : nLine(nL), nColumn(nC) {}

    const OString&  GetText() const { return aText; }
    void SetText( const OString& rT ) { aText = rT; }
    bool            IsError() const { return nLine != 0; }
    SvIdlError &    operator = ( const SvIdlError & rRef )
    { aText   = rRef.aText;
      nLine   = rRef.nLine;
      nColumn = rRef.nColumn;
      return *this;
    }
};

class SvIdlDataBase
{
    bool                        bExport;
    OUString                    aExportFile;
    sal_uInt32                  nUniqueId;
    sal_uInt32                  nVerbosity;
    StringList                  aIdFileList;
    SvStringHashTable *         pIdTable;

    SvMetaTypeMemberList        aTypeList;
    SvMetaClassMemberList       aClassList;
    SvMetaModuleMemberList      aModuleList;
    SvMetaAttributeMemberList   aAttrList;
    SvMetaTypeMemberList        aTmpTypeList; // not persistent

protected:
    ::std::set< OUString > m_DepFiles;
    SvMetaObjectMemberStack     aContextStack;
    OUString                    aPath;
    SvIdlError                  aError;
    void WriteReset()
    {
        aUsedTypes.clear();
        aIFaceName.clear();
    }
public:
    OUString sSlotMapFile;

                explicit SvIdlDataBase( const SvCommand& rCmd );
                ~SvIdlDataBase();

    SvMetaAttributeMemberList&  GetAttrList() { return aAttrList; }
    SvMetaTypeMemberList &      GetTypeList();
    SvMetaClassMemberList &     GetClassList()  { return aClassList; }
    SvMetaModuleMemberList &    GetModuleList() { return aModuleList; }
    SvMetaModule *              GetModule( const OString& rName );

    // list of used types while writing
    SvMetaTypeMemberList    aUsedTypes;
    OString                 aIFaceName;
    SvIdentifier            aStructSlotId;

    void                    StartNewFile( const OUString& rName );
    void                    SetExportFile( const OUString& rName )
                            { aExportFile = rName; }
    void                    AppendAttr( SvMetaAttribute *pSlot );
    const SvIdlError &      GetError() const { return aError; }
    void                    SetError( const SvIdlError & r )
                            { aError = r; }

    const OUString &        GetPath() const { return aPath; }
    SvMetaObjectMemberStack & GetStack()      { return aContextStack; }

    void                    Write(const OString& rText);
    static void             WriteError(const OString& rErrWrn,
                                    const OString& rFileName,
                                    const OString& rErrorText,
                                    sal_uLong nRow = 0, sal_uLong nColumn = 0 );
    void                    WriteError( SvTokenStream & rInStm );
    void                    SetError( const OString& rError, SvToken& rTok );
    void                    Push( SvMetaObject * pObj );
    sal_uInt32              GetUniqueId() { return ++nUniqueId; }
    bool                    FindId( const OString& rIdName, sal_uLong * pVal );
    bool                    InsertId( const OString& rIdName, sal_uLong nVal );
    bool                    ReadIdFile( const OUString & rFileName );

    SvMetaType *            FindType( const OString& rName );
    static SvMetaType *     FindType( const SvMetaType *, SvMetaTypeMemberList & );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       ReadKnownAttr( SvTokenStream & rInStm,
                                            SvMetaType * pType = nullptr );
    SvMetaAttribute *       SearchKnownAttr( const SvIdentifier& );
    SvMetaClass *           ReadKnownClass( SvTokenStream & rInStm );
    void AddDepFile(OUString const& rFileName);
    bool WriteDepFile(SvFileStream & rStream, OUString const& rTarget);
};

class SvIdlWorkingBase : public SvIdlDataBase
{
public:
                explicit SvIdlWorkingBase( const SvCommand& rCmd );

    bool        ReadSvIdl( SvTokenStream &, bool bImported, const OUString & rPath );

    bool        WriteSfx( SvStream & );
};

#endif // INCLUDED_IDL_INC_DATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
