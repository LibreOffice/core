/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
************************************************************************/

#ifndef DBACCESS_SUBCOMPONENTMANAGER_HXX
#define DBACCESS_SUBCOMPONENTMANAGER_HXX

#include "AppElementType.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XController.hpp>
/** === end UNO includes === **/

#include <comphelper/sharedmutex.hxx>
#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    struct SubComponentManager_Data;
    class OApplicationController;

    //====================================================================
    //= SubComponentManager
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   SubComponentManager_Base;
    class SubComponentManager : public SubComponentManager_Base
    {
    public:
        SubComponentManager( OApplicationController& _rController, const ::comphelper::SharedMutex& _rMutex );
        virtual ~SubComponentManager();

        void    disposing();

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XDatabaseDocumentUI helpers
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> >
                    getSubComponents() const;
        sal_Bool    closeSubComponents();

        // container access
        void        onSubComponentOpened(
                        const ::rtl::OUString&  _rName,
                        const sal_Int32         _nComponentType,
                        const ElementOpenMode   _eOpenMode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&
                                                _rxComponent
                    );
        bool        empty() const;

        /** activates (i.e. brings to top) the frame in which the given component is loaded, if any

            @return
                <TRUE/> if any only of such a frame was found, i.e. the component had already been loaded
                previously
        */
        bool        activateSubFrame(
                        const ::rtl::OUString& _rName,
                        const sal_Int32 _nComponentType,
                        const ElementOpenMode _eOpenMode,
                              ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_rComponent
                    ) const;

        /** closes all frames of the given component

            If a view for the component (given by name and type) has been loaded into one or more
            frames (with potentially different OpenModes), then those frames are gracefully closed.

            @return
                <TRUE/> if and only if closing those frames was successful, or frames for the given sub component
                exist.
        */
        bool        closeSubFrames(
                        const ::rtl::OUString& _rName,
                        const sal_Int32 _nComponentType
                    );

        /** searches for the given sub component

            @param i_rComponent
                the sub component to look up
            @param o_rName
                contains, upon successful return, the name of the sub component
            @param o_nComponentType
                contains, upon successful return, the type of the sub component
            @return
                <TRUE/> if and only if the component was found
        */
        bool        lookupSubComponent(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& i_rComponent,
                              ::rtl::OUString&  o_rName,
                              sal_Int32&        o_rComponentType
                    );

    private:
        ::std::auto_ptr< SubComponentManager_Data > m_pData;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SUBCOMPONENTMANAGER_HXX
