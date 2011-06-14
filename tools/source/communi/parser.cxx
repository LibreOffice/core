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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>

#include "tools/iparser.hxx"
#include "tools/geninfo.hxx"

//
// class InformationParser
//

#define cKeyLevelChar '\t'

/*****************************************************************************/
InformationParser::InformationParser( sal_Bool bReplace )
/*****************************************************************************/
                : bRecover( sal_False ),
                sOldLine( "" ),
                bReplaceVariables( bReplace ),
                nLevel( 0 ),
                sUPD( "" ),
                sVersion( "" ),
                pActStream( NULL ),
                nErrorCode( 0 ),
                nErrorLine( 0 ),
                sErrorText( "" ),
                nActLine( 0 )
{
}

/*****************************************************************************/
InformationParser::~InformationParser()
/*****************************************************************************/
{
}

/*****************************************************************************/
ByteString &InformationParser::ReadLine()
/*****************************************************************************/
{
    ByteString sLine;

    if ( bRecover ) {
        bRecover = sal_False;
    }
    else {
         if ( !pActStream->IsEof()) {
            pActStream->ReadLine( sLine );
            xub_StrLen nStart = 0;
            xub_StrLen nEnd = sLine.Len();
            sal_Bool bCopy = sal_False;
            while ( nStart < nEnd && ( sLine.GetChar( nStart ) == ' ' || sLine.GetChar( nStart ) == 0x09 ) )
            {
                nStart++;
                bCopy = sal_True;
            }

            while ( nStart < nEnd && ( sLine.GetChar( nEnd-1 ) == ' ' || sLine.GetChar( nEnd-1 ) == 0x09 ) )
            {
                nEnd--;
                bCopy = sal_True;
            }

            if ( bCopy )
                sLine = sLine.Copy( nStart, nEnd - nStart );

            if (( sLine.GetChar( 0 ) == '#' ) || ( !sLine.Len())) {
                if ( sCurrentComment.Len())
                    sCurrentComment += "\n";
                sCurrentComment += sLine;
                return ReadLine();
            }
            else {
                if ( bReplaceVariables ) {
                    sLine.SearchAndReplaceAll( "%UPD", sUPD );
                    sLine.SearchAndReplaceAll( "%VERSION", sVersion );
                }
            }
        }
        else {
            if ( nLevel ) {
                sLine = "}";
                fprintf( stdout, "Reached EOF parsing %s. Suplying extra '}'\n",ByteString( sStreamName, gsl_getSystemTextEncoding()).GetBuffer() );
    //          nErrorCode = IP_UNEXPECTED_EOF;
    //          nErrorLine = nActLine;
            }
            else
                sLine = "";
        }

        sOldLine = sLine;
        nActLine++;
    }

    return sOldLine;
}

/*****************************************************************************/
GenericInformation *InformationParser::ReadKey(
                                    GenericInformationList *pExistingList )
/*****************************************************************************/
{
    // this method has no error handling yet, but it works very fast.
    // it is used to create whole informations and sub informations in
    // a simple data format in memory, readed in a configuration file with
    // following format:

    /*

    key [value]
    {
        key [value]
        key [value]
        {
            key [value]
            ...
            ...
        }
    }
    key [value]
    ...
    ...

    */

    GenericInformation *pInfo = NULL;

    ByteString sLine( ReadLine());
    ByteString sKey;
    ByteString sValue;
    ByteString sComment( sCurrentComment );
    sCurrentComment = "";

    // key separated from value by tab?
    sal_uInt16 nWSPos = sLine.Search( ' ' );
    if ( sLine.Search( '\t' ) < nWSPos ) {
        nWSPos = sLine.Search( '\t' );
        sLine.SearchAndReplace( "\t", " " );
    }

    if ( sLine.GetTokenCount( ' ' ) > 1 ) {
        sKey = sLine.GetToken( 0, ' ' );
        sValue = sLine.Copy( sKey.Len() + 1 );
        while (( sValue.Search( ' ' ) == 0 ) || ( sValue.Search( '\t' ) == 0 )) {
            sValue.Erase( 0, 1 );
        }
    }
    else
        sKey=sLine;

    if ( bReplaceVariables && !nLevel ) {
        sUPD = sKey.Copy( sKey.Len() - 3 );
        sVersion = sKey;
    }

    if ( ReadLine() == "{" ) {
        nLevel++;
        GenericInformationList *pSubList = new GenericInformationList();
        while ( ReadLine() != "}" ) {
            Recover();
            ReadKey( pSubList );
        }
        nLevel--;
        pInfo = new GenericInformation( sKey, sValue,
                        pExistingList, pSubList );
        pInfo->SetComment( sComment );
    }
    else {
        Recover();
        if ( !sKey.Equals( "}" ) && !sKey.Equals( "{" ) )
        {
            pInfo = new GenericInformation( sKey, sValue, pExistingList );
            pInfo->SetComment( sComment );
        }
    }

    return pInfo;
}

