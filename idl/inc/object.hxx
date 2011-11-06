/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _OBJECT_HXX
#define _OBJECT_HXX

#define _SVSTDARR_ULONGS
//#include <svtools/svstdarr.hxx>
#include <types.hxx>
#include <slot.hxx>

struct SvSlotElement
{
    SvMetaSlotRef   xSlot;
    ByteString          aPrefix;
            SvSlotElement( SvMetaSlot * pS, const ByteString & rPrefix )
                : xSlot( pS )
                , aPrefix( rPrefix )
             {}
};
DECLARE_LIST( SvSlotElementList, SvSlotElement* )
class SvMetaClass;
DECLARE_LIST( SvMetaClassList, SvMetaClass* )

class SvULongs : public List
{
public:
    void    Insert( sal_uLong& rId, sal_uLong nPos ) { sal_uLong nId(rId ); List::Insert( (void*) nId, nPos ); }
    void    Remove( sal_uLong& rId ){ sal_uLong nId(rId ); List::Remove( (void*) nId ); }
    sal_uLong   GetObject( sal_uLong nPos ){ return (sal_uLong) List::GetObject( nPos ); }
};

/******************** class SvClassElement *******************************/
SV_DECL_REF(SvMetaClass)
class SvClassElement : public SvPersistBase
{
    SvBOOL                      aAutomation;
    ByteString                      aPrefix;
    SvMetaClassRef              xClass;
public:
            SV_DECL_PERSIST1( SvClassElement, SvPersistBase, 1 )
            SvClassElement();

    void            SetPrefix( const ByteString & rPrefix )
                    { aPrefix = rPrefix; }
    const ByteString &  GetPrefix() const
                    { return aPrefix; }

    void            SetAutomation( sal_Bool rAutomation )
                    { aAutomation = rAutomation; }
    sal_Bool            GetAutomation() const
                    { return aAutomation; }

    void            SetClass( SvMetaClass * pClass )
                    { xClass = pClass; }
    SvMetaClass *   GetClass() const
                    { return xClass; }
};

SV_DECL_IMPL_REF(SvClassElement)
SV_DECL_IMPL_PERSIST_LIST(SvClassElement,SvClassElement *)

/******************** class SvMetaClass *********************************/
class SvMetaModule;
SV_DECL_PERSIST_LIST(SvMetaClass,SvMetaClass *)
class SvMetaClass : public SvMetaType
{
    SvMetaAttributeMemberList   aAttrList;
    SvClassElementMemberList    aClassList;
    SvMetaClassRef              aSuperClass;
    SvBOOL                      aAutomation;
    SvMetaClassRef              xAutomationInterface;

//    void                FillSbxMemberObject( SvIdlDataBase & rBase,
//                                            SbxObject *, StringList &,
//                                            sal_Bool bVariable );
    sal_Bool                TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                     SvMetaAttribute & rAttr ) const;
#ifdef IDL_COMPILER
    void                WriteSlotStubs( const ByteString & rShellName,
                                        SvSlotElementList & rSlotList,
                                        ByteStringList & rList,
                                        SvStream & rOutStm );
    sal_uInt16              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvSlotElementList & rSlotList,
                                            SvStream & rOutStm );
    sal_uInt16              WriteSlots( const ByteString & rShellName, sal_uInt16 nCount,
                                    SvSlotElementList & rSlotList,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );

    void                InsertSlots( SvSlotElementList& rList, SvULongs& rSuperList,
                                    SvMetaClassList & rClassList,
                                    const ByteString & rPrefix, SvIdlDataBase& rBase );

protected:
    virtual void    ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void    WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
    virtual void    ReadContextSvIdl( SvIdlDataBase &,
                                     SvTokenStream & rInStm );
    virtual void    WriteContextSvIdl( SvIdlDataBase & rBase,
                                     SvStream & rOutStm, sal_uInt16 nTab );
    void            WriteOdlMembers( ByteStringList & rSuperList,
                                    sal_Bool bVariable, sal_Bool bWriteTab,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaClass, SvMetaType, 6 )
            SvMetaClass();

    sal_Bool                GetAutomation() const
                        { return aAutomation; }
    SvMetaClass *       GetSuperClass() const
                        { return aSuperClass; }

    void                FillClasses( SvMetaClassList & rList );
//    virtual void        FillSbxObject( SvIdlDataBase & rBase, SbxObject * );

    const SvClassElementMemberList&
                        GetClassList() const
                        { return aClassList; }

#ifdef IDL_COMPILER
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                Table* pTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 Table * pTable );
    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
#endif
};
SV_IMPL_REF(SvMetaClass)
SV_IMPL_PERSIST_LIST(SvMetaClass,SvMetaClass *)


#endif // _OBJECT_HXX

