/*************************************************************************
 *
 *  $RCSfile: msvbasic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cmc $ $Date: 2001-02-16 12:48:28 $
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


#include <string.h>     // memset(), ...
#ifndef UNX
#include <io.h>         // access()
#endif

#ifndef _MSVBASIC_HXX
#include <msvbasic.hxx>
#endif

/* class VBA_Impl:
 * The VBA class provides a set of methods to handle Visual Basic For
 * Applications streams, the constructor is given the root ole2 stream
 * of the document, Open reads the VBA project file and figures out
 * the number of VBA streams, and the offset of the data within them.
 * Decompress decompresses a particular numbered stream, NoStreams returns
 * this number, and StreamName can give you the streams name. Decompress
 * will call Output when it has a 4096 byte collection of data to output,
 * and also with the final remainder of data if there is still some left
 * at the end of compression. Output is virtual to allow custom handling
 * of each chunk of decompressed data. So inherit from this to do something
 * useful with the data.
 *
 * cmc
 * */

BYTE VBA_Impl::ReadPString(SvStorageStreamRef &xVBAProject)
{
    UINT16 nIdLen;
    int i=0;
    BYTE nType = 0, nOut;

    *xVBAProject >> nIdLen;

    if (nIdLen < 6)
    {
        xVBAProject->SeekRel(-2);
    }
    else
        for(i=0; i < nIdLen / 2; i++)
        {
            *xVBAProject >> nOut;
            xVBAProject->SeekRel(1);
            if (i==2)
            {
                nType = nOut;
                if ((nType != 'G') && (nType != 'C'))
                    nType = 0;
                if ( nType == 0)
                {
                    xVBAProject->SeekRel(-8);
                    break;
                }
            }
        }

    return nType;
}

static BOOL lcl_CheckArray( SvStream &rStrm, const BYTE* pArr, USHORT nLen )
{
    BYTE* pMem = new BYTE[ nLen ];

    rStrm.Read( pMem, nLen );
    BOOL bRet = 0 == memcmp( pMem, pArr, nLen );
    delete pMem;
    return bRet;
}

void VBA_Impl::ConfirmFixedOctect(SvStorageStreamRef &xVBAProject)
{
    static const BYTE coTest[8] =
    {
        0x06, 0x02, 0x01, 0x00, 0x08, 0x02, 0x00, 0x00
    };

    if( !lcl_CheckArray( *xVBAProject, coTest, sizeof( coTest ) ))
        DBG_WARNING("Found a different octect, please report");
}

void VBA_Impl::Confirm12Zeros(SvStorageStreamRef &xVBAProject)
{
    static const BYTE coTest[12]={0};
    if( !lcl_CheckArray( *xVBAProject, coTest, sizeof( coTest ) ))
        DBG_WARNING("Found a Non Zero block, please report");
}

void VBA_Impl::ConfirmHalfWayMarker(SvStorageStreamRef &xVBAProject)
{
    static const BYTE coTest[12]={0,0,0,0,0,0,0,0,0,0,1,0};
    if( !lcl_CheckArray( *xVBAProject, coTest, sizeof( coTest ) ))
        DBG_WARNING("Found a different halfway marker, please report");
}

void VBA_Impl::ConfirmFixedMiddle(SvStorageStreamRef &xVBAProject)
{
    static const BYTE coTest[20] =
    {
        0x00, 0x00, 0xe1, 0x2e, 0x45, 0x0d, 0x8f, 0xe0,
        0x1a, 0x10, 0x85, 0x2e, 0x02, 0x60, 0x8c, 0x4d,
        0x0b, 0xb4, 0x00, 0x00
    };
    if( !lcl_CheckArray( *xVBAProject, coTest, sizeof( coTest ) ))
    {
        DBG_WARNING("Found a different middle marker, please report");
        xVBAProject->SeekRel( - sizeof( coTest ));
    }
}

void VBA_Impl::ConfirmFixedMiddle2(SvStorageStreamRef &xVBAProject)
{
    static const BYTE coTest[20] =
    {
        0x00, 0x00, 0x2e, 0xc9, 0x27, 0x8e, 0x64, 0x12,
        0x1c, 0x10, 0x8a, 0x2f, 0x04, 0x02, 0x24, 0x00,
        0x9c, 0x02, 0x00, 0x00
    };


    if( !lcl_CheckArray( *xVBAProject, coTest, sizeof( coTest ) ))
    {
        DBG_WARNING("Found a different middle2 marker, please report");
        xVBAProject->SeekRel( - sizeof( coTest ));
    }
}