/*****************************************************************************/
void InformationParser::Recover()
/*****************************************************************************/
{
    bRecover = sal_True;
}

/*****************************************************************************/
sal_Bool InformationParser::Save( SvStream &rOutStream,
                  const GenericInformationList *pSaveList,
                  sal_uInt16 level, sal_Bool bStripped )
/*****************************************************************************/
{
    sal_uInt16 i;
    sal_uIntPtr nInfoListCount;
    ByteString sTmpStr;
    GenericInformation *pGenericInfo;
    GenericInformationList *pGenericInfoList;

     static ByteString aKeyLevel;
    aKeyLevel.Expand( level, cKeyLevelChar );

    for ( nInfoListCount = 0; nInfoListCount < pSaveList->size(); nInfoListCount++) {
        // Key-Value Paare schreiben
        pGenericInfo = (*pSaveList)[ nInfoListCount ];
        sTmpStr = "";
        if ( !bStripped && level )
            sTmpStr.Append( aKeyLevel.GetBuffer(), level );

        if ( !bStripped )
            for ( i = 0; i < pGenericInfo->GetComment().GetTokenCount( '\n' ); i++ ) {
                sTmpStr += pGenericInfo->GetComment().GetToken( i, '\n' );
                sTmpStr += "\n";
                if ( level )
                    sTmpStr.Append( aKeyLevel.GetBuffer(), level );
            }

        sTmpStr += pGenericInfo->GetBuffer();
        sTmpStr += ' ';
        sTmpStr += pGenericInfo->GetValue();
        if ( !rOutStream.WriteLine( sTmpStr ) )
              return sal_False;

        // wenn vorhanden, bearbeite recursive die Sublisten
        if (( pGenericInfoList = pGenericInfo->GetSubList() ) != NULL ) {
              // oeffnende Klammer
              sTmpStr = "";
            if ( !bStripped && level )
                sTmpStr.Append( aKeyLevel.GetBuffer(), level );
              sTmpStr += '{';
              if ( !rOutStream.WriteLine( sTmpStr ) )
                return sal_False;
              // recursiv die sublist abarbeiten
              if ( !Save( rOutStream, pGenericInfoList, level+1, bStripped ) )
                return sal_False;
                  // schliessende Klammer
              sTmpStr = "";
            if ( !bStripped && level )
                sTmpStr.Append( aKeyLevel.GetBuffer(), level );
              sTmpStr += '}';
              if ( !rOutStream.WriteLine( sTmpStr ) )
                return sal_False;
        }
      }
      return sal_True;
}

/*****************************************************************************/
GenericInformationList *InformationParser::Execute(
                                SvStream &rSourceStream,
                                GenericInformationList *pExistingList )
/*****************************************************************************/
{
    GenericInformationList *pList;
    if ( pExistingList )
        pList = pExistingList;
    else
        pList = new GenericInformationList();

    pActStream = &rSourceStream;

    // read all infos out of current file
    while( !rSourceStream.IsEof()) {
        nLevel = 0;
        ReadKey( pList );
    }

    return pList;
}

/*****************************************************************************/
GenericInformationList *InformationParser::Execute( SvMemoryStream &rSourceStream,
                            GenericInformationList *pExistingList )
/*****************************************************************************/
{
      sStreamName = UniString( "Memory", gsl_getSystemTextEncoding());
      return Execute( (SvStream &)rSourceStream, pExistingList );
}

/*****************************************************************************/
GenericInformationList *InformationParser::Execute(
                                SvFileStream &rSourceStream,
                                GenericInformationList *pExistingList )
