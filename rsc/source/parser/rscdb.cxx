/*************************************************************************
 *
 *  $RCSfile: rscdb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/parser/rscdb.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

*************************************************************************/
/****************** I N C L U D E S **************************************/

#pragma hdrstop

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tools/intn.hxx>
#include <tools/fsys.hxx>
#include <tools/rc.h>

// Programmabhaengige Includes.
#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
#ifndef _RSCMGR_HXX
#include <rscmgr.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif

/*************************************************************************
|*
|*    RscTypCont :: RscTypCont
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
RscTypCont :: RscTypCont( RscError * pErrHdl,
                          LanguageType    nLangType,
                          RSCBYTEORDER_TYPE nOrder,
                          rtl_TextEncoding nSourceCharSet_,
                          const ByteString & rSearchPath,
                          USHORT nFlagsP )
    : nLangTypeId( nLangType ),
      nDfltLangTypeId( International::GetNeutralLanguage( nLangType ) ),
      nByteOrder( nOrder ),
      nSourceCharSet( nSourceCharSet_ ),
      aSearchPath( rSearchPath ),
      nFlags( nFlagsP ),
      aBool( pHS->Insert( "BOOL" ), RSC_NOTYPE ),
      aShort( pHS->Insert( "short" ), RSC_NOTYPE ),
      aUShort( pHS->Insert( "USHORT" ), RSC_NOTYPE ),
      aLong( pHS->Insert( "long" ), RSC_NOTYPE ),
      aEnumLong( pHS->Insert( "enum_long" ), RSC_NOTYPE ),
      aIdUShort( pHS->Insert( "IDUSHORT" ), RSC_NOTYPE ),
      aIdNoZeroUShort( pHS->Insert( "IDUSHORT" ), RSC_NOTYPE ),
      aNoZeroShort( pHS->Insert( "NoZeroShort" ), RSC_NOTYPE ),
      a1to12Short( pHS->Insert( "MonthShort" ), RSC_NOTYPE ),
      a0to23Short( pHS->Insert( "HourShort" ), RSC_NOTYPE ),
      a1to31Short( pHS->Insert( "DayShort" ), RSC_NOTYPE ),
      a0to59Short( pHS->Insert( "MinuteShort" ), RSC_NOTYPE ),
      a0to99Short( pHS->Insert( "_0to59Short" ), RSC_NOTYPE ),
      a0to9999Short( pHS->Insert( "YearShort" ), RSC_NOTYPE ),
      aIdLong( pHS->Insert( "IDLONG" ), RSC_NOTYPE, TRUE ),
      aString( pHS->Insert( "Chars" ), RSC_NOTYPE ),
      aWinBits( pHS->Insert( "WinBits" ), RSC_NOTYPE, FALSE ),
      aLangType( pHS->Insert( "LangEnum" ), RSC_NOTYPE ),
      aLangString( pHS->Insert( "Lang_Chars" ), RSC_NOTYPE, &aString,
                    &aLangType, &nLangTypeId, &nDfltLangTypeId ),
      aLangShort( pHS->Insert( "Lang_short" ), RSC_NOTYPE, &aShort,
                    &aLangType, &nLangTypeId, &nDfltLangTypeId )
{
    nUniqueId = 256;
    nPMId = RSC_VERSIONCONTROL +1; //mindestens einen groesser
    pEH = pErrHdl;
    Init();
}

/*************************************************************************
|*
|*    RscTypCont :: ~RscTypCont
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
void DestroyNode( RscTop * pRscTop, ObjNode * pObjNode ){
    if( pObjNode ){
        DestroyNode( pRscTop, (ObjNode*)pObjNode->Left() );
        DestroyNode( pRscTop, (ObjNode*)pObjNode->Right() );

        if( pObjNode->GetRscObj() ){
            pRscTop->Destroy( RSCINST( pRscTop, pObjNode->GetRscObj() ) );
            RscMem::Free( pObjNode->GetRscObj() );
        }
        delete pObjNode;
    };
}

void DestroySubTrees( RscTop * pRscTop ){
    if( pRscTop ){
        DestroySubTrees( (RscTop*)pRscTop->Left() );

        DestroyNode( pRscTop, pRscTop->GetObjNode() );

        DestroySubTrees( (RscTop*)pRscTop->Right() );
    };
}

void DestroyTree( RscTop * pRscTop ){
    if( pRscTop ){
        DestroyTree( (RscTop*)pRscTop->Left() );
        DestroyTree( (RscTop*)pRscTop->Right() );

        delete pRscTop;
    };
}

void Pre_dtorTree( RscTop * pRscTop ){
    if( pRscTop ){
        Pre_dtorTree( (RscTop*)pRscTop->Left() );
        Pre_dtorTree( (RscTop*)pRscTop->Right() );

        pRscTop->Pre_dtor();
    };
}

RscTypCont :: ~RscTypCont(){
    RscTop  *       pRscTmp;
    RscSysEntry   * pSysEntry;

    // Alle Unterbaeume loeschen
    aVersion.pClass->Destroy( aVersion );
    RscMem::Free( aVersion.pData );
    DestroySubTrees( pRoot );

    // Alle Klassen noch gueltig, jeweilige Instanzen freigeben
    // BasisTypen
    pRscTmp = aBaseLst.First();
    while( pRscTmp ){
        pRscTmp->Pre_dtor();
        pRscTmp = aBaseLst.Next();
    };
    aBool.Pre_dtor();
    aShort.Pre_dtor();
    aUShort.Pre_dtor();
    aIdUShort.Pre_dtor();
    aIdNoZeroUShort.Pre_dtor();
    aNoZeroShort.Pre_dtor();
    aIdLong.Pre_dtor();
    aString.Pre_dtor();
    aWinBits.Pre_dtor();
    aVersion.pClass->Pre_dtor();
    // Zusammengesetzte Typen
    Pre_dtorTree( pRoot );

    // Klassen zerstoeren
    delete aVersion.pClass;
    DestroyTree( pRoot );

    while( NULL != (pRscTmp = aBaseLst.Remove()) ){
        delete pRscTmp;
    };

    while( NULL != (pSysEntry = aSysLst.Remove()) ){
        delete pSysEntry;
    };
}

//=======================================================================
RscTop * RscTypCont::SearchType( HASHID nId )
/*  [Beschreibung]

    Sucht eine Basistyp nId;
*/
{
    if( nId == HASH_NONAME )
        return NULL;

#define ELSE_IF( a )                \
    else if( a##.GetId() == nId )   \
        return &a;                  \

    if( aBool.GetId() == nId )
        return &aBool;
    ELSE_IF( aShort )
    ELSE_IF( aUShort )
    ELSE_IF( aLong )
    ELSE_IF( aEnumLong )
    ELSE_IF( aIdUShort )
    ELSE_IF( aIdNoZeroUShort )
    ELSE_IF( aNoZeroShort )
    ELSE_IF( a1to12Short )
    ELSE_IF( a0to23Short )
    ELSE_IF( a1to31Short )
    ELSE_IF( a0to59Short )
    ELSE_IF( a0to99Short )
    ELSE_IF( a0to9999Short )
    ELSE_IF( aIdLong )
    ELSE_IF( aString )
    ELSE_IF( aWinBits )
    ELSE_IF( aLangType )
    ELSE_IF( aLangString )
    ELSE_IF( aLangShort )

    RscTop * pEle = aBaseLst.First();
    while( pEle )
    {
        if( pEle->GetId() == nId )
            return pEle;
        pEle = aBaseLst.Next();
    }
    return NULL;
}

/*************************************************************************
|*
|*    RscTypCont :: Search
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
RscTop * RscTypCont :: Search( HASHID nRT ){
    return( (RscTop *)pRoot->Search( nRT ) );
}

CLASS_DATA RscTypCont :: Search( HASHID nRT, const RscId & rId ){
    ObjNode *pObjNode;
    RscTop  *pRscTop;

    if( NULL != (pRscTop = Search( nRT )) ){
        if( NULL != (pObjNode = pRscTop->GetObjNode( rId )) ){
            return( pObjNode->GetRscObj() );
        }
    }
    return( (CLASS_DATA)0 );
}

/*************************************************************************
|*
|*    RscTypCont :: Delete()
|*
|*    Beschreibung
|*    Ersterstellung    MM 10.07.91
|*    Letzte Aenderung  MM 10.07.91
|*
*************************************************************************/
void RscTypCont :: Delete( HASHID nRT, const RscId & rId ){
    ObjNode *   pObjNode;
    RscTop  *   pRscTop;

    if( NULL != (pRscTop = Search( nRT )) ){
        if( NULL != (pObjNode = pRscTop->GetObjNode()) ){
            pObjNode = pObjNode->Search( rId );

            if( pObjNode ){
                //Objekt aus Baum entfernen
                pRscTop->pObjBiTree =
                    (ObjNode *)pRscTop->pObjBiTree->Remove( pObjNode );

                if( pObjNode->GetRscObj() ){
                    pRscTop->Destroy( RSCINST( pRscTop,
                                               pObjNode->GetRscObj() ) );
                    RscMem::Free( pObjNode->GetRscObj() );
                }
                delete pObjNode;
            }
        }
    }
}

/*************************************************************************
|*
|*    RscTypCont :: PutSysName()
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
USHORT RscTypCont :: PutSysName( USHORT nRscTyp, char * pFileName,
                                 USHORT nConst, USHORT nId, BOOL bFirst )
{
    RscSysEntry *   pSysEntry;
    BOOL            bId1 = FALSE;

    pSysEntry = aSysLst.First();
    while( pSysEntry )
    {
        if( pSysEntry->nKey == 1 )
            bId1 = TRUE;
        if( !strcmp( pSysEntry->aFileName.GetBuffer(), pFileName ) )
            if( pSysEntry->nRscTyp == nRscTyp
              && pSysEntry->nTyp == nConst
              && pSysEntry->nRefId == nId )
                break;
        pSysEntry = aSysLst.Next();
    }

    if ( !pSysEntry || (bFirst && !bId1) )
    {
        pSysEntry = new RscSysEntry;
        pSysEntry->nKey = nUniqueId++;
        pSysEntry->nRscTyp = nRscTyp;
        pSysEntry->nTyp = nConst;
        pSysEntry->nRefId = nId;
        pSysEntry->aFileName = (const char*)pFileName;
        if( bFirst && !bId1 )
        {
            pSysEntry->nKey = 1;
            aSysLst.Insert( pSysEntry, (ULONG)0 );
        }
        else
            aSysLst.Insert( pSysEntry, LIST_APPEND );
    }

    return pSysEntry->nKey;
}

/*************************************************************************
|*
|*    RscTypCont :: WriteInc
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 21.06.90
|*    Letzte Aenderung  MM 21.06.90
|*
*************************************************************************/
void RscTypCont :: WriteInc( FILE * fOutput, ULONG lFileKey )
{
    RscFile   * pFName;

    if( NOFILE_INDEX == lFileKey )
    {
        pFName = aFileTab.First();
        while( pFName )
        {
            if( pFName && pFName->IsIncFile() )
            {
                fprintf( fOutput, "#include " );
                fprintf( fOutput, "\"%s\"\n",
                                  pFName->aFileName.GetBuffer() );
            }
            pFName = aFileTab.Next();
        }
    }
    else
    {
        RscDepend *     pDep;
        RscFile   *     pFile;

        pFName = aFileTab.Get( lFileKey );
        if( pFName )
        {
            pDep = pFName->First();
            while( pDep )
            {
                if( pDep->GetFileKey() != lFileKey )
                {
                    pFile = aFileTab.GetFile( pDep->GetFileKey() );
                    if( pFile )
                    {
                        fprintf( fOutput, "#include " );
                        fprintf( fOutput, "\"%s\"\n",
                                 pFile->aFileName.GetBuffer() );
                    }
                }
                pDep = pFName->Next();
            };
        };
    };
}

/*************************************************************************
|*
|*    RscTypCont :: Methoden die ueber all Knoten laufen
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/

class RscEnumerateObj
{
friend class RscEnumerateRef;
private:
    ERRTYPE     aError;     // Enthaelt den ersten Fehler
    RscTypCont* pTypCont;
    FILE *      fOutput;    // AusgabeDatei
    ULONG       lFileKey;   // Welche src-Datei
    RscTop *    pClass;

    DECL_LINK( CallBackWriteRc, ObjNode * );
    DECL_LINK( CallBackWriteSrc, ObjNode * );
    DECL_LINK( CallBackWriteCxx, ObjNode * );
    DECL_LINK( CallBackWriteHxx, ObjNode * );

    ERRTYPE WriteRc( RscTop * pCl, ObjNode * pRoot )
    {
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteRc ) );
        return aError;
    }
    ERRTYPE WriteSrc( RscTop * pCl, ObjNode * pRoot ){
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteSrc ) );
        return aError;
    }
    ERRTYPE WriteCxx( RscTop * pCl, ObjNode * pRoot ){
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteCxx ) );
        return aError;
    }
    ERRTYPE WriteHxx( RscTop * pCl, ObjNode * pRoot ){
        pClass = pCl;
        if( pRoot )
            pRoot->EnumNodes( LINK( this, RscEnumerateObj, CallBackWriteHxx ) );
        return aError;
    }
public:
    void WriteRcFile( RscWriteRc & rMem, FILE * fOutput );
};

/*************************************************************************
|*
|*    RscEnumerateObj :: CallBackWriteRc
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
IMPL_LINK( RscEnumerateObj, CallBackWriteRc, ObjNode *, pObjNode )
{
    RscWriteRc      aMem( pTypCont->GetByteOrder() );

    aError = pClass->WriteRcHeader( RSCINST( pClass, pObjNode->GetRscObj() ),
                                     aMem, pTypCont,
                                     pObjNode->GetRscId(), 0, TRUE );
    if( aError.IsError() || aError.IsWarning() )
        pTypCont->pEH->Error( aError, pClass, pObjNode->GetRscId() );

    WriteRcFile( aMem, fOutput );
    return 0;
}

/*************************************************************************
|*
|*    RscEnumerateObj :: CallBackWriteSrc
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
IMPL_LINK_INLINE_START( RscEnumerateObj, CallBackWriteSrc, ObjNode *, pObjNode )
{
    if( pObjNode->GetFileKey() == lFileKey ){
        pClass->WriteSrcHeader( RSCINST( pClass, pObjNode->GetRscObj() ),
                                fOutput, pTypCont, 0,
                                pObjNode->GetRscId(), "" );
        fprintf( fOutput, ";\n" );
    }
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateObj, CallBackWriteSrc, ObjNode *, pObjNode )

/*************************************************************************
|*
|*    RscEnumerateObj :: CallBackWriteCxx
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
IMPL_LINK_INLINE_START( RscEnumerateObj, CallBackWriteCxx, ObjNode *, pObjNode )
{
    if( pClass->IsCodeWriteable() && pObjNode->GetFileKey() == lFileKey )
        aError = pClass->WriteCxxHeader(
                              RSCINST( pClass, pObjNode->GetRscObj() ),
                              fOutput, pTypCont, pObjNode->GetRscId() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateObj, CallBackWriteCxx, ObjNode *, pObjNode )

/*************************************************************************
|*
|*    RscEnumerateObj :: CallBackWriteHxx
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
IMPL_LINK_INLINE_START( RscEnumerateObj, CallBackWriteHxx, ObjNode *, pObjNode )
{
    if( pClass->IsCodeWriteable() && pObjNode->GetFileKey() == lFileKey )
        aError = pClass->WriteHxxHeader(
                              RSCINST( pClass, pObjNode->GetRscObj() ),
                              fOutput, pTypCont, pObjNode->GetRscId() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateObj, CallBackWriteHxx, ObjNode *, pObjNode )

/*************************************************************************
|*
|*    RscEnumerateObj :: WriteRcFile
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
ULONG WritePMRes( FILE * fOutput, USHORT nType, USHORT nId, UINT32 nLen )
{
    fputc( (char)0xFF, fOutput );
    fwrite( (const char *)&nType, sizeof( nType ), 1, fOutput );
    fputc( (char)0xFF, fOutput );
    fwrite( (const char *)&nId, sizeof( nId ), 1, fOutput );
    USHORT fsOption = 0x0030; // Aus Resourcefile ermittelt
    fwrite( (const char *)&fsOption, sizeof( fsOption ), 1, fOutput );
    // !!! Achtung: Groesse der Resource NICHT dazuaddieren
    // !!! nLen += 12; // groesse des Headers
    ULONG nFilePos = ftell( fOutput );
    fwrite( (const char *)&nLen, sizeof( nLen ), 1, fOutput );
    return nFilePos;
}

void RscEnumerateObj :: WriteRcFile( RscWriteRc & rMem, FILE * fOut ){
    // Definition der Struktur, aus denen die Resource aufgebaut ist
    /*
    struct RSHEADER_TYPE{
        USHORT          nId;        // Identifier der Resource
        USHORT          nRT;        // Resource Typ
        USHORT          nGlobOff;   // Globaler Offset
        USHORT          nLocalOff;  // Lokaler Offset
    } aHeader;
    */

    USHORT nId = rMem.GetShort( 0 );
    USHORT nRT = rMem.GetShort( 2 );

    // Tabelle wird entsprechend gefuellt
    pTypCont->PutTranslatorKey( ((INT32)nRT << 16) + nId );

    if( nRT == RSC_VERSIONCONTROL )
    { // kommt immmer als letztes
        INT32 nCount = pTypCont->aIdTranslator.Count();
        // groesse der Tabelle
        UINT32 nSize = (nCount * 2 + 1) * sizeof( INT32 );

        rMem.Put( nCount ); //Anzahl speichern
        INT32   l = (INT32)pTypCont->aIdTranslator.First();
        while( nCount )
        {
            // Schluessel schreiben
            l = pTypCont->aIdTranslator.GetCurKey();
            rMem.Put( l );
            // Objekt Id oder Position schreiben
            l = (INT32)pTypCont->aIdTranslator.GetCurObject();
            rMem.Put( l );
            l = (INT32)pTypCont->aIdTranslator.Next();
            nCount--;
        }
        rMem.Put( nSize ); // Groesse hinten Speichern
    }

    //Dateioffset neu setzen
    pTypCont->IncFilePos( rMem.Size() );


    //Position wurde vorher in Tabelle geschrieben
    fwrite( rMem.GetBuffer(), rMem.Size(), 1, fOut );

};

