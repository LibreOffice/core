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

#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#define DBAUI_FILEPICKER_INTERACTION_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

namespace dbaui
{

    // OFilePickerInteractionHandler
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::task::XInteractionHandler
                                    >   OFilePickerInteractionHandler_Base;

    /** a InteractionHandler implementation which extends another handler with some customizability
    */
    class OFilePickerInteractionHandler : public OFilePickerInteractionHandler_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >
                    m_xMaster;      // our master handler
        sal_Bool    m_bDoesNotExist;

    public:
        OFilePickerInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxMaster );

        inline sal_Bool isDoesNotExist( ) const { return m_bDoesNotExist; }

    protected:
        // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _rxRequest ) throw (::com::sun::star::uno::RuntimeException);

        virtual ~OFilePickerInteractionHandler();
    };

}   // namespace dbaui

#endif // DBAUI_FILEPICKER_INTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
