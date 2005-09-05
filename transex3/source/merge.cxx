/*************************************************************************
 *
 *  $RCSfile: merge.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-05 11:21:10 $
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
#include <stdio.h>
#include <tools/fsys.hxx>
#include "export.hxx"
#include "utf8conv.hxx"
#include <iostream>

using namespace std;

extern void ConvertHalfwitdhToFullwidth( String& rString );

//
// class PFormEntrys
//

ByteString PFormEntrys::Dump(){
    ByteString sRet( "PFormEntrys\n" );
    //sRet.Append( Export::DumpMap( ByteString("sText") , sText ) );
    //sRet.Append("\n");
    ByteString a("sText");
    if ( sText.size() ) Export::DumpMap( a , sText );
    return sRet;
}

/*****************************************************************************/
BOOL PFormEntrys::GetText( ByteString &rReturn,
    //USHORT nTyp, USHORT nLangIndex, BOOL bDel )
    USHORT nTyp, const ByteString &nLangIndex, BOOL bDel )
/*****************************************************************************/
{
    //printf("DBG: PFormEntrys::GetText(nId=%s)\n",nLangIndex.GetBuffer() );

            // DEBUG******************
            //ByteStringHashMap::const_iterator idbg;
            //std::cout << "HASHKEYS : \n";
            //for( idbg = sText.begin() ; idbg != sText.end(); ++idbg )
            //    std::cout << (idbg->first).GetBuffer() << "\n";
            //std::cout << "\n\n";
            // DEBUG******************


    BOOL bReturn;
    switch ( nTyp ) {
        case STRING_TYP_TEXT :
            rReturn = sText[ nLangIndex ];
            if ( bDel )
                sText[ nLangIndex ] = "";
            bReturn = bTextFirst[ nLangIndex ];
            bTextFirst[ nLangIndex ] = FALSE;
            break;
        case STRING_TYP_HELPTEXT :
            rReturn = sHelpText;
            break;
        case STRING_TYP_QUICKHELPTEXT :
            rReturn = sQuickHelpText[ nLangIndex ];
            if ( bDel )
                sQuickHelpText[ nLangIndex ] = "";
            bReturn = bQuickHelpTextFirst[ nLangIndex ];
            bQuickHelpTextFirst[ nLangIndex ] = FALSE;
            break;
        case STRING_TYP_TITLE :
            rReturn = sTitle[ nLangIndex ];
            if ( bDel )
                sTitle[ nLangIndex ] = "";
            bReturn = bTitleFirst[ nLangIndex ];
            bTitleFirst[ nLangIndex ] = FALSE;
            break;
    }
    //printf("Returning '%s'\n",rReturn.GetBuffer());
    return bReturn;
}


//
// class MergeData
//

/*****************************************************************************/
MergeData::~MergeData()
/*****************************************************************************/
{
}

/*****************************************************************************/
PFormEntrys* MergeData::GetPFormEntrys( ResData *pResData )
/*****************************************************************************/
{
//    if( pResData->sPForm.Len() && aMap.find( pResData->sPForm ) != aMap.end() ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() ){
        return aMap[ ByteString("HACK") ];
    }
    else{
        return 0;
    }
}

void MergeData::Insert( const ByteString& rPFO , PFormEntrys* pfEntrys ){
//    aMap.insert( PFormEntrysHashMap::value_type( rPFO , pfEntrys ) );
    aMap.insert( PFormEntrysHashMap::value_type( ByteString("HACK") , pfEntrys ) );

}
ByteString MergeData::Dump(){
    ByteString sRet( "MergeData\n" );
/*  sRet.Append("sTyp = ");
    sRet.Append(sTyp);
    sRet.Append("\n");
    sRet.Append("sGid=");
    sRet.Append(sGID);
    sRet.Append("\n");
    sRet.Append("sLid=");
    sRet.Append(sLID);
    sRet.Append("\n");*/

    printf("MergeData sTyp = %s , sGid = %s , sLid =%s\n",sTyp.GetBuffer(),sGID.GetBuffer(),sLID.GetBuffer());

    PFormEntrysHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg ){
/*      sRet.Append( "aMap[" );
        sRet.Append( idbg->first );
           sRet.Append( "]= " );
        sRet.Append( ( (PFormEntrys*)(idbg->second) )->Dump() );
        sRet.Append("\n");*/
        printf("aMap[ %s ] = " ,idbg->first.GetBuffer());
        ( (PFormEntrys*)(idbg->second) )->Dump();
        printf("\n") ;
    }
    //sRet.Append("\n");
    printf("\n") ;
    return sRet;
}

