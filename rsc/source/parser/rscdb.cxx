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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tools/rc.h>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <sal/macros.h>

#include <rsctree.hxx>
#include <rsctop.hxx>
#include <rscmgr.hxx>
#include <rscdb.hxx>
#include <rscrsc.hxx>


RscTypCont::RscTypCont( RscError * pErrHdl,
                          RSCBYTEORDER_TYPE nOrder,
                          const OString& rSearchPath,
                          CommandFlags nFlagsP )
    : nSourceCharSet( RTL_TEXTENCODING_UTF8 )
    , nByteOrder( nOrder )
    , aSearchPath( rSearchPath )
    , nUniqueId(256)
    , nFilePos( 0 )
    , nPMId(RSC_VERSIONCONTROL +1) // at least one more
    , aBool( pHS->getID( "sal_Bool" ), RSC_NOTYPE )
    , aShort( pHS->getID( "short" ), RSC_NOTYPE )
    , aUShort( pHS->getID( "sal_uInt16" ), RSC_NOTYPE )
    , aEnumLong( pHS->getID( "enum_long" ), RSC_NOTYPE )
    , aIdNoZeroUShort( pHS->getID( "IDUSHORT" ), RSC_NOTYPE )
    , aString( pHS->getID( "Chars" ), RSC_NOTYPE )
    , aStringLiteral( pHS->getID( "Chars" ), RSC_NOTYPE )
    , aLangType()
    , aLangString( pHS->getID( "Lang_Chars" ), RSC_NOTYPE, &aString, &aLangType )
    , pEH(pErrHdl)
    , nFlags( nFlagsP )
{
    Init();
}

OString RscTypCont::ChangeLanguage(const OString& rNewLang)
{
    OString aRet = aLanguage;
    aLanguage = rNewLang;

    ::std::vector< OUString > aFallbacks;

    if (rNewLang.isEmpty())
        aFallbacks.push_back( "" );     // do not resolve to SYSTEM (en-US)
    else
        aFallbacks = LanguageTag( OStringToOUString( rNewLang, RTL_TEXTENCODING_ASCII_US)).getFallbackStrings( true);

    bool bAppendEnUsFallback = ! (rNewLang.equalsIgnoreAsciiCase( "en-US" ) ||
                                  rNewLang.equalsIgnoreAsciiCase( "x-no-translate" ) );
    if (bAppendEnUsFallback)
        aFallbacks.push_back( "en-US");

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "RscTypCont::ChangeLanguage: " );
#endif

    aLangFallbacks.clear();

    for (OUString& rItem : aFallbacks)
    {
        OString aLang(OUStringToOString(rItem, RTL_TEXTENCODING_ASCII_US));
        sal_uInt32 nID = GetLangId( aLang );
        bool bAdd = (nID == 0);
        if ( bAdd )
        {
            AddLanguage( aLang.getStr() );
            nID = GetLangId( aLang );
        }
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, " '%s' (0x%hx) (%s)", aLang.getStr(), (int)nID, (bAdd ? "added" : "exists") );
#endif
        aLangFallbacks.push_back( nID);
    }

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "\n" );
#endif

    return aRet;
}

Atom RscTypCont::AddLanguage( const char* pLang )
{
    return aLangType.AddLanguage( pLang, aNmTb );
}


void DestroyNode( RscTop * pRscTop, ObjNode * pObjNode )
{
    if( pObjNode )
    {
        DestroyNode( pRscTop, static_cast<ObjNode*>(pObjNode->Left()) );
        DestroyNode( pRscTop, static_cast<ObjNode*>(pObjNode->Right()) );

        if( pObjNode->GetRscObj() )
        {
            pRscTop->Destroy( RSCINST( pRscTop, pObjNode->GetRscObj() ) );
            rtl_freeMemory( pObjNode->GetRscObj() );
        }
        delete pObjNode;
    }
}

void DestroySubTrees( RscTop * pRscTop )
{
    if( pRscTop )
    {
        DestroySubTrees( static_cast<RscTop*>(pRscTop->Left()) );
        DestroyNode( pRscTop, pRscTop->GetObjNode() );
        DestroySubTrees( static_cast<RscTop*>(pRscTop->Right()) );
    }
}

