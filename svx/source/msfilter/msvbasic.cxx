/*************************************************************************
 *
 *  $RCSfile: msvbasic.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:49:47 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <string.h>     // memset(), ...
#ifndef UNX
#include <io.h>         // access()
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>   //rtl_getTextEncodingFromWindowsCodePage
#endif

#ifndef _MSVBASIC_HXX
#include "msvbasic.hxx"
#endif

/*
A few urls which may in the future be of some use
http://www.virusbtn.com/vb2000/Programme/papers/bontchev.pdf
*/

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
const int MINVBASTRING = 6;

sal_uInt8 VBA_Impl::ReadPString(SvStorageStreamRef &xVBAProject,
    bool bIsUnicode)
{
    sal_uInt16 nIdLen, nOut16;
    sal_uInt8 nType = 0, nOut8;
    String sReference;

    *xVBAProject >> nIdLen;

    if (nIdLen < MINVBASTRING) //Error recovery
        xVBAProject->SeekRel(-2); //undo 2 byte len
    else
    {
        for(sal_uInt16 i=0; i < nIdLen / (bIsUnicode ? 2 : 1); i++)
        {
            if (bIsUnicode)
                *xVBAProject >> nOut16;
            else
            {
                *xVBAProject >> nOut8;
                nOut16 = nOut8;
            }
            sReference += nOut16;
            if (i==2)
            {
                if ((nOut16 == 'G') || (nOut16 == 'H') || (nOut16 == 'C') ||
                    nOut16 == 'D')
                {
                    nType = static_cast<sal_uInt8>(nOut16);
                }
                if (nType == 0)
                {
                    //Error recovery, 2byte len + 3 characters of used type
                    xVBAProject->SeekRel(-(2 + 3 * (bIsUnicode ? 2 : 1)));
                    break;
                }
            }
        }
        maReferences.push_back(sReference);
    }
    return nType;
}

