/*************************************************************************
 *
 *  $RCSfile: database.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#ifndef _DATABASE_HXX
#define _DATABASE_HXX

#include <module.hxx>
#include <hash.hxx>
#include <lex.hxx>

#ifndef _PSTM_HXX //autogen
#include <tools/pstm.hxx>
#endif

/*************************************************************************
*************************************************************************/

#ifdef IDL_COMPILER
/******************** class SvIdlError ***********************************/
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

/******************** class SvIdlDataBase ********************************/
class SvIdlDataBase
{
    BOOL                        bExport;
    String                      aExportFile;
    ULONG                       nUniqueId;
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
    SvMetaTypeMemberList        aTmpTypeList; // nicht Persistent

protected:
#ifdef IDL_COMPILER
    SvMetaAttributeMemberList&  GetAttrList() { return aAttrList; }
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
                SvIdlDataBase();
                ~SvIdlDataBase();
    static BOOL IsBinaryFormat( SvStream & rInStm );

    void        Load( SvStream & rInStm );
    void        Save( SvStream & rInStm, UINT32 nContextFlags );

    BOOL                      IsModified() const;


    SvStringHashTable *       GetIdTable() { return pIdTable; }
    SvMetaTypeMemberList &    GetTypeList();
    SvMetaClassMemberList &   GetClassList()  { return aClassList; }
    SvMetaModuleMemberList &  GetModuleList() { return aModuleList; }
    SvMetaModule *            GetModule( const ByteString & rName );

    // Liste der benutzten Typen beim Schreiben
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
    BOOL                    FillTypeList( SvMetaTypeList & rOutList,
                                          SvToken * pNameTok );

    ULONG                   GetUniqueId() { return ++nUniqueId; }
    BOOL                    FindId( const ByteString & rIdName, ULONG * pVal );
    BOOL                    InsertId( const ByteString & rIdName, ULONG nVal );
    BOOL                    ReadIdFile( const String & rFileName );

    SvMetaType *            FindType( const ByteString & rName );
    static SvMetaType *     FindType( const SvMetaType *, SvMetaTypeMemberList & );
    static ByteString *     FindName( const ByteString & rName, ByteStringList & );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       FindAttr( SvMetaAttributeMemberList *, SvMetaAttribute * ) const;
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
                SvIdlWorkingBase();

    BOOL        ReadSvIdl( SvTokenStream &, BOOL bImported, const String & rPath );
    BOOL        WriteSvIdl( SvStream & );

    BOOL        WriteSfx( SvStream & );
    BOOL        WriteHelpIds( SvStream & );
    BOOL        WriteCHeader( SvStream & );
    BOOL        WriteCSource( SvStream & );
    BOOL        WriteSfxItem( SvStream & );
//    BOOL        WriteSbx( SvStream & );
    BOOL        WriteOdl( SvStream & );
    BOOL        WriteSrc( SvStream & );
    BOOL        WriteCxx( SvStream & );
    BOOL        WriteHxx( SvStream & );
    BOOL        WriteCSV( SvStream& );
    BOOL        WriteDocumentation( SvStream& );
};
#endif

#endif // _DATABASE_HXX