/*****************************************************************************/
{
    if ( !rSourceStream.IsOpen())
        return NULL;
    sStreamName = rSourceStream.GetFileName();
    return Execute( (SvStream &)rSourceStream, pExistingList );
}

/*****************************************************************************/
GenericInformationList *InformationParser::Execute( UniString &rSourceFile,
                                GenericInformationList *pExistingList )
/*****************************************************************************/
{
    DirEntry aDirEntry( rSourceFile );
    if ( !aDirEntry.Exists())
        return NULL;

    GenericInformationList *pList;
    if ( pExistingList )
        pList = pExistingList;
    else
        pList = new GenericInformationList();

    // reset status
    nErrorCode = 0;
    nErrorLine = 0;
    nActLine = 0;

    SvFileStream aActStream;
    aActStream.Open( rSourceFile, STREAM_READ );
    if( aActStream.GetError())
        return NULL;

    pActStream = &aActStream;
    if ( !Execute( aActStream, pList )) {
        delete pList;
        pList = NULL;
    }

    // close the stream
    aActStream.Close();
     pActStream = NULL;

    if ( !nErrorCode )
        return pList;

    return NULL;
}

/*****************************************************************************/
GenericInformationList *InformationParser::Execute( Dir &rDir,
                                GenericInformationList *pExistingList )
/*****************************************************************************/
{
    GenericInformationList *pList;

    if ( pExistingList )
        pList = pExistingList;
    else
        pList = new GenericInformationList();

    for ( sal_uInt16 i = 0; i < rDir.Count(); i++ ) {

        // execute this dir
        UniString sNextFile( rDir[i].GetFull());
        GenericInformationList *pSubList = Execute( sNextFile );

        if ( !pSubList ) {
            // any errors ?
            delete pList;
            return NULL;
        }

        // create new info and insert it into list
        ByteString sFileKey( rDir[i].GetName(), RTL_TEXTENCODING_UTF8 );
        new GenericInformation(
                                            sFileKey,
                                            ByteString( "" ),
                                            pList, pSubList );
    }

    return pList;
}

/*****************************************************************************/
sal_Bool InformationParser::Save( SvFileStream &rSourceStream,
                  const GenericInformationList *pSaveList )
/*****************************************************************************/
{
    if ( !rSourceStream.IsOpen() || !Save( (SvStream &)rSourceStream, pSaveList, 0, sal_False ))
    {
        printf( "ERROR saving file \"%s\"\n",ByteString( rSourceStream.GetFileName(), gsl_getSystemTextEncoding()).GetBuffer() );
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
sal_Bool InformationParser::Save( SvMemoryStream &rSourceStream,
                  const GenericInformationList *pSaveList )
/*****************************************************************************/
{
    Time a;
    sal_Bool bRet = Save( (SvStream &)rSourceStream, pSaveList, 0, sal_True );
    Time b;
    b = b - a;
    return bRet;
}

/*****************************************************************************/
sal_Bool InformationParser::Save( const UniString &rSourceFile,
                  const GenericInformationList *pSaveList )
/*****************************************************************************/
{
  SvFileStream *pOutFile = new SvFileStream( rSourceFile, STREAM_STD_WRITE | STREAM_TRUNC );

  if ( !Save( *pOutFile, pSaveList )) {
    delete pOutFile;
    return sal_False;
  }
  delete pOutFile;
  return sal_True;
}

/*****************************************************************************/
sal_uInt16 InformationParser::GetErrorCode()
/*****************************************************************************/
{
    return nErrorCode;
}

/*****************************************************************************/
ByteString &InformationParser::GetErrorText()
/*****************************************************************************/
{
  //    sErrorText = pActStream->GetFileName();
    sErrorText = ByteString( sStreamName, gsl_getSystemTextEncoding());
    sErrorText += ByteString( " (" );
    sErrorText += ByteString::CreateFromInt64(nErrorLine);
    sErrorText += ByteString( "): " );

    switch ( nErrorCode ) {
    case IP_NO_ERROR:
        sErrorText += ByteString( "Keine Fehler aufgetereten" );
        break;
    case IP_UNEXPECTED_EOF:
        sErrorText += ByteString( "Ungültiges Dateiende!" );
        break;
    }

    return sErrorText;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