PFormEntrys* MergeData::GetPFObject( const ByteString& rPFO ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() ){
//    if( aMap.find( rPFO ) != aMap.end() ){
        return aMap[ rPFO ];
    }
    else{
        return 0;
    }
}


/*****************************************************************************/
PFormEntrys *MergeData::InsertEntry( const ByteString &rPForm )
/*****************************************************************************/
{
    PFormEntrys* pFEntrys = new PFormEntrys( rPForm );
    aMap.insert( PFormEntrysHashMap::value_type( rPForm , pFEntrys ) );
    return pFEntrys;
}

/*****************************************************************************/
BOOL MergeData::operator==( ResData *pData )
/*****************************************************************************/
{
    return (( pData->sId == sLID ) &&
            ( pData->sGId == sGID ) &&
            ( ByteString( pData->sResTyp ).ToUpperAscii() ==
                ByteString( sTyp ).ToUpperAscii()));
}

//
// class MergeDataFile
//

#define FFORMAT_UNKNOWN 0x0000
#define FFORMAT_NEW     0x0001
#define FFORMAT_OLD     0x0002

/*void MergeDataFile::Quote( ByteString& sText ){
    for( int x=0; x < sText.Len(); x++ ){
        if( sText.GetChar( x ) == '\"' ){
            if( x > 1 && sText.GetChar( x-1) != '\\' ){
                sText.Insert('\\', x);
                x++;
            }else{
                sText.Insert('\\',0);
                x++;
            }
        }
    }
}*/
/*****************************************************************************/
MergeDataFile::MergeDataFile( const ByteString &rFileName, const ByteString& sFile ,BOOL bErrLog,
                            CharSet aCharSet, BOOL bUTF8 )
