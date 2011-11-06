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


#include  <salhelper/dynload.hxx>
#include <rtl/ustring>
#include <stdio.h>
#include "samplelib.hxx"


using namespace salhelper;
using namespace rtl;


class SampleLibLoader
    : public ::salhelper::ODynamicLoader<SampleLib_Api>
{
public:
    SampleLibLoader():
        ::salhelper::ODynamicLoader<SampleLib_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_MODULENAME( "samplelib") ) ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(SAMPLELIB_INIT_FUNCTION_NAME) ))
        {}

};


int main( int argc, char *argv[ ], char *envp[ ] )
{
    SampleLibLoader Loader;
    SampleLibLoader Loader2;
    Loader= Loader2;
    SampleLib_Api *pApi= Loader.getApi();

    sal_Int32 retint= pApi->funcA( 10);
    double retdouble= pApi->funcB( 3.14);


    return 0;
}