void DestroyTree( RscTop * pRscTop )
{
    if( pRscTop )
    {
        DestroyTree( static_cast<RscTop*>(pRscTop->Left()) );
        DestroyTree( static_cast<RscTop*>(pRscTop->Right()) );

        delete pRscTop;
    }
}

void Pre_dtorTree( RscTop * pRscTop )
{
    if( pRscTop )
    {
        Pre_dtorTree( static_cast<RscTop*>(pRscTop->Left()) );
        Pre_dtorTree( static_cast<RscTop*>(pRscTop->Right()) );

        pRscTop->Pre_dtor();
    }
}

RscTypCont::~RscTypCont()
{
    // delete all subtrees
    aVersion.pClass->Destroy( aVersion );
    rtl_freeMemory( aVersion.pData );
    DestroySubTrees( pRoot );

    // all classes are still valid, destroy each instance
    // of base types
    for (RscTop* pItem : aBaseLst)
        pItem->Pre_dtor();

    aBool.Pre_dtor();
    aShort.Pre_dtor();
    aUShort.Pre_dtor();
    aIdNoZeroUShort.Pre_dtor();
    aString.Pre_dtor();
    aVersion.pClass->Pre_dtor();
    // sub-types
    Pre_dtorTree( pRoot );

    // destroy classes
    delete aVersion.pClass;
    DestroyTree( pRoot );

    for (RscTop* pItem : aBaseLst)
        delete pItem;

    for (RscSysEntry* pItem: aSysLst)
        delete pItem;
}

void RscTypCont::ClearSysNames()
{
    for (RscSysEntry* pItem: aSysLst)
        delete pItem;

    aSysLst.clear();
}

sal_uInt32 RscTypCont::PutSysName( sal_uInt32 nRscTyp, char * pFileName )
{
    RscSysEntry *pFoundEntry = nullptr;

    for (RscSysEntry* pItem: aSysLst)
    {
        if( !strcmp( pItem->aFileName.getStr(), pFileName ) &&
            pItem->nRscTyp == nRscTyp &&
            pItem->nTyp    == 0 &&
            pItem->nRefId  == 0)
        {
            pFoundEntry = pItem;
            break;
        }
    }
    RscSysEntry *pSysEntry = pFoundEntry;

    if ( !pSysEntry )
    {
        pSysEntry = new RscSysEntry;
        pSysEntry->nKey = nUniqueId++;
        pSysEntry->nRscTyp = nRscTyp;
        pSysEntry->nTyp = 0;
        pSysEntry->nRefId = 0;
        pSysEntry->aFileName = pFileName;
        aSysLst.push_back( pSysEntry );
    }

    return pSysEntry->nKey;
}


class RscEnumerateObj
{
friend class RscEnumerateRef;
private:
    ERRTYPE     aError;     // contains the first field
    RscTypCont* pTypCont;
    FILE *      fOutput;    // output file
    RscFileTab::Index lFileKey;   // what source file
    RscTop *    pClass;

    RscEnumerateObj(RscTypCont* pTC, FILE* pOutputFile)
        :pTypCont(pTC)
        ,fOutput(pOutputFile)
        ,lFileKey(0)
        ,pClass(nullptr)
    {
    }

    DECL_LINK( CallBackWriteRc, const NameNode&, void );
    DECL_LINK( CallBackWriteSrc, const NameNode&, void );

    void WriteRc( RscTop * pCl, ObjNode * pRoot )
    {
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteRc ) );
    }
    void WriteSrc( RscTop * pCl, ObjNode * pRoot ){
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteSrc ) );
    }
public:
    void WriteRcFile( RscWriteRc & rMem, FILE * fOutput );
};

IMPL_LINK( RscEnumerateObj, CallBackWriteRc, const NameNode&, rNode, void )
{
    const ObjNode& rObjNode = static_cast<const ObjNode&>(rNode);
    RscWriteRc     aMem( pTypCont->GetByteOrder() );

    aError = pClass->WriteRcHeader( RSCINST( pClass, rObjNode.GetRscObj() ),
                                     aMem, pTypCont,
                                     rObjNode.GetRscId(), 0, true );
    if( aError.IsError() || aError.IsWarning() )
        pTypCont->pEH->Error( aError, pClass, rObjNode.GetRscId() );

    WriteRcFile( aMem, fOutput );
}

