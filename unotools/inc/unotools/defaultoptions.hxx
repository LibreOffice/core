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


#ifndef INCLUDED_unotools_DEFAULTOPTIONS_HXX
#define INCLUDED_unotools_DEFAULTOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include "tools/solar.h"
#include <unotools/options.hxx>

class String;
class SvtDefaultOptions_Impl;

// class SvtDefaultOptions -----------------------------------------------

class SvtDefaultOptions_Impl;
class UNOTOOLS_DLLPUBLIC SvtDefaultOptions: public utl::detail::Options
{
private:
    SvtDefaultOptions_Impl* pImp;

public:

    SvtDefaultOptions();
    virtual ~SvtDefaultOptions();

    String  GetDefaultPath( sal_uInt16 nId ) const;
};

#endif // #ifndef INCLUDED_unotools_DEFAULTOPTIONS_HXX

