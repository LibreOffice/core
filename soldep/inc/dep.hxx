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



#ifndef _DEP_HXX
#define _DEP_HXX

#include <soldep/sstring.hxx>

class SByteExtStringList : public SByteStringList
{
    private:
        sal_Bool        bDeleteStrings;
    public:
        SByteExtStringList(sal_Bool bDelete = sal_True) : bDeleteStrings (bDelete) {}
        ~SByteExtStringList();
};

class VersionDepInfo : public SByteExtStringList
{
    private:
        ByteString*                 pVersion;
    public:
        VersionDepInfo() : pVersion (NULL) {}
        VersionDepInfo(const ByteString& rCurrentVersion) : pVersion (NULL) { pVersion = new ByteString(rCurrentVersion); }
        ~VersionDepInfo();
        void                Clear();
        const ByteString*   GetVersion() { return pVersion; }
        //void              SetVersion(ByteString* pStr) { pVersion = pStr; }
};

DECLARE_LIST( VersionDepInfoList_Impl, VersionDepInfo* )

class VersionDepInfoList : public VersionDepInfoList_Impl
{
    public:
        VersionDepInfoList () {}
        ~VersionDepInfoList () {}
        VersionDepInfo*     GetVersion (ByteString& rVersion);
        void                RemoveVersion (ByteString& rVersion);
        VersionDepInfo*     InsertVersion (ByteString& rVersion);

};


#endif