void VBA_Impl::Output( int nLen, const BYTE *pData )
{
    /*
    Each StarBasic module is tragically limited to the maximum len of a
    string and WordBasic is not, so each overlarge module must be split
    */
    String sTemp((const sal_Char *)pData, (xub_StrLen)nLen,
        RTL_TEXTENCODING_MS_1252);
    int nTmp = sTemp.GetTokenCount('\x0D');
    int nIndex = aVBAStrings.GetSize()-1;
    if (aVBAStrings.Get(nIndex)->Len() +
        nLen + ((nLines+nTmp) * sComment.Len()) >= STRING_MAXLEN)
    {
        //DBG_ASSERT(0,"New Module String\n");
        //we are too large for our boots, break out into another
        //string
        nLines=0;
        nIndex++;
        aVBAStrings.SetSize(nIndex+1);
        aVBAStrings.Put(nIndex,new String);
    }
    *(aVBAStrings.Get(nIndex)) += sTemp;
    nLines+=nTmp;
}


int VBA_Impl::ReadVBAProject(const SvStorageRef &rxVBAStorage)
{
    SvStorageStreamRef xVBAProject;
    xVBAProject = rxVBAStorage->OpenStream( String(
                                RTL_CONSTASCII_STRINGPARAM( "_VBA_PROJECT" ),
                                RTL_TEXTENCODING_MS_1252 ),
                    STREAM_STD_READ | STREAM_NOCREATE );

    if( !xVBAProject.Is() || SVSTREAM_OK != xVBAProject->GetError() )
    {
        DBG_WARNING("Not able to find vba project, cannot find macros");
        return(0);
    }
    xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    static const BYTE coHeader[30] =
    {
        0xcc, 0x61, 0x5e, 0x00, 0x00, 0x01, 0x00, 0xff,
        0x07, 0x04, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00,
        0xe4, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00
    };

    if( !lcl_CheckArray( *xVBAProject, coHeader, sizeof( coHeader ) ))
        DBG_WARNING("Warning VBA header is different, please report");

    UINT16 nValue, nSValue;
    *xVBAProject >> nValue >> nSValue;

    //*pOut << "Trigger value 1 is " << nValue << endl;
    if( nSValue != 0x02 )
        DBG_WARNING("Warning VBA number is different, please report");

    //*pOut << "Other strings after the middle are..." << endl;
    //There appears to be almost any number of strings acceptable
    //most begin with */G , and sometimes with
    //*/C. Those with G always have a trailer of 12 bytes, those
    //with C come in pairs, the first with no trailer, and the
    //second with one of 12 bytes. The following code attemts
    //to read these strings and ends when it reaches a sequence of
    //bytes which fails a test to be a valid string. So this
    //while loop here is the particular piece of code which is
    //very suspect and likely to be the cause of any crashes and
    //problems.
    int nCount=0;
    BYTE nType;
    while ((nType = ReadPString(xVBAProject)) != 0)
    {
        //*pOut << endl;
        //*pOut << "testcharacter is " << testc << endl;
        switch (nType)
        {
            case 'C':
                nCount++;
                if (nCount == 2)
                {
                    Confirm12Zeros(xVBAProject);
                    nCount=0;
                }
                break;
            default:
  //            case 'G':
                Confirm12Zeros(xVBAProject);
                break;
        }
    }

    //appears to be a fixed 20 byte sequence here, and then the strings
    //continue
    ConfirmFixedMiddle(xVBAProject);

    nCount=0;

    while ((nType = ReadPString(xVBAProject)) != 0)
    {
        //*pOut << endl;
        //*pOut << "testcharacter is " << testc << endl;
        switch (nType)
        {
            case 'C':
                nCount++;
                if (nCount == 2)
                {
                    Confirm12Zeros(xVBAProject);
                    nCount=0;
                }
                break;
            default:
//          case 'G':
                Confirm12Zeros(xVBAProject);
                break;
        }
    }

    //there *may* be another different 20byte fixed string
    ConfirmFixedMiddle2(xVBAProject);

    //*pOut << "testc is " << testc << endl;
    //*pOut << "position is " << xVBAProject->Tell() << endl;

    UINT16 nModules;
    *xVBAProject >> nModules;

    //begin section, this section isn't really 100% correct
    //*pOut << nModules << hex << " vba modules" << endl;
    xVBAProject->SeekRel(2*nModules);
    xVBAProject->SeekRel(4);
    //*pOut << "position is " << xVBAProject->Tell() << endl;
    ConfirmFixedOctect(xVBAProject);

    UINT16 nJunkSize;
    *xVBAProject >> nJunkSize;
    while(nJunkSize != 0xFFFF)
    {
        xVBAProject->Read(&nJunkSize,2); // usually 18 02, sometimes 1e 02
        //but sometimes its a run of numbers until 0xffff, gagh!!!
        //*pOut << "position is " << xVBAProject->Tell() << "len is "
        //  << nJunkSize << endl;
    }

    UINT16 nFTest;
    *xVBAProject >> nFTest;
    if (nFTest != 0xFFFF)
        xVBAProject->SeekRel(nFTest);
    *xVBAProject >> nFTest;
    if (nFTest != 0xFFFF)
        xVBAProject->SeekRel(nFTest);

    xVBAProject->SeekRel(100);
    //*pOut << "position is " << xVBAProject->Tell() << endl;
    //end section

    *xVBAProject >> nOffsets;
    pOffsets = new VBAOffset_Impl[ nOffsets ];
    int i, j;
    for( i=0; i < nOffsets; i++)
    {
        UINT16 nLen;
        *xVBAProject >> nLen;

        sal_Unicode* pBuf = pOffsets[i].sName.AllocBuffer( nLen / 2 );
        xVBAProject->Read( (sal_Char*)pBuf, nLen  );

#ifdef __BIGENDIAN
        for( j = 0; j < nLen / 2; ++j, ++pBuf )
            *pBuf = SWAPLONG( *pBuf );
#endif // ifdef __BIGENDIAN

        *xVBAProject >> nLen;
        xVBAProject->SeekRel( nLen );

        //begin section, another problem area
        *xVBAProject >> nLen;
        if ( nLen == 0xFFFF)
        {
            xVBAProject->SeekRel(2);
            *xVBAProject >> nLen;
            xVBAProject->SeekRel( nLen );
        }
        else
            xVBAProject->SeekRel( nLen+2 );
        //
        /* I have a theory that maybe you read a 16bit len, and
         * if it has 0x02 for the second byte then it is a special
         * token of its own that affects nothing else, otherwise
         * it is a len of the following data. C. I must test this
         * theory later.
         */
        //end section

        //begin section, another problem area
        /*
        The various 0xFFFF seems to be makers that a particular
        field is empty, though without having enough example of
        what the fields are its difficult to determine where
        they appear in the records
        */

        *xVBAProject >> nLen;
        if ( nLen != 0xFFFF)
            xVBAProject->SeekRel( nLen );

        xVBAProject->SeekRel(6);
        USHORT nOctects;
        *xVBAProject >> nOctects;
        for(j=0;j<nOctects;j++)
            xVBAProject->SeekRel(8);

        xVBAProject->SeekRel(5);
        //end section

        *xVBAProject >> pOffsets[i].nOffset;
        //*pOut << pOffsets[i].pName.GetStr() << " at 0x" << hex << pOffsets[i].nOffset << endl;
        xVBAProject->SeekRel(2);
    }

    //*pOut << endl;
    return nOffsets;
}

