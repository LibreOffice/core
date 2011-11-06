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



#ifndef _SAX_OBJECT_HXX_
#define _SAX_OBJECT_HXX_

#include <sal/types.h>
#include "sax/dllapi.h"

namespace sax
{

    /** simple base class to allow refcounting with rtl::Reference or css::uno::Reference */
    class SAX_DLLPUBLIC SaxObject
    {
    public:
        SaxObject();
        virtual ~SaxObject();
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();
    private:
        sal_uInt32 mnRefCount;
    };
}

#endif // _SAX_OBJECT_HXX_
