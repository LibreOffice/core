/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appbaslib.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 17:03:52 $
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

#ifndef APPBASLIB_HXX
#define APPBASLIB_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSTORAGEBASEDLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

class BasicManager;

/** helper class which holds and manipulates a BasicManager
*/
class SfxBasicManagerHolder
{
private:
    BasicManager*   mpBasicManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >
                    mxBasicContainer;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >
                    mxDialogContainer;

public:
    SfxBasicManagerHolder();

    enum ContainerType
    {
        SCRIPTS, DIALOGS
    };

    /** returns <TRUE/> if and only if the instance is currently bound to a non-<NULL/>
        BasicManager.
    */
    bool    isValid() const { return mpBasicManager != NULL; }

    /** returns the BasicManager which this instance is currently bound to
    */
    BasicManager*
            get() const { return mpBasicManager; }

    /** binds the instance to the given BasicManager
    */
    void    reset( BasicManager* _pBasicManager );

    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
            getLibraryContainer( ContainerType _eType );

    /** determines whether any of our library containers is modified, i.e. returns <TRUE/>
        in its isContainerModified call.
    */
    bool    isAnyContainerModified() const;

    /** calls the storeLibraries at both our script and basic library container
    */
    void    storeAllLibraries();

    /** calls the setStorage at all our XStorageBasedLibraryContainer.
    */
    void    setStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            );

    /** calls the storeLibrariesToStorage at all our XStorageBasedLibraryContainer.
    */
    void    storeLibrariesToStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            );


    /** checks if any modules in the SfxLibraryContainer exceed the binary
        limits.
    */
    sal_Bool LegacyPsswdBinaryLimitExceeded( ::com::sun::star::uno::Sequence< rtl::OUString >& sModules );


private:
    void    impl_releaseContainers();

    bool    impl_getContainer(
                ContainerType _eType,
                ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >& _out_rxContainer );
};

class SfxApplicationScriptLibraryContainer
{
public:
    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();
    static ::rtl::OUString impl_getStaticImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

};

class SfxApplicationDialogLibraryContainer
{
public:
    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();
    static ::rtl::OUString impl_getStaticImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

};

#endif // APPBASLIB_HXX

