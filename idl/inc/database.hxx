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

#include "module.hxx"
#include "hash.hxx"
#include "lex.hxx"
#include <rtl/string.hxx>

#include <rtl/ustring.hxx>
#include <set>
#include <exception>

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
    void            SetText( const OString& rT ) { aText = rT; }
    bool            IsError() const { return nLine != 0; }
};

class SvParseException : public std::exception
{
public:
    SvIdlError aError;
    SvParseException( SvTokenStream const & rInStm, const OString& rError );
    SvParseException( const OString& rError, SvToken const & rTok );
};



class SvIdlDataBase
{
    bool                        bExport;
    OUString                    aExportFile;
    sal_uInt32                  nUniqueId;
    sal_uInt32                  nVerbosity;
    std::vector<OUString>       aIdFileList;
    std::unique_ptr<SvStringHashTable> pIdTable;

    SvRefMemberList<SvMetaType *>      aTypeList;
    SvRefMemberList<SvMetaClass *>     aClassList;
    SvRefMemberList<SvMetaModule *>    aModuleList;
    SvRefMemberList<SvMetaSlot *>      aSlotList;
    SvRefMemberList<SvMetaObject *>    aContextStack;

protected:
    ::std::set< OUString >      m_DepFiles;
    OUString                    aPath;
    SvIdlError                  aError;
    void WriteReset()
    {
        aUsedTypes.clear();
    }
public:
    OUString sSlotMapFile;

                explicit SvIdlDataBase( const SvCommand& rCmd );
                ~SvIdlDataBase();

    SvRefMemberList<SvMetaType *>&     GetTypeList();
    SvRefMemberList<SvMetaClass *>&    GetClassList()  { return aClassList; }
    SvRefMemberList<SvMetaModule *>&   GetModuleList() { return aModuleList; }

    // list of used types while writing
    SvRefMemberList<SvMetaType *>    aUsedTypes;

    void                    StartNewFile( const OUString& rName );
    void                    SetExportFile( const OUString& rName )
                            { aExportFile = rName; }
    void                    AppendSlot( SvMetaSlot *pSlot );
    const SvIdlError &      GetError() const { return aError; }
    void                    SetError( const SvIdlError & r )
                            { aError = r; }

    const OUString &        GetPath() const { return aPath; }
    void                    SetPath(const OUString &s) { aPath = s; }
    SvRefMemberList<SvMetaObject *>& GetStack() { return aContextStack; }

    void                    Write(const OString& rText) const;
    void                    WriteError( SvTokenStream & rInStm );
    void                    SetError( const OString& rError, SvToken const & rTok );
    void                    SetAndWriteError( SvTokenStream & rInStm, const OString& rError );
    void                    Push( SvMetaObject * pObj );
    sal_uInt32              GetUniqueId() { return ++nUniqueId; }
    bool                    FindId( const OString& rIdName, sal_uLong * pVal );
    void                    InsertId( const OString& rIdName, sal_uLong nVal );
    bool                    ReadIdFile( const OString& rFileName );

    SvMetaType *            FindType( const OString& rName );
    static SvMetaType *     FindType( const SvMetaType *, SvRefMemberList<SvMetaType *>& );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       ReadKnownAttr( SvTokenStream & rInStm,
                                            SvMetaType * pType );
    SvMetaAttribute *       FindKnownAttr( const SvIdentifier& );
    SvMetaClass *           ReadKnownClass( SvTokenStream & rInStm );
    SvMetaClass *           FindKnownClass( const OString& aName );
    void AddDepFile(OUString const& rFileName);
    void WriteDepFile(SvFileStream & rStream, OUString const& rTarget);
};

class SvIdlWorkingBase : public SvIdlDataBase
{
public:
                explicit SvIdlWorkingBase( const SvCommand& rCmd );

    bool        WriteSfx( SvStream & );
};

#endif // INCLUDED_IDL_INC_DATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
