/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _DATABASE_HXX
#define _DATABASE_HXX

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
    rtl::OString aText;
public:
    sal_uInt32  nLine, nColumn;

            SvIdlError() : nLine(0), nColumn(0) {}
            SvIdlError( sal_uInt32 nL, sal_uInt32 nC )
                : nLine(nL), nColumn(nC) {}

    const rtl::OString&  GetText() const { return aText; }
    void SetText( const rtl::OString& rT ) { aText = rT; }
    sal_Bool            IsError() const { return nLine != 0; }
    void            Clear() { nLine = nColumn = 0; }
    SvIdlError &    operator = ( const SvIdlError & rRef )
    { aText   = rRef.aText;
      nLine   = rRef.nLine;
      nColumn = rRef.nColumn;
      return *this;
    }
};

class SvIdlDataBase
{
    sal_Bool                        bExport;
    String                      aExportFile;
    sal_uInt32                  nUniqueId;
    sal_uInt32                  nVerbosity;
    String                      aDataBaseFile;
    SvFileStream *              pStm;
    sal_Bool                        bIsModified;
    SvPersistStream             aPersStream;
    StringList                  aIdFileList;
    SvStringHashTable *         pIdTable;

    SvMetaTypeMemberList        aTypeList;
    SvMetaClassMemberList       aClassList;
    SvMetaModuleMemberList      aModuleList;
    SvMetaAttributeMemberList   aAttrList;
    SvMetaTypeMemberList        aTmpTypeList; // not persistent

protected:
    ::std::set< ::rtl::OUString > m_DepFiles;
    SvMetaObjectMemberStack     aContextStack;
    String                      aPath;
    SvIdlError                  aError;
    void WriteReset()
    {
        aUsedTypes.Clear();
        aIFaceName = rtl::OString();
    }
public:
                explicit SvIdlDataBase( const SvCommand& rCmd );
                ~SvIdlDataBase();
    static sal_Bool IsBinaryFormat( SvStream & rInStm );

    void        Load( SvStream & rInStm );
    void        Save( SvStream & rInStm, sal_uInt32 nContextFlags );

    SvMetaAttributeMemberList&  GetAttrList() { return aAttrList; }
    SvStringHashTable *       GetIdTable() { return pIdTable; }
    SvMetaTypeMemberList &    GetTypeList();
    SvMetaClassMemberList &   GetClassList()  { return aClassList; }
    SvMetaModuleMemberList &  GetModuleList() { return aModuleList; }
    SvMetaModule *            GetModule( const rtl::OString& rName );

    // list of used types while writing
    SvMetaTypeMemberList    aUsedTypes;
    rtl::OString            aIFaceName;
    SvNumberIdentifier      aStructSlotId;

    void                    StartNewFile( const String& rName );
    void                    SetExportFile( const String& rName )
                            { aExportFile = rName; }
    void                    AppendAttr( SvMetaAttribute *pSlot );
    const SvIdlError &      GetError() const { return aError; }
    void                    SetError( const SvIdlError & r )
                            { aError = r; }

    const String &            GetPath() const { return aPath; }
    SvMetaObjectMemberStack & GetStack()      { return aContextStack; }

    void                    Write(const rtl::OString& rText);
    void                    WriteError(const rtl::OString& rErrWrn,
                                    const rtl::OString& rFileName,
                                    const rtl::OString& rErrorText,
                                    sal_uLong nRow = 0, sal_uLong nColumn = 0 ) const;
    void                    WriteError( SvTokenStream & rInStm );
    void                    SetError( const rtl::OString& rError, SvToken * pTok );
    void                    Push( SvMetaObject * pObj );
    sal_Bool                    Pop( sal_Bool bOk, SvTokenStream & rInStm, sal_uInt32 nTokPos )
                            {
                                GetStack().Pop();
                                if( bOk )
                                    aError.Clear();
                                else
                                    rInStm.Seek( nTokPos );
                                return bOk;
                            }
    sal_uInt32              GetUniqueId() { return ++nUniqueId; }
    sal_Bool                FindId( const rtl::OString& rIdName, sal_uLong * pVal );
    sal_Bool                InsertId( const rtl::OString& rIdName, sal_uLong nVal );
    sal_Bool                    ReadIdFile( const String & rFileName );

    SvMetaType *            FindType( const rtl::OString& rName );
    static SvMetaType *     FindType( const SvMetaType *, SvMetaTypeMemberList & );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       ReadKnownAttr( SvTokenStream & rInStm,
                                            SvMetaType * pType = NULL );
    SvMetaAttribute *       SearchKnownAttr( const SvNumberIdentifier& );
    SvMetaClass *           ReadKnownClass( SvTokenStream & rInStm );
    void AddDepFile(String const& rFileName);
    bool WriteDepFile(SvFileStream & rStream, ::rtl::OUString const& rTarget);
};

class SvIdlWorkingBase : public SvIdlDataBase
{
public:
                explicit SvIdlWorkingBase( const SvCommand& rCmd );

    sal_Bool        ReadSvIdl( SvTokenStream &, sal_Bool bImported, const String & rPath );
    sal_Bool        WriteSvIdl( SvStream & );

    sal_Bool        WriteSfx( SvStream & );
    sal_Bool        WriteHelpIds( SvStream & );
    sal_Bool        WriteSfxItem( SvStream & );
    sal_Bool        WriteCSV( SvStream& );
    sal_Bool        WriteDocumentation( SvStream& );
};

#endif // _DATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
