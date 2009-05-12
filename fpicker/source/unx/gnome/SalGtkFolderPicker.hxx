/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SalGtkFolderPicker.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SALGTKFOLDERPICKER_HXX_
#define _SALGTKFOLDERPICKER_HXX_

//_______________________________________________________________________________________________________________________
//  includes of other projects
//_______________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _SALGTKPICKER_HXX_
#include "SalGtkPicker.hxx"
#endif

#include <memory>

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.hxx>
#endif

#include <list>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalGtkFolderPicker :
        public SalGtkPicker,
    public cppu::WeakImplHelper3<
    ::com::sun::star::ui::dialogs::XFolderPicker,
    ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::util::XCancellable >
{
    public:

        // constructor
        SalGtkFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

        //------------------------------------------------------------------------------------
        // XExecutableDialog functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int16 SAL_CALL execute(  )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XFolderPicker functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL getDirectory( )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDescription( const rtl::OUString& rDescription )
            throw( com::sun::star::uno::RuntimeException );


        //------------------------------------------------
        // XServiceInfo
        //------------------------------------------------

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        //------------------------------------------------
        // XCancellable
        //------------------------------------------------

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------
        // XEventListener
        //------------------------------------------------

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::uno::RuntimeException);

    private:
        // prevent copy and assignment
        SalGtkFolderPicker( const SalGtkFolderPicker& );
        SalGtkFolderPicker& operator=( const SalGtkFolderPicker& );
    private:
        // to instanciate own services
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;

#if 0
    public:
         virtual ~SalGtkFolderPicker();
#endif
};

#endif // _SALGTKFOLDERPICKER_HXX_
