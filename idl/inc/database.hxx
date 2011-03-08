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
#include <tools/pstm.hxx>

class SvCommand;

#ifdef IDL_COMPILER

class SvIdlError
{
    ByteString  aText;
public:
    UINT32  nLine, nColumn;

            SvIdlError() : nLine(0), nColumn(0) {}
            SvIdlError( UINT32 nL, UINT32 nC )
                : nLine(nL), nColumn(nC) {}

    const ByteString &  GetText() const { return aText; }
    void            SetText( const ByteString & rT ) { aText = rT; }
    BOOL            IsError() const { return nLine != 0; }
    void            Clear() { nLine = nColumn = 0; }
    SvIdlError &    operator = ( const SvIdlError & rRef )
    { aText   = rRef.aText;
      nLine   = rRef.nLine;
      nColumn = rRef.nColumn;
      return *this;
    }
};
#endif

class SvIdlDataBase
{
    BOOL                        bExport;
    String                      aExportFile;
    sal_uInt32                  nUniqueId;
    sal_uInt32                  nVerbosity;
    String                      aDataBaseFile;
    SvFileStream *              pStm;
    BOOL                        bIsModified;
    SvPersistStream             aPersStream;
    StringList                  aIdFileList;
    SvStringHashTable *         pIdTable;

    SvMetaTypeMemberList        aTypeList;
    SvMetaClassMemberList       aClassList;
    SvMetaModuleMemberList      aModuleList;
    SvMetaAttributeMemberList   aAttrList;
    SvMetaTypeMemberList        aTmpTypeList; // not persistent

protected:
#ifdef IDL_COMPILER
    ByteString                      aModulePrefix;
    SvMetaObjectMemberStack     aContextStack;
    String                      aPath;
    SvIdlError                  aError;
    void                        WriteReset()
                                {
                                    aUsedTypes.Clear();
                                    aIFaceName.Erase();
                                }
#endif
public:
                explicit SvIdlDataBase( const SvCommand& rCmd );
                ~SvIdlDataBase();
    static BOOL IsBinaryFormat( SvStream & rInStm );

    void        Load( SvStream & rInStm );
    void        Save( SvStream & rInStm, UINT32 nContextFlags );

    SvMetaAttributeMemberList&  GetAttrList() { return aAttrList; }
    SvStringHashTable *       GetIdTable() { return pIdTable; }
    SvMetaTypeMemberList &    GetTypeList();
    SvMetaClassMemberList &   GetClassList()  { return aClassList; }
    SvMetaModuleMemberList &  GetModuleList() { return aModuleList; }
    SvMetaModule *            GetModule( const ByteString & rName );

    // list of used types while writing
    SvMetaTypeMemberList    aUsedTypes;
    ByteString                  aIFaceName;
    SvNumberIdentifier      aStructSlotId;

#ifdef IDL_COMPILER
    void                    StartNewFile( const String& rName );
    void                    SetExportFile( const String& rName )
                            { aExportFile = rName; }
    void                    AppendAttr( SvMetaAttribute *pSlot );
    const ByteString&           GetActModulePrefix() const { return aModulePrefix; }
    const SvIdlError &      GetError() const { return aError; }
    void                    SetError( const SvIdlError & r )
                            { aError = r; }

    const String &            GetPath() const { return aPath; }
    SvMetaObjectMemberStack & GetStack()      { return aContextStack; }

    void                    Write( const ByteString & rText );
    void                    WriteError( const ByteString & rErrWrn,
                                    const ByteString & rFileName,
                                    const ByteString & rErrorText,
                                    ULONG nRow = 0, ULONG nColumn = 0 ) const;
    void                    WriteError( SvTokenStream & rInStm );
    void                    SetError( const ByteString & rError, SvToken * pTok );
    void                    Push( SvMetaObject * pObj );
    BOOL                    Pop( BOOL bOk, SvTokenStream & rInStm, UINT32 nTokPos )
                            {
                                GetStack().Pop();
                                if( bOk )
                                    aError.Clear();
                                else
                                    rInStm.Seek( nTokPos );
                                return bOk;
                            }
    sal_uInt32              GetUniqueId() { return ++nUniqueId; }
    BOOL                    FindId( const ByteString & rIdName, ULONG * pVal );
    BOOL                    InsertId( const ByteString & rIdName, ULONG nVal );
    BOOL                    ReadIdFile( const String & rFileName );

    SvMetaType *            FindType( const ByteString & rName );
    static SvMetaType *     FindType( const SvMetaType *, SvMetaTypeMemberList & );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       ReadKnownAttr( SvTokenStream & rInStm,
                                            SvMetaType * pType = NULL );
    SvMetaAttribute *       SearchKnownAttr( const SvNumberIdentifier& );
    SvMetaClass *           ReadKnownClass( SvTokenStream & rInStm );
#endif
};

#ifdef IDL_COMPILER
class SvIdlWorkingBase : public SvIdlDataBase
{
public:
                explicit SvIdlWorkingBase( const SvCommand& rCmd );

    BOOL        ReadSvIdl( SvTokenStream &, BOOL bImported, const String & rPath );
    BOOL        WriteSvIdl( SvStream & );

    BOOL        WriteSfx( SvStream & );
    BOOL        WriteHelpIds( SvStream & );
    BOOL        WriteSfxItem( SvStream & );
    BOOL        WriteCSV( SvStream& );
    BOOL        WriteDocumentation( SvStream& );
};
#endif

#endif // _DATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