class RscEnumerateRef
{
private:
    RscTop *        pRoot;

    DECL_LINK( CallBackWriteRc, RscTop * );
    DECL_LINK( CallBackWriteSrc, RscTop * );
    DECL_LINK( CallBackWriteCxx, RscTop * );
    DECL_LINK( CallBackWriteHxx, RscTop * );
    DECL_LINK( CallBackWriteSyntax, RscTop * );
    DECL_LINK( CallBackWriteRcCtor, RscTop * );
public:
    RscEnumerateObj aEnumObj;

            RscEnumerateRef( RscTypCont * pTC, RscTop * pR,
                             FILE * fOutput )
            {
                aEnumObj.pTypCont = pTC;
                aEnumObj.fOutput  = fOutput;
                pRoot             = pR;
            }
    ERRTYPE WriteRc()
    {
        aEnumObj.aError.Clear();
        pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteRc ) );
        return aEnumObj.aError;
    };

    ERRTYPE WriteSrc( ULONG lFileKey )
    {
        aEnumObj.lFileKey = lFileKey;

        aEnumObj.aError.Clear();
        pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteSrc ) );
        return aEnumObj.aError;
    }

    ERRTYPE WriteCxx( ULONG lFileKey )
    {
        aEnumObj.lFileKey = lFileKey;

        aEnumObj.aError.Clear();
        pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteCxx ) );
        return aEnumObj.aError;
    }

    ERRTYPE WriteHxx(  ULONG lFileKey )
    {
        aEnumObj.lFileKey = lFileKey;

        aEnumObj.aError.Clear();
        pRoot->EnumNodes( LINK( this, RscEnumerateRef, CallBackWriteHxx ) );
        return aEnumObj.aError;
    }

    void    WriteSyntax()
            {
                pRoot->EnumNodes( LINK( this, RscEnumerateRef,
                                        CallBackWriteSyntax ) );
            }

    void    WriteRcCtor()
            {
                pRoot->EnumNodes( LINK( this, RscEnumerateRef,
                                        CallBackWriteRcCtor ) );
            }
};

