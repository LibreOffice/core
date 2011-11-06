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



#ifndef SVTOOLS_ACCESSIBLE_FACTORY_ACCESS_HXX
#define SVTOOLS_ACCESSIBLE_FACTORY_ACCESS_HXX

#include <svtools/accessiblefactory.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= AccessibleFactoryAccess
    //====================================================================
    /** a client for the accessibility implementations which have been outsourced
        from the main svtools library

        All instances of this class share a reference to a common IAccessibleFactory
        instance, which is used for creating all kind of Accessibility related
        components.

        When the AccessibleFactoryAccess goes aways, this factory goes aways, to, and the respective
        library is unloaded.

        This class is not thread-safe.
    */
    class AccessibleFactoryAccess
    {
    private:
        bool    m_bInitialized;

    public:
        AccessibleFactoryAccess();
        ~AccessibleFactoryAccess();

        IAccessibleFactory& getFactory();

    private:
        void ensureInitialized();
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_ACCESSIBLE_FACTORY_ACCESS_HXX

