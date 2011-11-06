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




#ifndef SC_BASE_HXX
#define SC_BASE_HXX

#include <sal/config.h>
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include <tools/string.hxx>

#include <tools/color.hxx>
#include "flttypes.hxx"
#include "ftools.hxx"

// Stream wrapper class
class ScBiffReader
{
    protected:
    sal_uInt16 mnId;
    sal_uInt16 mnLength;
    sal_uInt32 mnOffset;
    SvStream *mpStream;
    bool mbEndOfFile;

    public:
    ScBiffReader( SfxMedium& rMedium );
    ~ScBiffReader();
    bool recordsLeft() { return mpStream && !mpStream->IsEof(); }
    bool IsEndOfFile() { return mbEndOfFile; }
    void SetEof( bool bValue ){ mbEndOfFile = bValue; }
    bool nextRecord();
    sal_uInt16 getId() { return mnId; }
    sal_uInt16 getLength() { return mnLength; }
    SvStream& getStream() { return *mpStream; }
};
#endif

