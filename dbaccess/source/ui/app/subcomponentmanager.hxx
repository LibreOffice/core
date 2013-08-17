/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef DBACCESS_SUBCOMPONENTMANAGER_HXX
#define DBACCESS_SUBCOMPONENTMANAGER_HXX

#include "AppElementType.hxx"

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <comphelper/sharedmutex.hxx>
#include <cppuhelper/implbase1.hxx>

#include <memory>

namespace dbaui
{

    struct SubComponentManager_Data;
    class OApplicationController;

    // SubComponentManager
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
                        const OUString&  _rName,
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
                        const OUString& _rName,
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
                        const OUString& _rName,
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
                              OUString&  o_rName,
                              sal_Int32&        o_rComponentType
                    );

    private:
        ::std::auto_ptr< SubComponentManager_Data > m_pData;
    };

} // namespace dbaui

#endif // DBACCESS_SUBCOMPONENTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