/*************************************************************************
|*
|*    RscRscEnumerateRef :: CallBack...
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteRc, RscTop *, pRef )
{
    aEnumObj.WriteRc( pRef, pRef->GetObjNode() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteRc, RscTop *, pRef )
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteSrc, RscTop *, pRef )
{
    aEnumObj.WriteSrc( pRef, pRef->GetObjNode() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteSrc, RscTop *, pRef )
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteCxx, RscTop *, pRef )
{
    if( pRef->IsCodeWriteable() )
        aEnumObj.WriteCxx( pRef, pRef->GetObjNode() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteCxx, RscTop *, pRef )
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteHxx, RscTop *, pRef )
{
    if( pRef->IsCodeWriteable() )
        aEnumObj.WriteHxx( pRef, pRef->GetObjNode() );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteHxx, RscTop *, pRef )
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteSyntax, RscTop *, pRef )
{
    pRef->WriteSyntaxHeader( aEnumObj.fOutput, aEnumObj.pTypCont );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteSyntax, RscTop *, pRef )
IMPL_LINK_INLINE_START( RscEnumerateRef, CallBackWriteRcCtor, RscTop *, pRef )
{
    pRef->WriteRcCtor( aEnumObj.fOutput, aEnumObj.pTypCont );
    return 0;
}
IMPL_LINK_INLINE_END( RscEnumerateRef, CallBackWriteRcCtor, RscTop *, pRef )

/*************************************************************************
|*
|*    RscTypCont :: WriteRc
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 22.07.91
|*
*************************************************************************/
ERRTYPE RscTypCont::WriteRc( FILE * fOutput )
{
    RscSysEntry * pSysEntry;
    ERRTYPE       aError;
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );

    aIdTranslator.Clear();
    nFilePos = 0;
    nPMId = RSCVERSION_ID +1; //mindestens einen groesser

    aError = aEnumRef.WriteRc();

    if( aError.IsOk() )
    {
        // Systemabhaengige Resourcen schreiben
        pSysEntry = aSysLst.First();
        while( pSysEntry && aError.IsOk() )
        {
            UniString aUniFileName( pSysEntry->aFileName, RTL_TEXTENCODING_ASCII_US );
            DirEntry aFullName( aUniFileName );
            aFullName.Find( UniString( GetSearchPath(), RTL_TEXTENCODING_ASCII_US ) );
            pSysEntry->aFileName = ByteString( aFullName.GetFull(), RTL_TEXTENCODING_ASCII_US );
            if( !::Append( fOutput, pSysEntry->aFileName.GetBuffer() ) )
            {
                pEH->FatalError( ERR_OPENFILE, RscId(), pSysEntry->aFileName.GetBuffer() );
                break;
            }
            UniString aUniFileName2( pSysEntry->aFileName, RTL_TEXTENCODING_ASCII_US );
            FileStat aFS( aUniFileName2 );
            if( !aFS.IsKind( FSYS_KIND_FILE ) )
            {
                pEH->FatalError( ERR_OPENFILE, RscId(), pSysEntry->aFileName.GetBuffer() );
                break;
            }

            // Tabelle wird entsprechend gefuellt
            PutTranslatorKey( ((ULONG)RT_SYS_BITMAP << 16) + pSysEntry->nKey );
            UINT32 nSize = aFS.GetSize();
            IncFilePos( nSize );

            // wegen Alignment
            nSize = sizeof( int ) - nSize % sizeof( int );
            nSize %= sizeof( int );
            IncFilePos( nSize );
            while( nSize-- )
                fputc( 0, fOutput );

            pSysEntry = aSysLst.Next();
        }

        // Versionskontrolle schreiben
        RscWriteRc  aMem( nByteOrder );
        aVersion.pClass->WriteRcHeader( aVersion, aMem, this,
                                        RscId( RSCVERSION_ID ), 0, TRUE );
        aEnumRef.aEnumObj.WriteRcFile( aMem, fOutput );
    }

    return( aError );
}

