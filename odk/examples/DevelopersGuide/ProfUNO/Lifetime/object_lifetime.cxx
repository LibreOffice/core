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



#include <stdio.h>

#include <cppuhelper/weak.hxx>

class MyOWeakObject : public ::cppu::OWeakObject
{
public:
    MyOWeakObject() { fprintf( stdout, "constructed\n" ); }
    ~MyOWeakObject() { fprintf( stdout, "destructed\n" ); }
};


void simple_object_creation_and_destruction()
{
    // create the UNO object
    com::sun::star::uno::XInterface * p = new MyOWeakObject();

    // acquire it, refcount becomes one
    p->acquire();

    fprintf( stdout, "before release\n" );

    // release it, refcount drops to zero
    p->release();

    fprintf( stdout, "after release\n" );
}


int main( char * argv[] )
{
    simple_object_creation_and_destruction();
    return 0;
}
