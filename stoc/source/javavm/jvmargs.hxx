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




#ifndef __JVM_HXX
#define __JVM_HXX


#include <vector>
#include <rtl/ustring.hxx>

#include "jni.h"


extern "C" {
    typedef jint JNICALL JNI_InitArgs_Type(void *);
    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);

}

namespace stoc_javavm {

    class JVM {
        ::std::vector<rtl::OUString> _props;

    public:
        JVM() throw();

        void pushProp(const ::rtl::OUString & uString);
        const ::std::vector< ::rtl::OUString> & getProperties() const;
    };
}

#endif
