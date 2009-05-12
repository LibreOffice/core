/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _TYPES_HXX
#define _TYPES_HXX

#include <tools/ref.hxx>
#include <basobj.hxx>

class SvSlotElementList;
struct SvSlotElement;

/******************** class SvMetaAttribute *****************************/
SV_DECL_REF(SvMetaType)
SV_DECL_REF(SvMetaAttribute)
SV_DECL_PERSIST_LIST(SvMetaAttribute,SvMetaAttribute *)
class SvMetaAttribute : public SvMetaReference
{
    SvMetaTypeRef       aType;
    SvNumberIdentifier  aSlotId;
    SvBOOL              aAutomation;
    SvBOOL              aExport;
    SvBOOL              aReadonly;
    SvBOOL              aIsCollection;
    SvBOOL              aReadOnlyDoc;
    SvBOOL              aHidden;
    BOOL                bNewAttr;

protected:
#ifdef IDL_COMPILER
    virtual void WriteCSource( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, BOOL bSet );
    ULONG        MakeSlotValue( SvIdlDataBase & rBase, BOOL bVariable ) const;
    virtual void WriteAttributes( SvIdlDataBase & rBase,
                                      SvStream & rOutStm, USHORT nTab,
                                        WriteType, WriteAttribute = 0 );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaAttribute, SvMetaReference, 2 )
                        SvMetaAttribute();
                        SvMetaAttribute( SvMetaType * );

    void                SetNewAttribute( BOOL bNew )
                        { bNewAttr = bNew; }
    BOOL                IsNewAttribute() const
                        { return bNewAttr; }
    BOOL                GetReadonly() const;

    void                SetSlotId( const SvNumberIdentifier & rId )
                        { aSlotId = rId; }
    const SvNumberIdentifier & GetSlotId() const;

    void                SetExport( BOOL bSet )
                        { aExport = bSet; }
    BOOL                GetExport() const;

    void                SetHidden( BOOL bSet )
                        { aHidden = bSet; }
    BOOL                GetHidden() const;

    void                SetAutomation( BOOL bSet )
                        { aAutomation = bSet; }
    BOOL                GetAutomation() const;

    void                SetIsCollection( BOOL bSet )
                        { aIsCollection = bSet; }
    BOOL                GetIsCollection() const;
    void                SetReadOnlyDoc( BOOL bSet )
                        { aReadOnlyDoc = bSet; }
    BOOL                GetReadOnlyDoc() const;

    void                SetType( SvMetaType * pT ) { aType = pT; }
    SvMetaType *        GetType() const;

    virtual BOOL        IsMethod() const;
    virtual BOOL        IsVariable() const;
    virtual ByteString      GetMangleName( BOOL bVariable ) const;

//    void                FillSbxObject( SbxInfo * pInfo, USHORT nSbxFlags = 0 );
//    virtual void        FillSbxObject( SvIdlDataBase & rBase, SbxObject * pObj, BOOL bVariable );

#ifdef IDL_COMPILER
    virtual BOOL        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        WriteParam( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                    WriteType );
    void                WriteRecursiv_Impl( SvIdlDataBase & rBase,
                                        SvStream & rOutStm, USHORT nTab,
                                          WriteType, WriteAttribute );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    ULONG               MakeSfx( ByteString * pAtrrArray );
    virtual void        Insert( SvSlotElementList&, const ByteString & rPrefix,
                                SvIdlDataBase& );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& );
    void                FillIDTable(Table *pIDTable);
    ByteString              Compare( SvMetaAttribute *pAttr );
#endif
};
SV_IMPL_REF(SvMetaAttribute)
SV_IMPL_PERSIST_LIST(SvMetaAttribute,SvMetaAttribute *)


/******************** class SvType *********************************/
enum { CALL_VALUE, CALL_POINTER, CALL_REFERENCE };
enum { TYPE_METHOD, TYPE_STRUCT, TYPE_BASE, TYPE_ENUM, TYPE_UNION,
      TYPE_CLASS, TYPE_POINTER };
class SvMetaType : public SvMetaExtern
{
    SvBOOL                      aIn;    // Eingangsparameter
    SvBOOL                      aOut;   // Returnparameter
    Svint                       aCall0, aCall1;
    Svint                       aSbxDataType;
    SvIdentifier                aSvName;
    SvIdentifier                aSbxName;
    SvIdentifier                aOdlName;
    SvIdentifier                aCName;
    SvIdentifier                aBasicPostfix;
    SvIdentifier                aBasicName;
    SvMetaAttributeMemberList * pAttrList;
    int                         nType;
    BOOL                        bIsItem;
    BOOL                        bIsShell;
    char                        cParserChar;

#ifdef IDL_COMPILER
    void    WriteSfxItem( const ByteString & rItemName, SvIdlDataBase & rBase,
                        SvStream & rOutStm );
protected:
    BOOL        ReadNamesSvIdl( SvIdlDataBase & rBase,
                                         SvTokenStream & rInStm );
    virtual void ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   USHORT nTab );

    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    virtual void WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    BOOL    ReadHeaderSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    void    WriteHeaderSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                              USHORT nTab );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaType, SvMetaExtern, 18 )
            SvMetaType();
            SvMetaType( const ByteString & rTypeName, char cParserChar,
                                const ByteString & rCName );
            SvMetaType( const ByteString & rTypeName, const ByteString & rSbxName,
                        const ByteString & rOdlName, char cParserChar,
                        const ByteString & rCName, const ByteString & rBasicName,
                        const ByteString & rBasicPostfix/*, SbxDataType nT = SbxEMPTY */);

    SvMetaAttributeMemberList & GetAttrList() const;
    ULONG               GetAttrCount() const
                        {
                            return pAttrList ? pAttrList->Count() : 0L;
                        }
    void                AppendAttr( SvMetaAttribute * pAttr )
                        {
                            GetAttrList().Append( pAttr );
                        }

    void                SetType( int nT );
    int                 GetType() const { return nType; }
    SvMetaType *        GetBaseType() const;
    SvMetaType *        GetReturnType() const;
    BOOL                IsItem() const { return bIsItem; }
    BOOL                IsShell() const { return bIsShell; }