IMPL_LINK( RscEnumerateObj, CallBackWriteSrc, const NameNode&, rNode, void )
{
    const ObjNode& rObjNode = static_cast<const ObjNode&>(rNode);
    if( rObjNode.GetFileKey() == lFileKey )
    {
        pClass->WriteSrcHeader( RSCINST( pClass, rObjNode.GetRscObj() ),
                                fOutput, pTypCont, 0,
                                rObjNode.GetRscId(), "" );
        fprintf( fOutput, ";\n" );
    }
}

void RscEnumerateObj::WriteRcFile( RscWriteRc & rMem, FILE * fOut )
{
    // structure definition from which the resource is built
    /*
    struct RSHEADER_TYPE{
        sal_uInt32          nId;        // resource identifier
        sal_uInt32          nRT;        // resource type
        sal_uInt32          nGlobOff;   // global offset
        sal_uInt32          nLocalOff;  // local offset
    } aHeader;
    */

    sal_uInt32 nId = rMem.GetLong( 0 );
    sal_uInt32 nRT = rMem.GetLong( 4 );

    // table is filled with nId and nRT
    pTypCont->PutTranslatorKey( (sal_uInt64(nRT) << 32) + sal_uInt64(nId) );

    if( nRT == RSC_VERSIONCONTROL )
    { // always comes last
        sal_Int32 nCount = pTypCont->aIdTranslator.size();
        // table size
        sal_uInt32 nSize = (nCount * (sizeof(sal_uInt64)+sizeof(sal_Int32))) + sizeof(sal_Int32);

        rMem.Put( nCount ); // save the count
        for (auto& rItem : pTypCont->aIdTranslator)
        {
            // save the key
            rMem.Put( rItem.first );
            // save the object id or position
            rMem.Put( static_cast<sal_Int32>(rItem.second) );
        }
        rMem.Put( nSize ); // save the size next
    }

    // reset the file offset
    pTypCont->IncFilePos( rMem.Size() );


    // position was written previously in the table
    bool bSuccess = (1 == fwrite( rMem.GetBuffer(), rMem.Size(), 1, fOut ));
    SAL_WARN_IF(!bSuccess, "rsc", "short write");
};

class RscEnumerateRef
{
private:
    RscTop *        pRoot;

    DECL_LINK( CallBackWriteRc, const NameNode&, void );
    DECL_LINK( CallBackWriteSrc, const NameNode&, void );
public:
    RscEnumerateObj aEnumObj;

    RscEnumerateRef(RscTypCont* pTC, RscTop* pR, FILE* fOutput)
        : pRoot(pR), aEnumObj(pTC, fOutput)
        {
        }
    ERRTYPE const & WriteRc()
        {
            aEnumObj.aError.Clear();
            pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteRc ) );
            return aEnumObj.aError;
        }
    ERRTYPE const & WriteSrc( RscFileTab::Index lFileKey )
        {
            aEnumObj.lFileKey = lFileKey;

            aEnumObj.aError.Clear();
            pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteSrc ) );
            return aEnumObj.aError;
        }
};

IMPL_LINK( RscEnumerateRef, CallBackWriteRc, const NameNode&, rNode, void )
{
    const RscTop& rRef = static_cast<const RscTop&>(rNode);
    aEnumObj.WriteRc( const_cast<RscTop*>(&rRef), rRef.GetObjNode() );
}

IMPL_LINK( RscEnumerateRef, CallBackWriteSrc, const NameNode&, rNode, void )
{
    const RscTop& rRef = static_cast<const RscTop&>(rNode);
    aEnumObj.WriteSrc( const_cast<RscTop*>(&rRef), rRef.GetObjNode() );
}