/*************************************************************************
|*
|*    RscTypCont :: WriteSrc
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 22.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
void RscTypCont :: WriteSrc( FILE * fOutput, ULONG nFileKey,
                             CharSet nCharSet, BOOL bName )
{
    RscFile     *   pFName;
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );

    if( bName )
    {
        WriteInc( fOutput, nFileKey );

        if( NOFILE_INDEX == nFileKey )
        {
            pFName = aFileTab.First();
            while( pFName  ){
                if( !pFName->IsIncFile() )
                    pFName->aDefLst.WriteAll( fOutput );
                aEnumRef.WriteSrc( aFileTab.GetIndex( pFName ) );
                pFName = aFileTab.Next();
            };
        }
        else
        {
            pFName = aFileTab.Get( nFileKey );
            if( pFName ){
                pFName->aDefLst.WriteAll( fOutput );
                aEnumRef.WriteSrc( nFileKey );
            }
        }
    }
    else
    {
        RscId::SetNames( FALSE );
        if( NOFILE_INDEX == nFileKey )
        {
            pFName = aFileTab.First();
            while( pFName  )
            {
                aEnumRef.WriteSrc( aFileTab.GetIndex( pFName ) );
                pFName = aFileTab.Next();
            };
        }
        else
             aEnumRef.WriteSrc( nFileKey );
        RscId::SetNames();
    };
}

/*************************************************************************
|*
|*    RscTypCont :: WriteHxx
|*
|*    Beschreibung
|*    Ersterstellung    MM 30.05.91
|*    Letzte Aenderung  MM 30.05.91
|*
*************************************************************************/
ERRTYPE RscTypCont :: WriteHxx( FILE * fOutput, ULONG nFileKey )
{
    fprintf( fOutput, "#include <tools/rc.hxx>\n" );
    fprintf( fOutput, "#include <tools/resid.hxx>\n" );
    fprintf( fOutput, "#include <vcl/accel.hxx>\n" );
    fprintf( fOutput, "#include <vcl/bitmap.hxx>\n" );
    fprintf( fOutput, "#include <vcl/button.hxx>\n" );
    fprintf( fOutput, "#include <vcl/color.hxx>\n" );
    fprintf( fOutput, "#include <vcl/combobox.hxx>\n" );
    fprintf( fOutput, "#include <vcl/ctrl.hxx>\n" );
    fprintf( fOutput, "#include <vcl/dialog.hxx>\n" );
    fprintf( fOutput, "#include <vcl/edit.hxx>\n" );
    fprintf( fOutput, "#include <vcl/field.hxx>\n" );
    fprintf( fOutput, "#include <vcl/fixed.hxx>\n" );
    fprintf( fOutput, "#include <vcl/group.hxx>\n" );
    fprintf( fOutput, "#include <vcl/image.hxx>\n" );
    fprintf( fOutput, "#include <vcl/imagebtn.hxx>\n" );
    fprintf( fOutput, "#include <vcl/keycod.hxx>\n" );
    fprintf( fOutput, "#include <vcl/lstbox.hxx>\n" );
    fprintf( fOutput, "#include <vcl/mapmod.hxx>\n" );
    fprintf( fOutput, "#include <vcl/menu.hxx>\n" );
    fprintf( fOutput, "#include <vcl/menubtn.hxx>\n" );
    fprintf( fOutput, "#include <vcl/morebtn.hxx>\n" );
    fprintf( fOutput, "#include <vcl/msgbox.hxx>\n" );
    fprintf( fOutput, "#include <vcl/scrbar.hxx>\n" );
    fprintf( fOutput, "#include <vcl/spin.hxx>\n" );
    fprintf( fOutput, "#include <vcl/spinfld.hxx>\n" );
    fprintf( fOutput, "#include <vcl/splitwin.hxx>\n" );
    fprintf( fOutput, "#include <vcl/status.hxx>\n" );
    fprintf( fOutput, "#include <vcl/tabctrl.hxx>\n" );
    fprintf( fOutput, "#include <vcl/tabdlg.hxx>\n" );
    fprintf( fOutput, "#include <vcl/tabpage.hxx>\n" );
    fprintf( fOutput, "#include <vcl/toolbox.hxx>\n" );
    fprintf( fOutput, "#include <vcl/window.hxx>\n" );
    fprintf( fOutput, "#include <vcl/wrkwin.hxx>\n" );
    fprintf( fOutput, "#include <svtools/svmedit.hxx>\n" );

    RscEnumerateRef aEnumRef( this, pRoot, fOutput );
    ERRTYPE         aError;

    if( NOFILE_INDEX == nFileKey )
    {
        RscFile     *   pFName;

        pFName = aFileTab.First();
        while( pFName  )
        {
            aError = aEnumRef.WriteHxx( aFileTab.GetIndex( pFName ) );
            pFName = aFileTab.Next();
        };
    }
    else
        aError = aEnumRef.WriteHxx( nFileKey );

    return aError;
}

