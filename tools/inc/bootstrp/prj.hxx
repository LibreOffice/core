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



#ifndef _PRJ_HXX
#define _PRJ_HXX

#include <tools/fsys.hxx>
#include <tools/stream.hxx>

/*********************************************************************
*
*   Die Klasse SimpleConfig kann benutzt werden, um aus einer Textdatei
*   alle Tokens zu lesen
*
*********************************************************************/

class SimpleConfig
{
    long            nLine;
    String          aFileName;
    SvFileStream    aFileStream;
    ByteString          aTmpStr;
    ByteString          aStringBuffer;

    ByteString          GetNextLine();
public:
                    SimpleConfig(String aSimpleConfigFileName);
                    SimpleConfig(DirEntry& rDirEntry);
                    ~SimpleConfig();
    ByteString          GetNext();
    ByteString          GetCleanedNextLine( sal_Bool bReadComments = sal_False );
};

#endif
