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


// local includes
#include "export.hxx"

DECLARE_LIST( LngLineList, ByteString * )

#define LNG_OK              0x0000
#define LNG_FILE_NOTFOUND   0x0001
#define LNG_COULD_NOT_OPEN  0x0002
#define SDF_OK              0x0003
#define SDF_FILE_NOTFOUND   0x0004
#define SDF_COULD_NOT_OPEN  0x0005

//
// class LngParser
//

class LngParser
{
private:
    sal_uInt16 nError;
    LngLineList *pLines;
    ByteString sSource;
    sal_Bool bDBIsUTF8;
    sal_Bool bULF;
    bool bQuiet;
    std::vector<ByteString> aLanguages;

    void FillInFallbacks( ByteStringHashMap Text );
    bool isNextGroup(  ByteString &sGroup_out , ByteString &sLine_in);
    void ReadLine( const ByteString &sLine_in , ByteStringHashMap &rText_inout );
    void WriteSDF( SvFileStream &aSDFStream , ByteStringHashMap &rText_inout ,
                    const ByteString &rPrj ,
                    const ByteString &rRoot , const ByteString &sActFileName , const ByteString &sID );
public:
    LngParser( const ByteString &rLngFile, sal_Bool bUTF8, sal_Bool bULFFormat );
    ~LngParser();

    sal_Bool CreateSDF( const ByteString &rSDFFile, const ByteString &rPrj, const ByteString &rRoot );
    sal_Bool Merge( const ByteString &rSDFFile, const ByteString &rDestinationFile , const ByteString &rPrj );
};