/*************************************************************************
|*
|*    RscTypCont :: WriteCxx
|*
|*    Beschreibung
|*    Ersterstellung    MM 30.05.91
|*    Letzte Aenderung  MM 30.05.91
|*
*************************************************************************/
ERRTYPE RscTypCont::WriteCxx( FILE * fOutput, ULONG nFileKey,
                              const ByteString & rHxxName )
{
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );
    ERRTYPE         aError;
    fprintf( fOutput, "#include <string.h>\n" );
    WriteInc( fOutput, nFileKey );
    if( rHxxName.Len() )
        fprintf( fOutput, "#include \"%s\"\n", rHxxName.GetBuffer() );
    fprintf( fOutput, "\n\n" );

    if( NOFILE_INDEX == nFileKey )
    {
        RscFile     *   pFName;

        pFName = aFileTab.First();
        while( pFName  )
        {
            aError = aEnumRef.WriteCxx( aFileTab.GetIndex( pFName ) );
            pFName = aFileTab.Next();
        };
    }
    else
        aError = aEnumRef.WriteCxx( nFileKey );

    return aError;
}

/*************************************************************************
|*
|*    RscTypCont :: WriteSyntax
|*
|*    Beschreibung
|*    Ersterstellung    MM 30.05.91
|*    Letzte Aenderung  MM 30.05.91
|*
*************************************************************************/
void RscTypCont::WriteSyntax( FILE * fOutput )
{
    for( USHORT i = 0; i < aBaseLst.Count(); i++ )
        aBaseLst.GetObject( i )->WriteSyntaxHeader( fOutput, this );
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );
    aEnumRef.WriteSyntax();
}

