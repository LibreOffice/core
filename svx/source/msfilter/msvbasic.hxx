/*************************************************************************
 *
 *  $RCSfile: msvbasic.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:22 $
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
#ifndef _MSVBASIC_HXX
#define _MSVBASIC_HXX

#ifdef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _DYNARY_HXX
#include <tools/dynary.hxx>
#endif


/* class VBA:
 * The VBA class provides a set of methods to handle Visual Basic For
 * Applications streams, the constructor is given the root ole2 stream
 * of the document, Open reads the VBA project file and figures out
 * the number of VBA streams, and the offset of the data within them.
 * Decompress decompresses a particular numbered stream, NoStreams returns
 * this number, and StreamName can give you the streams name. Decompress
 * will return a string with the decompressed data. The optional extra
 * argument will be set if not NULL to 1 in the case of a string overflow,
 * if I can figure out how to do that.
 *
 * Otherwise it is possible to inherit from VBA and implement a Output
 * member which gets called with each 4096 output sized block.
 *
 * cmc
 * */

#define WINDOWLEN 4096
DECLARE_DYNARRAY(StringArray,String *);

class VBA_Impl
{
public:
        VBA_Impl( SvStorage &rIn, BOOL bCmmntd = TRUE )
            : xStor(&rIn), pOffsets(0), nOffsets(0),
            bCommented(bCmmntd),aVBAStrings(0),nLines(0),
            sComment(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Rem ")))
            {}
        ~VBA_Impl()
            {
            if (nOffsets)
                delete [] pOffsets;
            for(int i=0;i<aVBAStrings.GetSize();i++)
                delete aVBAStrings.Get(i);
            }
        //0 for failure, 1 for success
        BOOL Open( const String &rToplevel, const String &rSublevel);
        const StringArray & Decompress(UINT16 nIndex, int *pOverflow=0);
        UINT16 GetNoStreams() const { return nOffsets; }
        const String &GetStreamName( UINT16 nIndex ) const
            {
                DBG_ASSERT( nIndex < nOffsets, "Index out of range" );
                return pOffsets[ nIndex ].sName;
            }
        virtual void Output(int len,const BYTE *data);
private:
        struct VBAOffset_Impl
        {
            String sName;
            UINT32 nOffset;
        };

        SvStorageRef xVBA;
        StringArray aVBAStrings;
        String sComment;
        SvStorageRef xStor;
        VBAOffset_Impl *pOffsets;
        UINT16 nOffsets;
        BYTE aHistory[ WINDOWLEN ];
        BOOL bCommented;
        int nLines;

        //0 for failure, anything else for success
        int ReadVBAProject(const SvStorageRef &rxVBAStorage);
        int DecompressVBA(int index, SvStorageStreamRef &rxVBAStream);
        void Confirm12Zeros(SvStorageStreamRef &xVBAProject);
        void ConfirmHalfWayMarker(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedMiddle(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedMiddle2(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedOctect(SvStorageStreamRef &xVBAProject);
        BYTE ReadPString(SvStorageStreamRef &xVBAProject);
};




#endif