/*****************************************************************************/
                : bErrorLog( bErrLog )
{
    SvFileStream aInputStream( String( rFileName, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    aInputStream.SetStreamCharSet( aCharSet );
    ByteString sLine;

    ByteString sTYP;
    ByteString sGID;
    ByteString sLID;
    ByteString sPFO;
    ByteString nLANG;
    ByteString sTEXT;
    ByteString sQHTEXT;
    ByteString sTITLE;

    if( !aInputStream.IsOpen() ) {
        printf("ERROR : Can't open %s\n", rFileName.GetBuffer());
        exit( -1 );
    }
    while ( !aInputStream.IsEof()) {
        aInputStream.ReadLine( sLine );
        sLine = sLine.Convert( RTL_TEXTENCODING_MS_1252, aCharSet );

        if ( sLine.GetTokenCount( '\t' ) == 15  ) {
            // Skip all wrong filenames
            ByteString filename = sLine.GetToken( 1 , '\t' );
            filename = filename.Copy( filename.SearchCharBackward( "\\" )+1 , filename.Len() );

            if( filename.Equals( sFile ) )
            {
                sTYP = sLine.GetToken( 3, '\t' );
                sGID = sLine.GetToken( 4, '\t' );
                sLID = sLine.GetToken( 5, '\t' );
                sPFO = sLine.GetToken( 7, '\t' );
                sPFO = ByteString("HACK");
                nLANG = sLine.GetToken( 9, '\t' );

                sTEXT = sLine.GetToken( 10, '\t' );
    //            printf("%s\n",sTEXT.GetBuffer());
    //            Quote( sTEXT );
    //            printf("%s\n",sTEXT.GetBuffer());

                sQHTEXT = sLine.GetToken( 12, '\t' );
                sTITLE = sLine.GetToken( 13, '\t' );

                nLANG.EraseLeadingAndTrailingChars();
                if (  !nLANG.EqualsIgnoreCaseAscii("en-US")  ){
                    InsertEntry( sTYP, sGID, sLID, sPFO, nLANG, sTEXT, sQHTEXT, sTITLE );
                    if( nLANG.Len() > 0 ){
                        bool bFound = false;
                        for( int x = 0; x < aLanguages.size(); x++ ){
                            if( aLanguages[ x ].Equals( nLANG ) )
                                bFound = true;
                        }
                        // Remember read languages for -l all switch
                        if( !bFound )   aLanguages.push_back( nLANG );
                    }
                }
            }
        }
        else if ( sLine.GetTokenCount( '\t' ) == 10 ){
            printf("ERROR: File format is obsolete and no longer supported!\n");
        }
    }
    aInputStream.Close();
}
/*****************************************************************************/
MergeDataFile::~MergeDataFile()
/*****************************************************************************/
{
}

/*****************************************************************************/
void MergeDataFile::WriteErrorLog( const ByteString &rFileName )
/*****************************************************************************/
{
// DEAD
}

ByteString MergeDataFile::Dump(){
    ByteString sRet( "MergeDataFile\n" );

      //sRet.Append( Export::DumpMap( "aLanguageSet" , aLanguageSet ) );
    //sRet.Append( Export::DumpMap( "aLanguages" , aLanguages ) );
    printf("MergeDataFile\n");
    MergeDataHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg ){
        /*sRet.Append( "aMap[" );
        sRet.Append( idbg->first );
           sRet.Append( "]= " );
        sRet.Append( ((MergeData*) (idbg->second))->Dump() );
        sRet.Append("\n");*/

        printf("aMap[ %s ] = ",idbg->first.GetBuffer());
        ((MergeData*) (idbg->second))->Dump();
        printf("\n");
    }
    printf("\n");
    //sRet.Append("\n");
    return sRet;
}

/*****************************************************************************/
void MergeDataFile::WriteError( const ByteString &rLine )
/*****************************************************************************/
{
    if ( bErrorLog ) {
        if ( !aErrLog.IsOpen())
            aErrLog.Open( String( sErrorLog, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aErrLog.WriteLine( rLine );
    }
    else
        fprintf( stderr, "%s\n", rLine.GetBuffer());
}
std::vector<ByteString> MergeDataFile::GetLanguages(){
    return aLanguages;
}

/*****************************************************************************/
MergeData *MergeDataFile::GetMergeData( ResData *pResData )
/*****************************************************************************/
{
    ByteString sOldG = pResData->sGId;
    ByteString sOldL = pResData->sId;
    ByteString sGID = pResData->sGId;
    ByteString sLID;
    if ( !sGID.Len())
        sGID = pResData->sId;
    else
        sLID = pResData->sId;
    pResData->sGId = sGID;
    pResData->sId = sLID;

    ByteString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId );

    //printf("DBG: Searching [%s]\n",sKey.GetBuffer());
    if( aMap.find( sKey ) != aMap.end() ){
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        //printf("DBG: Found[%s]\n",sKey.GetBuffer());
        return aMap[ sKey ];
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    //printf("DBG: Found[%s]\n",sKey.GetBuffer());
    return NULL;
}


/*****************************************************************************/
PFormEntrys *MergeDataFile::GetPFormEntrys( ResData *pResData )
/*****************************************************************************/
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData );
    if ( pData )
        return pData->GetPFormEntrys( pResData );
    return NULL;
}

/*****************************************************************************/
void MergeDataFile::InsertEntry(
                    const ByteString &rTYP, const ByteString &rGID,
                    const ByteString &rLID, const ByteString &rPFO,
                    const ByteString &nLANG, const ByteString &rTEXT,
                    const ByteString &rQHTEXT, const ByteString &rTITLE )
/*****************************************************************************/
{
    MergeData *pData;
    BOOL bFound = FALSE;

    // search for MergeData
    ByteString sKey = CreateKey( rTYP , rGID , rLID );
    ByteString sKey2;

    if( aMap.find( sKey ) != aMap.end() ){
        pData = aMap[ sKey ];
    }else{
        pData = new MergeData( rTYP, rGID, rLID );
        aMap.insert( MergeDataHashMap::value_type( CreateKey( rTYP , rGID , rLID ) , pData ) );
    }

    bFound = FALSE;
    PFormEntrys *pFEntrys = 0;

    // search for PFormEntrys

    pFEntrys = pData->GetPFObject( rPFO );
    if( !pFEntrys ){
        // create new PFormEntrys, cause no one exists with current properties
        pFEntrys = new PFormEntrys( rPFO );
        pData->Insert( rPFO , pFEntrys );
    }

    // finaly insert the cur string

//  if( rTYP.EqualsIgnoreCaseAscii("PairedList") ){
//      sKey2 = CreateKey( rTYP , rGID , rLID );

    /*      sKey2.Append( '-' );
        sKey2.Append( nLANG );
        sKey2.ToUpperAscii();*/
        //pFEntrys->InsertEntry( rLID , rTEXT, rQHTEXT, rTITLE );
//      pFEntrys->InsertEntry( sKey2 , rTEXT, rQHTEXT, rTITLE );
//  }
//  else
        pFEntrys->InsertEntry( nLANG , rTEXT, rQHTEXT, rTITLE );

    //printf("DBG: MergeDataFile::Insert[%s]=( sKey=%s,rTEXT=%s,%s,%s)\n",sKey2.GetBuffer(),nLANG.GetBuffer(),rTEXT.GetBuffer(),rQHTEXT.GetBuffer(),rTITLE.GetBuffer());
}
ByteString MergeDataFile::CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID ){
    ByteString sKey( rTYP );
    sKey.Append( '-' );
    sKey.Append( rGID );
    sKey.Append( '-' );
    sKey.Append( rLID  );
    return sKey.ToUpperAscii();
}


