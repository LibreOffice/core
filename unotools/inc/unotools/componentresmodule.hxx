/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentresmodule.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 09:32:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX
#define UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX

#ifndef COMPHELPER_INC_COMPHELPER_COMPONENTMODULE_HXX
#include <comphelper/componentmodule.hxx>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include <unotools/unotoolsdllapi.h>
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

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
        ModuleRes( USHORT _nId, OComponentResourceModule& _rModule ) : ResId( _nId, *_rModule.getResManager() ) { }
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
        ResClass( USHORT _nId ) : BaseClass( _nId, ModuleClass::getInstance() ) \
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

