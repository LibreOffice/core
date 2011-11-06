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

#include <rtl/ustring>
#include <stdio.h>
#include <typeinfo>
#include "samplelibrtti.hxx"


//using namespace salhelper;
using namespace rtl;



int main( int argc, char *argv[ ], char *envp[ ] )
{
    MyClassB b;
    MyClassA* pA= &b;
    // test the virtual function
    pA->funcA();

//  const type_info& i1= typeid ( b);
//  const type_info& i2= typeid( pA);

    if( typeid( b) == typeid( pA))
        printf("\nsame types");


    MyClassB* pB= dynamic_cast<MyClassB* >( pA);
        pB->funcA();
    return 0;
}
