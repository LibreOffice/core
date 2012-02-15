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



#ifndef COMPHELPER_FLAGGUARD_HXX
#define COMPHELPER_FLAGGUARD_HXX

#include "comphelper/scopeguard.hxx"

//......................................................................................................................
namespace comphelper
{
//......................................................................................................................

    //==================================================================================================================
    //= FlagRestorationGuard
    //==================================================================================================================
    class COMPHELPER_DLLPUBLIC FlagRestorationGuard : public ScopeGuard
    {
    public:
        FlagRestorationGuard( bool& i_flagRef, bool i_temporaryValue, exc_handling i_excHandling = IGNORE_EXCEPTIONS )
            :ScopeGuard( ::boost::bind( RestoreFlag, ::boost::ref( i_flagRef ), !!i_flagRef ), i_excHandling )
        {
            i_flagRef = i_temporaryValue;
        }

        ~FlagRestorationGuard();

    private:
        static void RestoreFlag( bool& i_flagRef, bool i_originalValue )
        {
            i_flagRef = i_originalValue;
        }
    };

    //==================================================================================================================
    //= FlagGuard
    //==================================================================================================================
    class COMPHELPER_DLLPUBLIC FlagGuard : public ScopeGuard
    {
    public:
        explicit FlagGuard( bool& i_flagRef, exc_handling i_excHandling = IGNORE_EXCEPTIONS )
            :ScopeGuard( ::boost::bind( ResetFlag, ::boost::ref( i_flagRef ) ), i_excHandling )
        {
            i_flagRef = true;
        }

        ~FlagGuard();

    private:
        static void ResetFlag( bool& i_flagRef )
        {
            i_flagRef = false;
        }
    };

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................

#endif // COMPHELPER_FLAGGUARD_HXX