//    void                SetSbxDataType( SbxDataType nT )
//                        { aSbxDataType = (int)nT; }
//    SbxDataType         GetSbxDataType() const;

    void                SetIn( BOOL b ) { aIn = b; }
    BOOL                GetIn() const;

    void                SetOut( BOOL b ) { aOut = b; }
    BOOL                GetOut() const;

    void                SetCall0( int e );
    int                 GetCall0() const;

    void                SetCall1( int e);
    int                 GetCall1() const;

    void                SetBasicName(const ByteString& rName)
                        { aBasicName = rName; }

    const ByteString &      GetBasicName() const;
    ByteString              GetBasicPostfix() const;
    const ByteString &      GetSvName() const;
    const ByteString &      GetSbxName() const;
    const ByteString &      GetOdlName() const;
    const ByteString &      GetCName() const;
    char                GetParserChar() const { return cParserChar; }

    virtual BOOL        SetName( const ByteString & rName, SvIdlDataBase * = NULL );

//    void                FillSbxObject( SbxVariable * pObj, BOOL bVariable );

#ifdef IDL_COMPILER
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    ByteString              GetCString() const;
    void                WriteSvIdlType( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    void                WriteOdlType( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    void                AppendParserString (ByteString &rString);

    ULONG               MakeSfx( ByteString * pAtrrArray );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    //BOOL              ReadTypePrefix( SvIdlDataBase &, SvTokenStream & rInStm );
    BOOL                ReadMethodArgs( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    void                WriteTypePrefix( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab, WriteType );
    void                WriteMethodArgs( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab, WriteType );
    void                WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab, WriteType );
    ByteString              GetParserString() const;
    void                WriteParamNames( SvIdlDataBase & rBase, SvStream & rOutStm,
                                        const ByteString & rChief );
#endif
};
SV_IMPL_REF(SvMetaType)
DECLARE_LIST(SvMetaTypeList,SvMetaType *)
SV_DECL_IMPL_PERSIST_LIST(SvMetaType,SvMetaType *)


/******************** class SvTypeString *********************************/
class SvMetaTypeString : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypeString, SvMetaType, 19 )
            SvMetaTypeString();
};
SV_DECL_IMPL_REF(SvMetaTypeString)
SV_DECL_IMPL_PERSIST_LIST(SvMetaTypeString,SvMetaTypeString *)


/******************** class SvMetaEnumValue **********************************/
class SvMetaEnumValue : public SvMetaName
{
    ByteString      aEnumValue;
public:
            SV_DECL_META_FACTORY1( SvMetaEnumValue, SvMetaName, 20 )
            SvMetaEnumValue();

#ifdef IDL_COMPILER
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
#endif
};
SV_DECL_IMPL_REF(SvMetaEnumValue)
SV_DECL_IMPL_PERSIST_LIST(SvMetaEnumValue,SvMetaEnumValue *)


/******************** class SvTypeEnum *********************************/
class SvMetaTypeEnum : public SvMetaType
{
    SvMetaEnumValueMemberList   aEnumValueList;
    ByteString                      aPrefix;
protected:
#ifdef IDL_COMPILER
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   USHORT nTab );
    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaTypeEnum, SvMetaType, 21 )
            SvMetaTypeEnum();

    USHORT              GetMaxValue() const;
    ULONG               Count() const { return aEnumValueList.Count(); }
    const ByteString &      GetPrefix() const { return aPrefix; }
    SvMetaEnumValue *   GetObject( ULONG n ) const
                        { return aEnumValueList.GetObject( n ); }

#ifdef IDL_COMPILER
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
#endif
};
SV_DECL_IMPL_REF(SvMetaTypeEnum)
SV_DECL_IMPL_PERSIST_LIST(SvMetaTypeEnum,SvMetaTypeEnum *)


/******************** class SvTypeVoid ***********************************/
class SvMetaTypevoid : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypevoid, SvMetaName, 22 )
            SvMetaTypevoid();
};
SV_DECL_IMPL_REF(SvMetaTypevoid)
SV_DECL_IMPL_PERSIST_LIST(SvMetaTypevoid,SvMetaTypevoid *)


#endif // _TYPES_HXX

