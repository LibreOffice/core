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

#ifndef INCLUDED_IDL_INC_TYPES_HXX
#define INCLUDED_IDL_INC_TYPES_HXX

#include <rtl/strbuf.hxx>
#include <tools/ref.hxx>
#include <basobj.hxx>
#include <map>

struct SvSlotElement;
typedef std::vector< SvSlotElement* > SvSlotElementList;

class SvMetaSlot;
typedef std::map<sal_uLong, SvMetaSlot*> HelpIdTable;

class SvMetaType;
typedef tools::SvRef<SvMetaType> SvMetaTypeRef;

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
    bool                bNewAttr;

protected:
    virtual void WriteCSource( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, bool bSet );
    sal_uLong        MakeSlotValue( SvIdlDataBase & rBase, bool bVariable ) const;
    virtual void WriteAttributes( SvIdlDataBase & rBase,
                                      SvStream & rOutStm, sal_uInt16 nTab,
                                        WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
public:
            SV_DECL_META_FACTORY1( SvMetaAttribute, SvMetaReference, 2 )
                        SvMetaAttribute();
                        SvMetaAttribute( SvMetaType * );

    void                SetNewAttribute( bool bNew )
                        { bNewAttr = bNew; }
    bool                IsNewAttribute() const
                        { return bNewAttr; }
    bool                GetReadonly() const;

    void                SetSlotId( const SvNumberIdentifier & rId )
                        { aSlotId = rId; }
    const SvNumberIdentifier & GetSlotId() const;

    void                SetExport( bool bSet )
                        { aExport = bSet; }
    bool                GetExport() const;

    void                SetHidden( bool bSet )
                        { aHidden = bSet; }
    bool                GetHidden() const;

    void                SetAutomation( bool bSet )
                        { aAutomation = bSet; }
    bool                GetAutomation() const;

    void                SetIsCollection( bool bSet )
                        { aIsCollection = bSet; }
    bool                GetIsCollection() const;
    void                SetReadOnlyDoc( bool bSet )
                        { aReadOnlyDoc = bSet; }
    bool                GetReadOnlyDoc() const;

    void                SetType( SvMetaType * pT ) { aType = pT; }
    SvMetaType *        GetType() const;

    virtual bool        IsMethod() const;
    virtual bool        IsVariable() const;
    virtual OString     GetMangleName( bool bVariable ) const;


    virtual bool        Test( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void        WriteParam( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                    WriteType );
    void                WriteRecursiv_Impl( SvIdlDataBase & rBase,
                                        SvStream & rOutStm, sal_uInt16 nTab,
                                          WriteType, WriteAttribute );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    sal_uLong           MakeSfx( OStringBuffer& rAtrrArray );
    virtual void        Insert( SvSlotElementList&, const OString& rPrefix,
                                SvIdlDataBase& );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  HelpIdTable& rIdTable );
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& );
    void                FillIDTable(HelpIdTable& rIDTable);
    OString        Compare( SvMetaAttribute *pAttr );
};

typedef tools::SvRef<SvMetaAttribute> SvMetaAttributeRef;

class SvMetaAttributeMemberList : public SvDeclPersistList<SvMetaAttribute *> {};

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
    bool                        bIsItem;
    bool                        bIsShell;
    char                        cParserChar;

    void    WriteSfxItem( const OString& rItemName, SvIdlDataBase & rBase,
                        SvStream & rOutStm );
protected:
    bool        ReadNamesSvIdl( SvIdlDataBase & rBase,
                                         SvTokenStream & rInStm );
    virtual void ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   sal_uInt16 nTab ) SAL_OVERRIDE;

    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    virtual void WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    bool    ReadHeaderSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    void    WriteHeaderSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                              sal_uInt16 nTab );
public:
            SV_DECL_META_FACTORY1( SvMetaType, SvMetaExtern, 18 )
            SvMetaType();
            SvMetaType( const OString& rTypeName, char cParserChar,
                                const OString& rCName );
            SvMetaType( const OString& rTypeName, const OString& rSbxName,
                        const OString& rOdlName, char cParserChar,
                        const OString& rCName, const OString& rBasicName,
                        const OString& rBasicPostfix );

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
    bool                IsItem() const { return bIsItem; }
    bool                IsShell() const { return bIsShell; }

    void                SetIn( bool b ) { aIn = b; }
    bool                GetIn() const;

    void                SetOut( bool b ) { aOut = b; }
    bool                GetOut() const;

    void                SetCall0( int e );
    int                 GetCall0() const;

    void                SetCall1( int e);
    int                 GetCall1() const;

    void                SetBasicName(const OString& rName)
                        { aBasicName.setString(rName); }

    const OString& GetBasicName() const;
    OString GetBasicPostfix() const;
    const OString& GetSvName() const;
    const OString& GetSbxName() const;
    const OString& GetOdlName() const;
    const OString& GetCName() const;
    char                GetParserChar() const { return cParserChar; }

    virtual bool        SetName( const OString& rName, SvIdlDataBase * = NULL ) SAL_OVERRIDE;


    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    OString             GetCString() const;
    void                WriteSvIdlType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    void                WriteOdlType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    sal_uLong           MakeSfx( OStringBuffer& rAtrrArray );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    bool                ReadMethodArgs( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    void                WriteTypePrefix( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    void                WriteMethodArgs( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    void                WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab, WriteType );
    OString             GetParserString() const;
    void                WriteParamNames( SvIdlDataBase & rBase, SvStream & rOutStm,
                                        const OString& rChief );
};

class SvMetaTypeMemberList : public SvDeclPersistList<SvMetaType *> {};

class SvMetaTypeString : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypeString, SvMetaType, 19 )
            SvMetaTypeString();
};
typedef tools::SvRef<SvMetaTypeString> SvMetaTypeStringRef;

class SvMetaTypeStringMemberList : public SvDeclPersistList<SvMetaTypeString *> {};

class SvMetaEnumValue : public SvMetaName
{
    OString aEnumValue;
public:
    SV_DECL_META_FACTORY1( SvMetaEnumValue, SvMetaName, 20 )
    SvMetaEnumValue();

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
};
typedef tools::SvRef<SvMetaEnumValue> SvMetaEnumValueRef;

class SvMetaEnumValueMemberList : public SvDeclPersistList<SvMetaEnumValue *> {};

class SvMetaTypeEnum : public SvMetaType
{
    SvMetaEnumValueMemberList   aEnumValueList;
    OString aPrefix;
protected:
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void WriteContextSvIdl( SvIdlDataBase &, SvStream & rOutStm,
                                   sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
public:
            SV_DECL_META_FACTORY1( SvMetaTypeEnum, SvMetaType, 21 )
            SvMetaTypeEnum();

    sal_uInt16          GetMaxValue() const;
    sal_uLong           Count() const { return aEnumValueList.size(); }
    const OString&      GetPrefix() const { return aPrefix; }
    SvMetaEnumValue *   GetObject( sal_uLong n ) const
                        { return aEnumValueList[n]; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
};
typedef tools::SvRef<SvMetaTypeEnum> SvMetaTypeEnumRef;

class SvMetaTypeEnumMemberList : public SvDeclPersistList<SvMetaTypeEnum *> {};

class SvMetaTypevoid : public SvMetaType
{
public:
            SV_DECL_META_FACTORY1( SvMetaTypevoid, SvMetaName, 22 )
            SvMetaTypevoid();
};
typedef tools::SvRef<SvMetaTypevoid> SvMetaTypevoidRef;
class SvMetaTypevoidMemberList : public SvDeclPersistList<SvMetaTypevoid *> {};


#endif // INCLUDED_IDL_INC_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