void VBA_Impl::Output( int nLen, const sal_uInt8*pData )
{
    /*
    Each StarBasic module is tragically limited to the maximum len of a
    string and WordBasic is not, so each overlarge module must be split
    */
    String sTemp((const sal_Char *)pData, (xub_StrLen)nLen,
        meCharSet);
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
        return 0;
    }

    static const sal_uInt8 aKnownId[] = {0xCC, 0x61};
    sal_uInt8 aId[2];
    xVBAProject->Read( aId, sizeof(aId) );
    if (memcmp( aId, aKnownId, sizeof(aId)))
    {
        DBG_WARNING("unrecognized VBA macro project type");
        return 0;
    }

    static const sal_uInt8 aOffice2003LE[] =
    {
        0x76, 0x00, 0x00, 0x01, 0x00, 0xFF
    };

    static const sal_uInt8 aOfficeXPLE[] =
    {
        0x73, 0x00, 0x00, 0x01, 0x00, 0xFF
    };

    static const sal_uInt8 aOfficeXPBE[] =
    {
        0x63, 0x00, 0x00, 0x0E, 0x00, 0xFF
    };

    static const sal_uInt8 aOffice2000LE[] =
    {
        0x6D, 0x00, 0x00, 0x01, 0x00, 0xFF
    };
    static const sal_uInt8 aOffice98BE[] =
    {
        0x60, 0x00, 0x00, 0x0E, 0x00, 0xFF
    };
    static const sal_uInt8 aOffice97LE[] =
    {
        0x5E, 0x00, 0x00, 0x01, 0x00, 0xFF
    };
    sal_uInt8 aProduct[6];
    xVBAProject->Read( aProduct, sizeof(aProduct) );

    bool bIsUnicode;
    if (!(memcmp(aProduct, aOffice2003LE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        bIsUnicode = true;
    }
    else if (!(memcmp(aProduct, aOfficeXPLE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        bIsUnicode = true;
    }
    else if (!(memcmp(aProduct, aOfficeXPBE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
        mbMac = true;
        bIsUnicode = false;
    }
    else if (!(memcmp(aProduct, aOffice2000LE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        bIsUnicode = true;
    }
    else if (!(memcmp(aProduct, aOffice98BE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
        mbMac = true;
        bIsUnicode = false;
    }
    else if (!(memcmp(aProduct, aOffice97LE, sizeof(aProduct))))
    {
        xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        bIsUnicode = true;
    }
    else
    {
        switch (aProduct[3])
        {
            case 0x1:
                xVBAProject->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                bIsUnicode = true;
                DBG_ASSERT(!this, "unrecognized VBA macro version, report to cmc. Guessing at unicode little endian");
                break;
            case 0xe:
                xVBAProject->SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
                mbMac = true;
                bIsUnicode = false;
                DBG_ASSERT(!this, "unrecognized VBA macro version, report to cmc. Guessing at 8bit big endian");
                break;
            default:
                DBG_ASSERT(!this, "totally unrecognized VBA macro version, report to cmc");
                return 0;
        }
    }

    sal_uInt32 nLidA;  //Language identifiers
    sal_uInt32 nLidB;
    sal_uInt16 nCharSet;
    sal_uInt16 nLenA;
    sal_uInt32 nUnknownB;
    sal_uInt32 nUnknownC;
    sal_uInt16 nLenB;
    sal_uInt16 nLenC;
    sal_uInt16 nLenD;

    *xVBAProject >> nLidA >> nLidB >> nCharSet >> nLenA >> nUnknownB;
    *xVBAProject >> nUnknownC >> nLenB >> nLenC >> nLenD;

    meCharSet = rtl_getTextEncodingFromWindowsCodePage(nCharSet);

    DBG_ASSERT(meCharSet != RTL_TEXTENCODING_DONTKNOW,
                "don't know what vba charset to use");
    if (meCharSet == RTL_TEXTENCODING_DONTKNOW)
        meCharSet = RTL_TEXTENCODING_MS_1252;

    if (nLenD != 0x02)
    {
        DBG_WARNING("Warning VBA number is different, please report");
        return 0;
    }

    /*
    A sequence of string that are prepended with a len and then begin with G
    or H, there are also those that begin with C or D. If a string begins with
    C or D, it is really two strings, one right after the other.  Each string
    then has a 12 bytes suffix

    Recognizing the end of the sequence is done by finding a str len of < 6
    which does not appear to be the beginning of an object id. Admittedly this
    isn't a great test, but nothing in the header appears to count the number
    of strings, and nothing else seems to match. So it'll have to do, its
    protected by a number of secondry tests to prove its a valid string, and
    everything gives up if this isn't proven.
    */
    bool bPredictsTrailingTwenty = false;
    while (1)
    {
        sal_uInt8 nType = ReadPString(xVBAProject,bIsUnicode);
        //Type C and D seem to come as pairs, so skip the following one
        if (nType == 'C' || nType == 'D')
        {
            nType = ReadPString(xVBAProject,bIsUnicode);
            DBG_ASSERT( nType == 'C' || nType == 'D',
                "VBA: This must be a 'C' or 'D' string!" );
            if (nType != 'C' && nType != 'D')
                return 0;
        }
        if (!nType)
            break;
        xVBAProject->SeekRel(10);
        sal_uInt16 nPredictsTrailingTwenty;
        *xVBAProject >> nPredictsTrailingTwenty;
        if (nPredictsTrailingTwenty)
            bPredictsTrailingTwenty = true;
        if (bPredictsTrailingTwenty)
        {
            sal_uInt16 nTestIsNotString;
            *xVBAProject >> nTestIsNotString;
            if (nTestIsNotString < MINVBASTRING)
            {
                DBG_ASSERT(nTestIsNotString <= 1,
                    "Haven't seen a len like this in VBA, report to CMC");
                xVBAProject->SeekRel(18);
                bPredictsTrailingTwenty = false;
            }
            else
                xVBAProject->SeekRel(-2);
        }
    }

    sal_Int16 nInt16s;
    *xVBAProject >> nInt16s;
    DBG_ASSERT( nInt16s >= 0, "VBA: Bad no of records in VBA Project, panic!" );
    if (!nInt16s)
        return 0;

    xVBAProject->SeekRel(2*nInt16s);

    sal_Int16 nInt32s;
    *xVBAProject >> nInt32s;
    DBG_ASSERT( nInt32s >= 0, "VBA: Bad no of records in VBA Project, panic!" );
    if (!nInt32s)
        return 0;
    xVBAProject->SeekRel(4*nInt32s);

    xVBAProject->SeekRel(2);
    for(int k=0;k<3;k++)
    {
        sal_uInt16 nLen;
        *xVBAProject >> nLen;
        if (nLen != 0xFFFF)
            xVBAProject->SeekRel(nLen);
    }
    xVBAProject->SeekRel(100); //Seems fixed len

    *xVBAProject >> nOffsets;
    DBG_ASSERT( nOffsets != 0xFFFF, "VBA: Bad nOffsets, panic!!" );
    if ((nOffsets == 0xFFFF) || (nOffsets == 0))
        return 0;
    pOffsets = new VBAOffset_Impl[ nOffsets ];

    int i, j;
    for( i=0; i < nOffsets; i++)
    {
        sal_uInt16 nLen;
        *xVBAProject >> nLen;

        if (bIsUnicode)
        {
            sal_Unicode* pBuf = pOffsets[i].sName.AllocBuffer( nLen / 2 );
            xVBAProject->Read( (sal_Char*)pBuf, nLen  );

#ifdef __BIGENDIAN
            for( j = 0; j < nLen / 2; ++j, ++pBuf )
                *pBuf = SWAPSHORT( *pBuf );
#endif // ifdef __BIGENDIAN
        }
        else
        {
            ByteString aByteStr;
            sal_Char*  pByteData = aByteStr.AllocBuffer( nLen );
            sal_Size nWasRead = xVBAProject->Read( pByteData, nLen );
            if( nWasRead != nLen )
                aByteStr.ReleaseBufferAccess();
            pOffsets[i].sName += String( aByteStr, meCharSet);
        }

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

        *xVBAProject >> nLen;
        DBG_ASSERT( nLen == 0xFFFF, "VBA: Bad field in VBA Project, panic!!" );
        if ( nLen != 0xFFFF)
            return 0;

        xVBAProject->SeekRel(6);
        sal_uInt16 nOctects;
        *xVBAProject >> nOctects;
        for(j=0;j<nOctects;j++)
            xVBAProject->SeekRel(8);

        xVBAProject->SeekRel(5);
        //end section

        *xVBAProject >> pOffsets[i].nOffset;
        xVBAProject->SeekRel(2);
    }

    return nOffsets;
}

bool VBA_Impl::Open( const String &rToplevel, const String &rSublevel )
{
    /* beginning test for vba stuff */
    bool bRet = false;
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
                bRet = true;
        }
    }
    /* end test for vba stuff */
    return bRet;
}

const StringArray &VBA_Impl::Decompress(sal_uInt16 nIndex, int *pOverflow)
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
            String sTempStringa;
            if (mbMac)
                sTempStringa = String(RTL_CONSTASCII_STRINGPARAM("\x0D"));
            else
                sTempStringa = String(RTL_CONSTASCII_STRINGPARAM("\x0D\x0A"));
            String sTempStringb(sTempStringa);
            sTempStringb+=sComment;
            for(ULONG i=0;i<aVBAStrings.GetSize();i++)
            {
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
    sal_uInt8 nLeadbyte;
    sal_uInt16 nToken;
    unsigned int nPos = 0;
    int nLen, nDistance, nShift, nClean=1;

    xVBAStream->Seek( pOffsets[ nIndex ].nOffset + 3 );

    while(xVBAStream->Read(&nLeadbyte,1))
    {
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
                int nPos2 = nPos % nWINDOWLEN;
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

                nLen += 3;

                nDistance = nToken >> nShift;

                //read the len of data from the history, wrapping around the
                //nWINDOWLEN boundary if necessary data read from the history
                //is also copied into the recent part of the history as well.
                for (i = 0; i < nLen; i++)
                {
                    unsigned char c;
                    c = aHistory[(nPos-nDistance-1) % nWINDOWLEN];
                    aHistory[nPos % nWINDOWLEN] = c;
                    nPos++;
                }
            }
            else
            {
                // special boundary case code, not guarantueed to be correct
                // seems to work though, there is something wrong with the
                // compression scheme (or maybe a feature) where when the data
                // ends on a nWINDOWLEN boundary and the excess bytes in the 8
                // dataunit list are discarded, and not interpreted as tokens
                // or normal data.
                if ((nPos != 0) && ((nPos % nWINDOWLEN) == 0) && (nClean))
                {
                    xVBAStream->SeekRel(2);
                    nClean=0;
                    Output(nWINDOWLEN, aHistory);
                    break;
                }
                //This is the normal case for when the data unit is not a
                //token to be looked up, but instead some normal data which
                //can be output, and placed in the history.
                if (xVBAStream->Read(&aHistory[nPos % nWINDOWLEN],1))
                    nPos++;

                if (nClean == 0)
                    nClean=1;
            }
        }
    }
    if (nPos % nWINDOWLEN)
        Output(nPos % nWINDOWLEN,aHistory);
    return(nPos);
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
