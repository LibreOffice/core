/*************************************************************************
 *
 *  $RCSfile: msvbasic.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
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
    UINT16 idlen;
    BYTE type=0;
    *xVBAProject >> idlen;
    BYTE out;
    int i=0;
    if (idlen < 6)
    {
        type=0;
        xVBAProject->SeekRel(-2);
        return(type);
    }

    for(i=0;i<idlen/2;i++)
    {
        *xVBAProject >> out;
        xVBAProject->SeekRel(1);
        if (i==2)
        {
            type=out;
            if ((type != 'G') && (type != 'C'))
                type=0;
            if (type == 0)
            {
                xVBAProject->SeekRel(-8);
                break;
            }
        }
    }


    return(type);
}

void VBA_Impl::ConfirmFixedOctect(SvStorageStreamRef &xVBAProject)
{
    static const BYTE stest[8] =
        {
        0x06, 0x02, 0x01, 0x00, 0x08, 0x02, 0x00, 0x00
        };

    BYTE test[8];
    xVBAProject->Read(test,8);
    if (memcmp(stest,test,8) != 0)
        DBG_WARNING("Found a different octect, please report");
}

void VBA_Impl::Confirm12Zeros(SvStorageStreamRef &xVBAProject)
{
    static const BYTE stest[12]={0};
    BYTE test[12];
    xVBAProject->Read(test,12);
    if (memcmp(stest,test,12) != 0)
        DBG_WARNING("Found a Non Zero block, please report");
}

void VBA_Impl::ConfirmHalfWayMarker(SvStorageStreamRef &xVBAProject)
{
    static const BYTE stest[12]={0,0,0,0,0,0,0,0,0,0,1,0};
    BYTE test[12];
    xVBAProject->Read(test,12);
    if (memcmp(stest,test,12) != 0)
        DBG_WARNING("Found a different halfway marker, please report");
}

void VBA_Impl::ConfirmFixedMiddle(SvStorageStreamRef &xVBAProject)
{
    static const BYTE stest[20] =
    {
        0x00, 0x00, 0xe1, 0x2e, 0x45, 0x0d, 0x8f, 0xe0,
        0x1a, 0x10, 0x85, 0x2e, 0x02, 0x60, 0x8c, 0x4d,
        0x0b, 0xb4, 0x00, 0x00
    };

    BYTE test[20];
    xVBAProject->Read(test,20);
    if (memcmp(stest,test,20) != 0)
    {
        DBG_WARNING("Found a different middle marker, please report");
        xVBAProject->SeekRel(-20);
    }
}

void VBA_Impl::ConfirmFixedMiddle2(SvStorageStreamRef &xVBAProject)
{
    static const BYTE stest[20] =
    {
        0x00, 0x00, 0x2e, 0xc9, 0x27, 0x8e, 0x64, 0x12,
        0x1c, 0x10, 0x8a, 0x2f, 0x04, 0x02, 0x24, 0x00,
        0x9c, 0x02, 0x00, 0x00
    };

    BYTE test[20];
    xVBAProject->Read(test,20);
    if (memcmp(stest,test,20) != 0)
        {
        DBG_WARNING("Found a different middle2 marker, please report");
        xVBAProject->SeekRel(-20);
        }
}


void VBA_Impl::Output( int nLen, const BYTE *pData)
{
    sVBAString += String( (const sal_Char *)pData, nLen );
/*
//For debugging purposes
    for(int i=0;i<len;i++)
        *pOut << data[i];
*/
}


