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


#ifndef L10NTOOLS_SRCITER_HXX
#define L10NTOOLS_SRCITER_HXX

#include <l10ntools/directory.hxx>

// class SourceTreeIterator

class SourceTreeIterator
{
private:
    transex::Directory aRootDirectory;
    bool bInExecute;

    void ExecuteDirectory( transex::Directory& pDirectory );

protected:
    bool bLocal;
    bool bSkipLinks;

public:
    SourceTreeIterator( const ByteString &rRootDirectory, const ByteString &rVersion , bool bLocal_in = false);
    virtual ~SourceTreeIterator();

    sal_Bool StartExecute();
    void EndExecute();

    virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};

#endif // L10NTOOLS_SRCITER_HXX

