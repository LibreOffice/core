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



 #ifndef IINTERFACE_HXX_INCLUDED
 #define IINTERFACE_HXX_INCLUDED

 #include <sal/types.h>

 /** The base interface
 */
 class IInterface
 {
 public:

    /** Acquire a reference to an instance. A caller shall release
        the instance by calling 'release' when it is no longer needed.
        'acquire' and 'release' calls need to be balanced.

        @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 acquire() = 0;

    /** Releases a reference to an instance. A caller has to call
        'release' when a before acquired reference to an instance
        is no longer needed. 'acquire' and 'release' calls need to
        be balanced.

    @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 release() = 0;

    /** Returns the current reference count. This method should be used for
        debugging purposes. Using it otherwise is a signal of a design flaw.
    */
    virtual sal_Int32 getReferenceCount() const = 0;

protected:
    virtual ~IInterface() {};
 };

 #endif // IDOCUMENT_HXX_INCLUDED
