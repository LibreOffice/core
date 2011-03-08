/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
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
************************************************************************/

#ifndef SUBCOMPONENTRECOVERY_HXX
#define SUBCOMPONENTRECOVERY_HXX

#include "subcomponents.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/embed/XStorage.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= SubComponentRecovery
    //====================================================================
    class DBACCESS_DLLPRIVATE SubComponentRecovery
    {
    public:
        SubComponentRecovery( const ::comphelper::ComponentContext& i_rContext, const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& i_rController,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& i_rComponent )
            :m_rContext( i_rContext )
            ,m_xDocumentUI( i_rController, ::com::sun::star::uno::UNO_SET_THROW )
            ,m_xComponent( i_rComponent )
            ,m_eType( UNKNOWN )
            ,m_aCompDesc()
        {
            impl_identifyComponent_throw();
        }

        SubComponentRecovery( const ::comphelper::ComponentContext& i_rContext, const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& i_rController,
                const SubComponentType i_eType )
            :m_rContext( i_rContext )
            ,m_xDocumentUI( i_rController, ::com::sun::star::uno::UNO_SET_THROW )
            ,m_xComponent()
            ,m_eType( i_eType )
            ,m_aCompDesc()
        {
        }

        // only to be used after being constructed with a component
        void    saveToRecoveryStorage(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rRecoveryStorage,
                    MapCompTypeToCompDescs& io_mapCompDescs
                );

        // only to be used after being constructed with a type
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                recoverFromStorage(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rRecoveryStorage,
                    const ::rtl::OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        static const ::rtl::OUString getComponentsStorageName( const SubComponentType i_eType );

    private:
        void    impl_saveSubDocument_throw(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rObjectStorage
                );

        void    impl_saveQueryDesign_throw(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rObjectStorage
                );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                impl_recoverSubDocument_throw(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rRecoveryStorage,
                    const ::rtl::OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                impl_recoverQueryDesign_throw(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rRecoveryStorage,
                    const ::rtl::OUString& i_rComponentName,
                    const bool i_bForEditing
                );

        void    impl_identifyComponent_throw();

    private:
        const ::comphelper::ComponentContext&   m_rContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                                                m_xDocumentUI;
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                                                m_xComponent;
        SubComponentType                        m_eType;
        SubComponentDescriptor                  m_aCompDesc;
    };


//........................................................................
} // namespace dbaccess
//........................................................................

#endif // SUBCOMPONENTRECOVERY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