//=======================================================================
void RscTypCont::WriteRcCtor
(
    FILE * fOutput
)
{
    RscEnumerateRef aEnumRef( this, pRoot, fOutput );
    aEnumRef.WriteRcCtor();
}

/*************************************************************************
|*
|*    RscTypCont :: Delete()
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
class RscDel
{
    ULONG lFileKey;
    DECL_LINK( Delete, RscTop * );
public:
    RscDel( RscTop * pRoot, ULONG lKey );
};


inline RscDel::RscDel( RscTop * pRoot, ULONG lKey )
{
    lFileKey = lKey;
    pRoot->EnumNodes( LINK( this, RscDel, Delete ) );
}

IMPL_LINK_INLINE_START( RscDel, Delete, RscTop *, pNode )
{
    if( pNode->GetObjNode() )
        pNode->pObjBiTree = pNode->GetObjNode()->DelObjNode( pNode, lFileKey );
    return 0;
}
IMPL_LINK_INLINE_END( RscDel, Delete, RscTop *, pNode )

void RscTypCont :: Delete( ULONG lFileKey ){
    // Resourceinstanzen loeschen
    RscDel aDel( pRoot, lFileKey );
    // Defines loeschen
    aFileTab.DeleteFileContext( lFileKey );
}

/*************************************************************************
|*
|*    RscTypCont :: MakeConsistent()
|*
|*    Beschreibung
|*    Ersterstellung    MM 23.09.91
|*    Letzte Aenderung  MM 23.09.91
|*
*************************************************************************/
BOOL IsInstConsistent( ObjNode * pObjNode, RscTop * pRscTop,
                       RscInconsList * pList )
{
    BOOL bRet = TRUE;

    if( pObjNode ){
        RSCINST aTmpI;

        if( ! IsInstConsistent( (ObjNode*)pObjNode->Left(), pRscTop, pList ) )
            bRet = FALSE;

        aTmpI.pClass = pRscTop;
        aTmpI.pData = pObjNode->GetRscObj();
        if( ! aTmpI.pClass->IsConsistent( aTmpI, pList ) )
            bRet = FALSE;

        if( ! IsInstConsistent( (ObjNode*)pObjNode->Right(), pRscTop, pList ) )
            bRet = FALSE;
    };

    return( bRet );
}

BOOL MakeConsistent( RscTop * pRscTop, RscInconsList * pList )
{
    BOOL bRet = TRUE;

    if( pRscTop ){
        if( ! ::MakeConsistent( (RscTop*)pRscTop->Left(), pList ) )
            bRet = FALSE;

        if( pRscTop->GetObjNode() ){
            if( ! pRscTop->GetObjNode()->IsConsistent() ){
                pRscTop->GetObjNode()->OrderTree();
                if( ! pRscTop->GetObjNode()->IsConsistent( pList ) )
                    bRet = FALSE;
            }
            if( ! IsInstConsistent( pRscTop->GetObjNode(), pRscTop, pList ) )
                bRet = FALSE;
        }

        if( ! ::MakeConsistent( (RscTop*)pRscTop->Right(), pList ) )
            bRet = FALSE;
    };

    return bRet;
}

BOOL RscTypCont :: MakeConsistent( RscInconsList * pList ){
    return( ::MakeConsistent( pRoot, pList ) );
}

USHORT RscTypCont::PutTranslatorKey( ULONG nKey )
{
    aIdTranslator.Insert( nKey, (void*)nFilePos );
    return nPMId++;
}

