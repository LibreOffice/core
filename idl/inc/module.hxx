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



#ifndef _MODULE_HXX
#define _MODULE_HXX

#include <slot.hxx>
#include <object.hxx>

struct SvNamePos
{
    SvGlobalName    aUUId;
    sal_uInt32          nStmPos;
    SvNamePos( const SvGlobalName & rName, sal_uInt32 nPos )
        : aUUId( rName )
        , nStmPos( nPos ) {}
};
DECLARE_LIST( SvNamePosList, SvNamePos *)

/******************** class SvMetaModule *********************************/
class SvMetaModule : public SvMetaExtern
{
    SvMetaClassMemberList       aClassList;
    SvMetaTypeMemberList        aTypeList;
    SvMetaAttributeMemberList   aAttrList;
// Browser
    String                  aIdlFileName;
    SvString                aHelpFileName;
    SvString                aSlotIdFile;
    SvString                aTypeLibFile;
    SvString                aModulePrefix;

#ifdef IDL_COMPILER
    sal_Bool                    bImported   : 1,
                            bIsModified : 1;
    SvGlobalName            aBeginName;
    SvGlobalName            aEndName;
    SvGlobalName            aNextName;
protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteContextSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
#endif
public:
                        SV_DECL_META_FACTORY1( SvMetaModule, SvMetaExtern, 13 )
                        SvMetaModule();

    const String &      GetIdlFileName() const { return aIdlFileName; }
    const ByteString &      GetModulePrefix() const { return aModulePrefix; }

    virtual sal_Bool        SetName( const ByteString & rName, SvIdlDataBase * = NULL  );

    const ByteString &      GetHelpFileName() const { return aHelpFileName; }
    const ByteString &      GetTypeLibFileName() const { return aTypeLibFile; }

    const SvMetaAttributeMemberList & GetAttrList() const { return aAttrList; }
    const SvMetaTypeMemberList & GetTypeList() const { return aTypeList; }
    const SvMetaClassMemberList & GetClassList() const { return aClassList; }

#ifdef IDL_COMPILER
                        SvMetaModule( const String & rIdlFileName,
                                      sal_Bool bImported );

    sal_Bool                FillNextName( SvGlobalName * );
    sal_Bool                IsImported() const { return bImported; }
    sal_Bool                IsModified() const { return bIsModified; }

    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    virtual void        WriteAttributes( SvIdlDataBase & rBase,
                                        SvStream & rOutStm, sal_uInt16 nTab,
                                            WriteType, WriteAttribute = 0 );
//    virtual void        WriteSbx( SvIdlDataBase & rBase, SvStream & rOutStm, SvNamePosList & rList );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                    WriteType, WriteAttribute = 0 );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                    Table* pTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      Table *pIdTable );

    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
#endif
};
SV_DECL_IMPL_REF(SvMetaModule)
SV_DECL_IMPL_PERSIST_LIST(SvMetaModule,SvMetaModule *)


#endif // _MODULE_HXX

