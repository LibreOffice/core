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


#ifndef UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX
#define UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX

#include <comphelper/componentmodule.hxx>
#include <unotools/unotoolsdllapi.h>

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/resid.hxx>

#include <memory>

class ResMgr;

//........................................................................
namespace utl
{
//........................................................................

    class OComponentResModuleImpl;

    //====================================================================
    //= OComponentResourceModule
    //====================================================================
    /** extends the <type scope="comphelper">OModule</type> implementation with
        simply resource access
    */
    class UNOTOOLS_DLLPUBLIC OComponentResourceModule : public ::comphelper::OModule
    {
    private:
        typedef ::comphelper::OModule   BaseClass;

    private:
        ::std::auto_ptr< OComponentResModuleImpl >  m_pImpl;

    public:
        OComponentResourceModule( const ::rtl::OString& _rResFilePrefix );
        ~OComponentResourceModule();

        /// get the vcl res manager of the module
        ResMgr* getResManager();

    protected:
        // OModule overridables
        virtual void onFirstClient();
        virtual void onLastClient();
    };

    //=========================================================================
    //= ModuleRes
    //=========================================================================
    /** specialized ResId, using the ressource manager provided by a given <type>OModule</type>
    */
    class UNOTOOLS_DLLPUBLIC ModuleRes : public ::ResId
    {
    public:
        ModuleRes( sal_uInt16 _nId, OComponentResourceModule& _rModule ) : ResId( _nId, *_rModule.getResManager() ) { }
    };

    //====================================================================
    //= defining a concrete module
    //====================================================================
#define DEFINE_MODULE( ModuleClass, ClientClass, ResClass ) \
    /* -------------------------------------------------------------------- */ \
    class ModuleClass : public ::utl::OComponentResourceModule \
    { \
        friend struct CreateModuleClass; \
        typedef ::utl::OComponentResourceModule BaseClass; \
    \
    public: \
        static ModuleClass& getInstance(); \
    \
    private: \
        ModuleClass(); \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    class ClientClass : public ::comphelper::OModuleClient \
    { \
    private: \
        typedef ::comphelper::OModuleClient BaseClass; \
    \
    public: \
        ClientClass() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    class ResClass : public ::utl::ModuleRes \
    { \
    private: \
        typedef ::utl::ModuleRes    BaseClass; \
    \
    public: \
        ResClass( sal_uInt16 _nId ) : BaseClass( _nId, ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OAutoRegistration : public ::comphelper::OAutoRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OAutoRegistration< TYPE >    BaseClass; \
    \
    public: \
        OAutoRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OSingletonRegistration : public ::comphelper::OSingletonRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OSingletonRegistration< TYPE >    BaseClass; \
    \
    public: \
        OSingletonRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OLegacySingletonRegistration : public ::comphelper::OLegacySingletonRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OLegacySingletonRegistration< TYPE >    BaseClass; \
    \
    public: \
        OLegacySingletonRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    };


    //====================================================================
    //= implementing a concrete module
    //====================================================================
#define IMPLEMENT_MODULE( ModuleClass, resprefix ) \
    struct CreateModuleClass \
    { \
        ModuleClass* operator()() \
        { \
            static ModuleClass* pModule = new ModuleClass; \
            return pModule; \
            /*  yes, in theory, this is a resource leak, since the ModuleClass \
                will never be cleaned up. However, using a non-heap instance of ModuleClass \
                would not work: It would be cleaned up when the module is unloaded. \
                This might happen (and is likely to happen) *after* the tools-library \
                has been unloaded. However, the module's dtor is where we would delete \
                our resource manager (in case not all our clients de-registered) - which \
                would call into the already-unloaded tools-library. */ \
        } \
    }; \
    \
    ModuleClass::ModuleClass() \
        :BaseClass( ::rtl::OString( resprefix ) ) \
    { \
    } \
    \
    ModuleClass& ModuleClass::getInstance() \
    { \
        return *rtl_Instance< ModuleClass, CreateModuleClass, ::osl::MutexGuard, ::osl::GetGlobalMutex >:: \
            create( CreateModuleClass(), ::osl::GetGlobalMutex() ); \
    } \

//........................................................................
} // namespace utl
//........................................................................

#endif // UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX

