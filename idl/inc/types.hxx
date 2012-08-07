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

#ifndef _TYPES_HXX
#define _TYPES_HXX

#include <rtl/strbuf.hxx>
#include <tools/ref.hxx>
#include <basobj.hxx>
#include <map>

struct SvSlotElement;
typedef std::vector< SvSlotElement* > SvSlotElementList;

class SvMetaSlot;
typedef std::map<sal_uLong, SvMetaSlot*> HelpIdTable;

SV_DECL_REF(SvMetaType)
SV_DECL_REF(SvMetaAttribute)

class SvMetaAttributeMemberList : public SvDeclPersistList<SvMetaAttribute *> {};

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
    sal_Bool                bNewAttr;

protected:
    virtual void WriteCSource( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, sal_Bool bSet );
    sal_uLong        MakeSlotValue( SvIdlDataBase & rBase, sal_Bool bVariable ) const;
    virtual void WriteAttributes( SvIdlDataBase & rBase,
                                      SvStream & rOutStm, sal_uInt16 nTab,
                                        WriteType, WriteAttribute = 0 );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
public:
            SV_DECL_META_FACTORY1( SvMetaAttribute, SvMetaReference, 2 )
                        SvMetaAttribute();
                        SvMetaAttribute( SvMetaType * );

    void                SetNewAttribute( sal_Bool bNew )
                        { bNewAttr = bNew; }
    sal_Bool                IsNewAttribute() const
                        { return bNewAttr; }
    sal_Bool                GetReadonly() const;

    void                SetSlotId( const SvNumberIdentifier & rId )
                        { aSlotId = rId; }
    const SvNumberIdentifier & GetSlotId() const;

    void                SetExport( sal_Bool bSet )
                        { aExport = bSet; }
    sal_Bool                GetExport() const;

    void                SetHidden( sal_Bool bSet )
                        { aHidden = bSet; }
    sal_Bool                GetHidden() const;

    void                SetAutomation( sal_Bool bSet )
                        { aAutomation = bSet; }
    sal_Bool                GetAutomation() const;

    void                SetIsCollection( sal_Bool bSet )
                        { aIsCollection = bSet; }
    sal_Bool                GetIsCollection() const;
    void                SetReadOnlyDoc( sal_Bool bSet )
                        { aReadOnlyDoc = bSet; }
    sal_Bool                GetReadOnlyDoc() const;

    void                SetType( SvMetaType * pT ) { aType = pT; }
    SvMetaType *        GetType() const;

    virtual sal_Bool        IsMethod() const;
    virtual sal_Bool        IsVariable() const;
    virtual rtl::OString    GetMangleName( sal_Bool bVariable ) const;


    virtual sal_Bool        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        WriteParam( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                    WriteType );
    void                WriteRecursiv_Impl( SvIdlDataBase & rBase,
                                        SvStream & rOutStm, sal_uInt16 nTab,
                                          WriteType, WriteAttribute );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
    sal_uLong           MakeSfx( rtl::OStringBuffer& rAtrrArray );
    virtual void        Insert( SvSlotElementList&, const rtl::OString& rPrefix,
                                SvIdlDataBase& );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  HelpIdTable& rIdTable );
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& );
    void                FillIDTable(HelpIdTable& rIDTable);
    rtl::OString        Compare( SvMetaAttribute *pAttr );
};
SV_IMPL_REF(SvMetaAttribute)


enum { CALL_VALUE, CALL_POINTER, CALL_REFERENCE };
enum { TYPE_METHOD, TYPE_STRUCT, TYPE_BASE, TYPE_ENUM, TYPE_UNION,
      TYPE_CLASS, TYPE_POINTER };
class SvMetaType : public SvMetaExtern
{
    SvBOOL                      aIn;    // input parameter
    SvBOOL                      aOut;   // return parameter
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
    sal_Bool                        bIsItem;
    sal_Bool                        bIsShell;
    char                        cParserChar;

    void    WriteSfxItem( const rtl::OString& rItemName, SvIdlDataBase & rBase,
                        SvStream & rOutStm );
protected:
    sal_Bool        ReadNamesSvIdl( SvIdlDataBase & rBase,
                                         SvTokenStream & rInStm );
    virtual void ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   sal_uInt16 nTab );

    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
    virtual void WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
    sal_Bool    ReadHeaderSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    void    WriteHeaderSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                              sal_uInt16 nTab );
public:
            SV_DECL_META_FACTORY1( SvMetaType, SvMetaExtern, 18 )
            SvMetaType();
            SvMetaType( const rtl::OString& rTypeName, char cParserChar,
                                const rtl::OString& rCName );
            SvMetaType( const rtl::OString& rTypeName, const rtl::OString& rSbxName,
                        const rtl::OString& rOdlName, char cParserChar,
                        const rtl::OString& rCName, const rtl::OString& rBasicName,
                        const rtl::OString& rBasicPostfix );

    SvMetaAttributeMemberList & GetAttrList() const;
    sal_uLong               GetAttrCount() const
                        {
                            return pAttrList ? pAttrList->size() : 0L;
                        }
    void                AppendAttr( SvMetaAttribute * pAttr )
                        {
                            GetAttrList().push_back( pAttr );
                        }

    void                SetType( int nT );
    int                 GetType() const { return nType; }
    SvMetaType *        GetBaseType() const;
    SvMetaType *        GetReturnType() const;
    sal_Bool                IsItem() const { return bIsItem; }
    sal_Bool                IsShell() const { return bIsShell; }

    void                SetIn( sal_Bool b ) { aIn = b; }
    sal_Bool                GetIn() const;

    void                SetOut( sal_Bool b ) { aOut = b; }
    sal_Bool                GetOut() const;

    void                SetCall0( int e );
    int                 GetCall0() const;

    void                SetCall1( int e);
    int                 GetCall1() const;

    void                SetBasicName(const rtl::OString& rName)
                        { aBasicName.setString(rName); }

    const rtl::OString& GetBasicName() const;
    rtl::OString GetBasicPostfix() const;
    const rtl::OString& GetSvName() const;
    const rtl::OString& GetSbxName() const;
    const rtl::OString& GetOdlName() const;
    const rtl::OString& GetCName() const;
    char                GetParserChar() const { return cParserChar; }

    virtual sal_Bool    SetName( const rtl::OString& rName, SvIdlDataBase * = NULL );


    virtual sal_Bool    ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
    rtl::OString        GetCString() const;
    void                WriteSvIdlType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    void                WriteOdlType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    sal_uLong           MakeSfx( rtl::OStringBuffer& rAtrrArray );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    sal_Bool                ReadMethodArgs( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    void                WriteTypePrefix( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    void                WriteMethodArgs( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    void                WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    rtl::OString GetParserString() const;
    void                WriteParamNames( SvIdlDataBase & rBase, SvStream & rOutStm,
                                        const rtl::OString& rChief );
};
SV_IMPL_REF(SvMetaType)

class SvMetaTypeMemberList : public SvDeclPersistList<SvMetaType *> {};

class SvMetaTypeString : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypeString, SvMetaType, 19 )
            SvMetaTypeString();
};
SV_DECL_IMPL_REF(SvMetaTypeString)

class SvMetaTypeStringMemberList : public SvDeclPersistList<SvMetaTypeString *> {};

class SvMetaEnumValue : public SvMetaName
{
    rtl::OString aEnumValue;
public:
    SV_DECL_META_FACTORY1( SvMetaEnumValue, SvMetaName, 20 )
    SvMetaEnumValue();

    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
};
SV_DECL_IMPL_REF(SvMetaEnumValue)

class SvMetaEnumValueMemberList : public SvDeclPersistList<SvMetaEnumValue *> {};

class SvMetaTypeEnum : public SvMetaType
{
    SvMetaEnumValueMemberList   aEnumValueList;
    rtl::OString aPrefix;
protected:
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   sal_uInt16 nTab );
    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
public:
            SV_DECL_META_FACTORY1( SvMetaTypeEnum, SvMetaType, 21 )
            SvMetaTypeEnum();

    sal_uInt16              GetMaxValue() const;
    sal_uLong               Count() const { return aEnumValueList.size(); }
    const rtl::OString&     GetPrefix() const { return aPrefix; }
    SvMetaEnumValue *   GetObject( sal_uLong n ) const
                        { return aEnumValueList[n]; }

    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
};
SV_DECL_IMPL_REF(SvMetaTypeEnum)

class SvMetaTypeEnumMemberList : public SvDeclPersistList<SvMetaTypeEnum *> {};

class SvMetaTypevoid : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypevoid, SvMetaName, 22 )
            SvMetaTypevoid();
};
SV_DECL_IMPL_REF(SvMetaTypevoid)
class SvMetaTypevoidMemberList : public SvDeclPersistList<SvMetaTypevoid *> {};


#endif // _TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
