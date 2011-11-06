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



#ifndef _SBXMSTRM_HXX
#define _SBXMSTRM_HXX 1

#include <tools/stream.hxx>
#include <basic/sbxdef.hxx>
#include <basic/sbxcore.hxx>

SV_DECL_REF(SbxMemoryStream)

class SbxMemoryStream : public SbxBase, public SvMemoryStream
{
  public:
    SbxMemoryStream(sal_uIntPtr nInitSize_=512, sal_uIntPtr nResize_=64) :
        SvMemoryStream(nInitSize_,nResize_) {}
    ~SbxMemoryStream();

    virtual SbxDataType  GetType()  const;
};

SV_IMPL_REF(SbxMemoryStream)

#endif
