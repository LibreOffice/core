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

#ifndef SUBCOMPONENTLOADER_HXX
#define SUBCOMPONENTLOADER_HXX

#include <dbaccess/dbaccessdllapi.h>

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>

#include <cppuhelper/implbase1.hxx>

namespace dbaccess
{

    // SubComponentLoader
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XWindowListener
                                    >   SubComponentLoader_Base;
    struct SubComponentLoader_Data;
    /** is a helper class which loads/opens a given sub component as soon as the main application
        window becomes visible.
    */
    class DBACCESS_DLLPRIVATE SubComponentLoader : public SubComponentLoader_Base
    {
    public:
        SubComponentLoader(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& i_rApplicationController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandProcessor >& i_rSubDocumentDefinition
        );

        SubComponentLoader(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& i_rApplicationController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& i_rNonDocumentComponent
        );

        // XWindowListener
        virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~SubComponentLoader();

    private:
        SubComponentLoader_Data*    m_pData;
    };

} // namespace dbaccess

#endif // SUBCOMPONENTLOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