int VBA_Impl::ReadVBAProject(const SvStorageRef &rxVBAStorage)
    {
    SvStorageStreamRef xVBAProject;
    xVBAProject = rxVBAStorage->OpenStream( "_VBA_PROJECT",
                    STREAM_STD_READ | STREAM_NOCREATE );

    if( !xVBAProject.Is() || SVSTREAM_OK != xVBAProject->GetError() )
    {
        DBG_WARNING("Not able to find vba project, cannot find macros");
        return(0);
    }
    xVBAProject->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    //*pOut << hex;
    BYTE header[30] =
    {
        0xcc, 0x61, 0x5e, 0x00, 0x00, 0x01, 0x00, 0xff,
        0x07, 0x04, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00,
        0xe4, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00
    };
    BYTE headerin[30];

    xVBAProject->Read(headerin,30);
    if (memcmp(header,headerin,30) != 0)
        DBG_WARNING("Warning VBA header is different, please report");
    UINT16 value;
    *xVBAProject >> value;
    //*pOut << "Trigger value 1 is " << value << endl;
    UINT16 svalue;
    *xVBAProject >> svalue;
    if (svalue != 0x02)
        DBG_WARNING("Warning VBA number is different, please report");

    int count=0;
    BYTE testc=0;

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
    while ((testc = ReadPString(xVBAProject)) != 0)
    {
        //*pOut << endl;
        //*pOut << "testcharacter is " << testc << endl;
        switch (testc)
        {
            case 'C':
                count++;
                if (count == 2)
                {
                    Confirm12Zeros(xVBAProject);
                    count=0;
                }
                break;
            default:
            case 'G':
                Confirm12Zeros(xVBAProject);
                break;
        }
    }

    //appears to be a fixed 20 byte sequence here, and then the strings
    //continue
    ConfirmFixedMiddle(xVBAProject);

    count=0;
    testc=0;

    while ((testc = ReadPString(xVBAProject)) != 0)
    {
        //*pOut << endl;
        //*pOut << "testcharacter is " << testc << endl;
        switch (testc)
        {
            case 'C':
                count++;
                if (count == 2)
                {
                    Confirm12Zeros(xVBAProject);
                    count=0;
                }
                break;
            default:
            case 'G':
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

    UINT16 junksize;
    while(junksize != 0xFFFF)
    {
        xVBAProject->Read(&junksize,2); // usually 18 02, sometimes 1e 02
        //but sometimes its a run of numbers until 0xffff, gagh!!!
        //*pOut << "position is " << xVBAProject->Tell() << "len is "
        //  << junksize << endl;
    }

    UINT16 ftest;
    *xVBAProject >> ftest;
    if (ftest != 0xFFFF)
        xVBAProject->SeekRel(ftest);
    *xVBAProject >> ftest;
    if (ftest != 0xFFFF)
        xVBAProject->SeekRel(ftest);

    xVBAProject->SeekRel(100);
    //*pOut << "position is " << xVBAProject->Tell() << endl;
    //end section


    *xVBAProject >> nOffsets;
    pOffsets = new VBAOffset_Impl[nOffsets];
    int i;
    for (i=0;i<nOffsets;i++)
        {
        BYTE discard;
        UINT16 len;
        *xVBAProject >> len;
        int j;
        for (j=0;j<len/2;j++)
            {
            *xVBAProject >> discard;
            pOffsets[i].sName += discard;
            *xVBAProject >> discard;
            }
        *xVBAProject >> len;
        xVBAProject->SeekRel(len);

        //begin section, another problem area
        *xVBAProject >> len;
        if (len == 0xFFFF)
        {
            xVBAProject->SeekRel(2);
            *xVBAProject >> len;
            xVBAProject->SeekRel(len);
        }
        else
            xVBAProject->SeekRel(len+2);
        //
        /* I have a theory that maybe you read a 16bit len, and
         * if it has 0x02 for the second byte then it is a special
         * token of its own that affects nothing else, otherwise
         * it is a len of the following data. C. I must test this
         * theory later.
         */
        //end section

        xVBAProject->SeekRel(8);
        BYTE no_of_octects;
        *xVBAProject >> no_of_octects;
        for(j=0;j<no_of_octects;j++)
            xVBAProject->SeekRel(8);
        xVBAProject->SeekRel(6);

        *xVBAProject >> pOffsets[i].nOffset;
        //*pOut << pOffsets[i].pName.GetStr() << " at 0x" << hex << pOffsets[i].nOffset << endl;
        xVBAProject->SeekRel(2);
        }

    //*pOut << endl;
    return(nOffsets);
    }

BOOL VBA_Impl::Open( const String &rToplevel,const String &rSublevel )
{
    /* beginning test for vba stuff */
    BOOL bRet = FALSE;
    SvStorageRef xMacros= xStor->OpenStorage(rToplevel);
    if( !xMacros.Is() || SVSTREAM_OK != xMacros->GetError() )
    {
        DBG_WARNING("No Macros Storage");
    }
    else
    {
        xVBA = xMacros->OpenStorage(rSublevel);
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

const String &VBA_Impl::Decompress( UINT16 nIndex, int *pOverflow)
{
    SvStorageStreamRef xVBAStream;
    sVBAString.Erase();

    DBG_ASSERT( nIndex < nOffsets, "Index out of range" );
    xVBAStream = xVBA->OpenStream( pOffsets[nIndex].sName,
                        STREAM_STD_READ | STREAM_NOCREATE );
    if (pOverflow)
        *pOverflow=0;
    if( !xVBAStream.Is() || SVSTREAM_OK !=
        xVBAStream->GetError() )
    {
        DBG_WARNING("Not able to open vb module ");
        DBG_WARNING((pOffsets[nIndex].sName).GetStr());
    }
    else
    {
        xVBAStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        DecompressVBA(nIndex,xVBAStream);
        /*
         * if len was too big for a single string set that variable ?
         *  if ((len > XX) && (pOverflow))
                *pOverflow=1;
         */
        if (bCommented)
        {
            String sTempStringa("\x0D\x0A");
            String sTempStringb("\x0D\x0ARem ");
            sVBAString.SearchAndReplaceAll(sTempStringa,sTempStringb);
            sVBAString.Insert("Rem ",0);
        }
    }
    return sVBAString;
}


int VBA_Impl::DecompressVBA( int nIndex, SvStorageStreamRef &xVBAStream )
{
    BYTE leadbyte;
    unsigned int pos = 0;

    //*pOut << "jumping to " << hex << offsets[nIndex].offset << endl;
    xVBAStream->Seek(pOffsets[nIndex].nOffset+3);

    int len;
    UINT16 token;
    int distance, shift, clean=1;

    while(xVBAStream->Read(&leadbyte,1))
        {
        //*pOut << "reading 8 data unit block beginning with " << leadbyte << int(leadbyte) << " at pos " << xVBAStream->Tell() << " real pos " << pos << endl;
        for(int position=0x01;position < 0x100;position=position<<1)
            {
            //we see if the leadbyte has flagged this location as a dataunit
            //which is actually a token which must be looked up in the history
            if (leadbyte & position)
                {
                *xVBAStream >> token;

                if (clean == 0)
                    clean=1;

                //For some reason the division of the token into the length
                //field of the data to be inserted, and the distance back into
                //the history differs depending on how full the history is
                int pos2 = pos%WINDOWLEN;
                if (pos2 <= 0x10)
                    shift = 12;
                else if (pos2 <= 0x20)
                    shift = 11;
                else if (pos2 <= 0x40)
                    shift = 10;
                else if (pos2 <= 0x80)
                    shift = 9;
                else if (pos2 <= 0x100)
                    shift = 8;
                else if (pos2 <= 0x200)
                    shift = 7;
                else if (pos2 <= 0x400)
                    shift = 6;
                else if (pos2 <= 0x800)
                    shift = 5;
                else
                    shift = 4;

                int i;
                len=0;
                for(i=0;i<shift;i++)
                    len |= token & (1<<i);

                //*pOut << endl << "match lookup token " << int(token) << "len " << int(len) << endl;

                len += 3;
                //*pOut << endl << "len is " << len << "shift is " << shift << endl;

                distance = token >> shift;
                //*pOut << "distance token shift is " << distance << " " << int(token) << " " << shift << "pos is " << pos << " " << xVBAStream->Tell() << endl;

                //read the len of data from the history, wrapping around the
                //WINDOWLEN boundary if necessary
                //data read from the history is also copied into the recent
                //part of the history as well.
                for (i = 0; i < len; i++)
                    {
                    unsigned char c;
                    //*pOut << endl << (pos%WINDOWLEN)-distance-1 << " " << pos << " " << distance << endl;
                    c = aHistory[(pos-distance-1)%WINDOWLEN];
                    aHistory[pos%WINDOWLEN] = c;
                    pos++;
                    //*pOut << "real pos is " << pos << endl;
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
                if ((pos != 0) && ((pos%WINDOWLEN) == 0) && (clean))
                    {
                    //*pOut << "at boundary position is " << position << " " << xVBAStream->Tell() << " pos is " << pos << endl;
                    //if (position != 0x01)
                    //*pOut << "must restart by eating remainder single byte data units" << endl;
                    xVBAStream->SeekRel(2);
                    clean=0;
                    Output(WINDOWLEN,aHistory);
                    break;
                    }
                //This is the normal case for when the data unit is not a
                //token to be looked up, but instead some normal data which
                //can be output, and placed in the history.
                if (xVBAStream->Read(&aHistory[pos%WINDOWLEN],1))
                {
                    pos++;
                    //temp removed
                    //*pOut << aHistory[pos++%WINDOWLEN];
                }
                if (clean == 0)
                    clean=1;
                //*pOut << "pos is " << pos << " " << xVBAStream->Tell() << endl;
                }
            }
        }
    if (pos%WINDOWLEN)
        Output(pos%WINDOWLEN,aHistory);
    return(pos);
}

/*************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/msvbasic.cxx,v 1.1.1.1 2000-09-18 17:14:59 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.5  2000/09/18 16:05:03  willem.vandorp
    OpenOffice header added.

    Revision 1.4  2000/02/03 09:31:52  cmc
    #72268# Office 2000 compatibility bug fixed

    Revision 1.2  2000/01/26 16:40:15  cmc
    Made VBA Project stream reader more robust

    Revision 1.1  2000/01/26 12:34:58  jp
    #72268#: move form svdraw to here

    Revision 1.4  2000/01/25 16:12:21  rt
    #65293# includes unnecessary

    Revision 1.3  2000/01/25 11:17:42  cmc
    #72268# Visual Basic Decompression


*************************************************************************/
