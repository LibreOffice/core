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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <stdlib.h>
#include <stdio.h>
//#include "bootstrp/sstring.hxx"
#include <vos/mutex.hxx>
#include <string.h>

#include <tools/stream.hxx>
#include <tools/geninfo.hxx>
#include "bootstrp/prj.hxx"
#include "bootstrp/inimgr.hxx"

DECLARE_LIST( UniStringList, UniString* )

//#define TEST  1

#if defined(WNT) || defined(OS2)
#define LIST_DELIMETER ';'
#define PATH_DELIMETER '\\'
#elif defined UNX
#define LIST_DELIMETER ':'
#define PATH_DELIMETER '/'
#endif

//Link Star::aDBNotFoundHdl;

//
//  class SimpleConfig
//

/*****************************************************************************/
SimpleConfig::SimpleConfig( String aSimpleConfigFileName )
/*****************************************************************************/
{
    nLine = 0;
    aFileName = aSimpleConfigFileName;
    aFileStream.Open ( aFileName, STREAM_READ );
}

/*****************************************************************************/
SimpleConfig::SimpleConfig( DirEntry& rDirEntry )
/*****************************************************************************/
{
    nLine = 0;
    aFileName = rDirEntry.GetFull();
    aFileStream.Open ( aFileName, STREAM_READ );
}

/*****************************************************************************/
SimpleConfig::~SimpleConfig()
/*****************************************************************************/
{
    aFileStream.Close ();
}

/*****************************************************************************/
ByteString SimpleConfig::GetNext()
/*****************************************************************************/
{
    ByteString aString;

    if ( aStringBuffer =="" )
      while ((aStringBuffer = GetNextLine()) == "\t") ; //solange bis != "\t"
    if ( aStringBuffer =="" )
        return ByteString();

    aString = aStringBuffer.GetToken(0,'\t');
    aStringBuffer.Erase(0, aString.Len()+1);

    aStringBuffer.EraseLeadingChars( '\t' );

    return aString;
}

/*****************************************************************************/
ByteString  SimpleConfig::GetNextLine()
/*****************************************************************************/
{
    ByteString aSecStr;
    nLine++;

    aFileStream.ReadLine ( aTmpStr );
    if ( aTmpStr.Search( "#" ) == 0 )
        return "\t";
    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
    while ( aTmpStr.SearchAndReplace(ByteString(' '),ByteString('\t') ) != STRING_NOTFOUND ) ;
    int nLength = aTmpStr.Len();
    sal_Bool bFound = sal_False;
    ByteString aEraseString;
    for ( sal_uInt16 i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20  && !bFound )
            aTmpStr.SetChar( i, 0x09 );
    }
    return aTmpStr;
}

/*****************************************************************************/
ByteString SimpleConfig::GetCleanedNextLine( sal_Bool bReadComments )
/*****************************************************************************/
{

    aFileStream.ReadLine ( aTmpStr );
    if ( aTmpStr.Search( "#" ) == 0 )
        {
        if (bReadComments )
            return aTmpStr;
        else
            while ( aTmpStr.Search( "#" ) == 0 )
            {
                aFileStream.ReadLine ( aTmpStr );
            }
        }

    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
//  while ( aTmpStr.SearchAndReplace(String(' '),String('\t') ) != (sal_uInt16)-1 );
    int nLength = aTmpStr.Len();
    ByteString aEraseString;
    sal_Bool bFirstTab = sal_True;
    for ( sal_uInt16 i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20 )
            aTmpStr.SetChar( i, 0x09 );

        if ( aTmpStr.GetChar( i ) ==  0x09 )
        {
            if ( bFirstTab )
                bFirstTab = sal_False;
            else
            {
                aTmpStr.SetChar( i, 0x20 );
            }
        }
        else
            bFirstTab = sal_True;

    }
    aTmpStr.EraseAllChars(' ');
    return aTmpStr;

}