ERRTYPE RscTypCont::WriteRc( WriteRcContext& rContext )
{
    ERRTYPE       aError;
    RscEnumerateRef aEnumRef( this, pRoot, rContext.fOutput );

    aIdTranslator.clear();
    nFilePos = 0;
    nPMId = RSCVERSION_ID +1; // at least one more

    aError = aEnumRef.WriteRc();

    // version control
    RscWriteRc aMem( nByteOrder );
    aVersion.pClass->WriteRcHeader( aVersion, aMem, this, RscId( RSCVERSION_ID ), 0, true );
    aEnumRef.aEnumObj.WriteRcFile( aMem, rContext.fOutput );

    return aError;
}

void RscTypCont::WriteSrc( FILE * fOutput, RscFileTab::Index nFileKey )
{
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );

    unsigned char aUTF8BOM[3] = { 0xef, 0xbb, 0xbf };
    size_t nItems = SAL_N_ELEMENTS(aUTF8BOM);
    bool bSuccess = (nItems == fwrite(aUTF8BOM, 1, nItems, fOutput));
    SAL_WARN_IF(!bSuccess, "rsc", "short write");
    RscId::SetNames( false );
    if( nFileKey == RscFileTab::IndexNotFound )
    {
        RscFileTab::Index aIndex = aFileTab.FirstIndex();
        while( aIndex != RscFileTab::IndexNotFound )
        {
            aEnumRef.WriteSrc( aIndex );
            aIndex = aFileTab.NextIndex( aIndex );
        };
    }
    else
         aEnumRef.WriteSrc( nFileKey );
    RscId::SetNames();
}

class RscDel
{
    RscFileTab::Index lFileKey;
    DECL_LINK( Delete, const NameNode&, void );
public:
    RscDel( RscTop * pRoot, RscFileTab::Index lKey );
};


inline RscDel::RscDel( RscTop * pRoot, RscFileTab::Index lKey )
    : lFileKey(lKey)
{
    pRoot->EnumNodes( LINK( this, RscDel, Delete ) );
}

IMPL_LINK( RscDel, Delete, const NameNode&, r, void )
{
    RscTop* pNode = const_cast<RscTop*>(static_cast<const RscTop*>(&r));
    if( pNode->GetObjNode() )
        pNode->pObjBiTree = pNode->GetObjNode()->DelObjNode( pNode, lFileKey );
}

void RscTypCont::Delete( RscFileTab::Index lFileKey )
{
    // delete resource instance
    RscDel aDel( pRoot, lFileKey );
    // delete defines
    aFileTab.DeleteFileContext( lFileKey );
}

bool IsInstConsistent( ObjNode * pObjNode, RscTop * pRscTop )
{
    bool bRet = true;

    if( pObjNode )
    {
        RSCINST aTmpI;

        if( ! IsInstConsistent( static_cast<ObjNode*>(pObjNode->Left()), pRscTop ) )
            bRet = false;

        aTmpI.pClass = pRscTop;
        aTmpI.pData = pObjNode->GetRscObj();
        if( ! aTmpI.pClass->IsConsistent( aTmpI ) )
            bRet = false;

        if( ! IsInstConsistent( static_cast<ObjNode*>(pObjNode->Right()), pRscTop ) )
            bRet = false;
    }

    return bRet;
}

bool MakeConsistent( RscTop * pRscTop )
{
    bool bRet = true;

    if( pRscTop )
    {
        if( ! ::MakeConsistent( static_cast<RscTop*>(pRscTop->Left()) ) )
            bRet = false;

        if( pRscTop->GetObjNode() )
        {
            if( ! pRscTop->GetObjNode()->IsConsistent() )
            {
                pRscTop->GetObjNode()->OrderTree();
                if( ! pRscTop->GetObjNode()->IsConsistent() )
                    bRet = false;
            }
            if( ! IsInstConsistent( pRscTop->GetObjNode(), pRscTop ) )
                bRet = false;
        }

        if( ! ::MakeConsistent( static_cast<RscTop*>(pRscTop->Right()) ) )
            bRet = false;
    }

    return bRet;
}

void RscTypCont::PutTranslatorKey( sal_uInt64 nKey )
{
    aIdTranslator[ nKey ] = nFilePos;
    nPMId++;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
