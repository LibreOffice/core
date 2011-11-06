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


#ifndef _MSVBASIC_HXX
#define _MSVBASIC_HXX

#ifdef _SOLAR_H
#include <tools/solar.h>
#endif
#include <tools/debug.hxx>
#include <sot/storage.hxx>


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

class VBA_Impl
{
public:
        VBA_Impl( SvStorage &rIn, sal_Bool bCmmntd = sal_True )
            : xStor(&rIn), pOffsets(0), nOffsets(0), bCommented(bCmmntd)
            {}
        ~VBA_Impl() {if (nOffsets) delete [] pOffsets;}
        //0 for failure, 1 for success
        sal_Bool Open( const String &rToplevel, const String &rSublevel);
        const String & Decompress( sal_uInt16 nIndex, int *pOverflow=0);
        sal_uInt16 GetNoStreams() const                 { return nOffsets; }
        const String &GetStreamName( sal_uInt16 nIndex ) const
            {
                DBG_ASSERT( nIndex < nOffsets, "Index out of range" );
                return pOffsets[ nIndex ].sName;
            }
        virtual void Output(int len,const sal_uInt8 *data);
private:
        struct VBAOffset_Impl
        {
            String sName;
            sal_uInt32 nOffset;
        };

        SvStorageRef xVBA;
        String sVBAString;
        SvStorageRef xStor;
        VBAOffset_Impl *pOffsets;
        sal_uInt16 nOffsets;
        sal_uInt8 aHistory[ WINDOWLEN ];
        sal_Bool bCommented;

        //0 for failure, anything else for success
        int ReadVBAProject(const SvStorageRef &rxVBAStorage);
        int DecompressVBA(int index, SvStorageStreamRef &rxVBAStream);
        void Confirm12Zeros(SvStorageStreamRef &xVBAProject);
        void ConfirmHalfWayMarker(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedMiddle(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedMiddle2(SvStorageStreamRef &xVBAProject);
        void ConfirmFixedOctect(SvStorageStreamRef &xVBAProject);
        sal_uInt8 ReadPString(SvStorageStreamRef &xVBAProject);
};




#endif