BOOL VBA_Impl::Open( const String &rToplevel, const String &rSublevel )
{
    /* beginning test for vba stuff */
    BOOL bRet = FALSE;
    SvStorageRef xMacros= xStor->OpenStorage( rToplevel,
                                    STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL );
    if( !xMacros.Is() || SVSTREAM_OK != xMacros->GetError() )
    {
        DBG_WARNING("No Macros Storage");
    }
    else
    {
        xVBA = xMacros->OpenStorage( rSublevel,
                                    STREAM_READWRITE | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYALL );
        if( !xVBA.Is() || SVSTREAM_OK != xVBA->GetError() )
        {
            DBG_WARNING("No Visual Basic in Storage");
        }
        else
        {
            if (ReadVBAProject(xVBA))
                bRet = TRUE;
        }
    }
    /* end test for vba stuff */
    return bRet;
}

const StringArray &VBA_Impl::Decompress(UINT16 nIndex, int *pOverflow)
{
    DBG_ASSERT( nIndex < nOffsets, "Index out of range" );
    SvStorageStreamRef xVBAStream;
    aVBAStrings.SetSize(1);
    aVBAStrings.Put(0,new String);

    xVBAStream = xVBA->OpenStream( pOffsets[nIndex].sName,
                        STREAM_STD_READ | STREAM_NOCREATE );
    if (pOverflow)
        *pOverflow=0;

    if( !xVBAStream.Is() || SVSTREAM_OK != xVBAStream->GetError() )
    {
        DBG_WARNING("Not able to open vb module ");
    }
    else
    {
        xVBAStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        DecompressVBA( nIndex, xVBAStream );
        /*
         * if len was too big for a single string set that variable ?
         *  if ((len > XX) && (pOverflow))
                *pOverflow=1;
         */
        if (bCommented)
        {
            for(int i=0;i<aVBAStrings.GetSize();i++)
            {
                String sTempStringa(
                    RTL_CONSTASCII_STRINGPARAM("\x0D\x0A"),
                    RTL_TEXTENCODING_MS_1252);
                String sTempStringb(
                    RTL_CONSTASCII_STRINGPARAM("\x0D\x0A"),
                    RTL_TEXTENCODING_MS_1252);
                sTempStringb+=sComment;
                aVBAStrings.Get(i)->SearchAndReplaceAll(
                    sTempStringa,sTempStringb);
                aVBAStrings.Get(i)->Insert(sComment,0);
            }
        }
    }
    return aVBAStrings;
}


int VBA_Impl::DecompressVBA( int nIndex, SvStorageStreamRef &xVBAStream )
{
    BYTE nLeadbyte;
    UINT16 nToken;
    unsigned int nPos = 0;
    int nLen, nDistance, nShift, nClean=1;

    //*pOut << "jumping to " << hex << offsets[nIndex].offset << endl;
    xVBAStream->Seek( pOffsets[ nIndex ].nOffset + 3 );

    while(xVBAStream->Read(&nLeadbyte,1))
    {
//      *pOut << "reading 8 data unit block beginning with " << nLeadbyte
//          << int(nLeadbyte) << " at pos " << xVBAStream->Tell() << " real pos "
//          << nPos << endl;
        for(int nPosition=0x01;nPosition < 0x100;nPosition=nPosition<<1)
        {
            //we see if the leadbyte has flagged this location as a dataunit
            //which is actually a token which must be looked up in the history
            if (nLeadbyte & nPosition)
            {
                *xVBAStream >> nToken;

                if (nClean == 0)
                    nClean=1;

                //For some reason the division of the token into the length
                //field of the data to be inserted, and the distance back into
                //the history differs depending on how full the history is
                int nPos2 = nPos%WINDOWLEN;
                if (nPos2 <= 0x10)
                    nShift = 12;
                else if (nPos2 <= 0x20)
                    nShift = 11;
                else if (nPos2 <= 0x40)
                    nShift = 10;
                else if (nPos2 <= 0x80)
                    nShift = 9;
                else if (nPos2 <= 0x100)
                    nShift = 8;
                else if (nPos2 <= 0x200)
                    nShift = 7;
                else if (nPos2 <= 0x400)
                    nShift = 6;
                else if (nPos2 <= 0x800)
                    nShift = 5;
                else
                    nShift = 4;

                int i;
                nLen=0;
                for(i=0;i<nShift;i++)
                    nLen |= nToken & (1<<i);

                //*pOut << endl << "match lookup token " << int(nToken) << "len " << int(nLen) << endl;

                nLen += 3;
                //*pOut << endl << "len is " << nLen << "shift is " << nShift << endl;

                nDistance = nToken >> nShift;
                //*pOut << "distance token shift is " << nDistance << " " << int(nToken) << " " << nShift << "pos is " << nPos << " " << xVBAStream->Tell() << endl;

                //read the len of data from the history, wrapping around the
                //WINDOWLEN boundary if necessary
                //data read from the history is also copied into the recent
                //part of the history as well.
                for (i = 0; i < nLen; i++)
                {
                    unsigned char c;
                    //*pOut << endl << (nPos%WINDOWLEN)-nDistance-1 << " " << nPos << " " << nDistance << endl;
                    c = aHistory[(nPos-nDistance-1)%WINDOWLEN];
                    aHistory[nPos%WINDOWLEN] = c;
                    nPos++;
                    //*pOut << "real pos is " << nPos << endl;
                    //
                    //temp removed
                    //*pOut << c ;
                }
            }
            else
            {
                // special boundary case code, not guarantueed to be correct
                // seems to work though, there is something wrong with the
                // compression scheme (or maybe a feature) where when
                // the data ends on a WINDOWLEN boundary and the excess
                // bytes in the 8 dataunit list are discarded, and not
                // interpreted as tokens or normal data.
                if ((nPos != 0) && ((nPos%WINDOWLEN) == 0) && (nClean))
                {
                    //*pOut << "at boundary position is " << nPosition << " " << xVBAStream->Tell() << " pos is " << nPos << endl;
                    //if (nPosition != 0x01)
                    //*pOut << "must restart by eating remainder single byte data units" << endl;
                    xVBAStream->SeekRel(2);
                    nClean=0;
                    Output(WINDOWLEN,aHistory);
                    break;
                }
                //This is the normal case for when the data unit is not a
                //token to be looked up, but instead some normal data which
                //can be output, and placed in the history.
                if (xVBAStream->Read(&aHistory[nPos%WINDOWLEN],1))
                {
                    nPos++;
                    //temp removed
                    //*pOut << aHistory[nPos++%WINDOWLEN];
                }
                if (nClean == 0)
                    nClean=1;
                //*pOut << "pos is " << nPos << " " << xVBAStream->Tell() << endl;
            }
        }
    }
    if (nPos%WINDOWLEN)
        Output(nPos%WINDOWLEN,aHistory);
    return(nPos);
}

