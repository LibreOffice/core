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

#ifndef INCLUDED_SVX_SOURCE_INC_FMSCRIPTINGENV_HXX
#define INCLUDED_SVX_SOURCE_INC_FMSCRIPTINGENV_HXX

#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

class FmFormModel;

namespace svxform
{
    class FormScriptListener;

    //= IFormScriptingEnvironment

    /** describes the interface implemented by a component which handles scripting requirements
        in a form/control environment.
    */
    class FormScriptingEnvironment : public ::salhelper::SimpleReferenceObject
    {
    friend class FormScriptListener;
    public:
        explicit FormScriptingEnvironment( FmFormModel& _rModel );
        FormScriptingEnvironment(const FormScriptingEnvironment&) = delete;
        FormScriptingEnvironment& operator=(const FormScriptingEnvironment&) = delete;

        /** registers an XEventAttacherManager whose events should be monitored and handled

            @param _rxManager
                the XEventAttacherManager to monitor. Must not be <NULL/>.

            @throws css::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws css::lang::DisposedException
                if the instance is already disposed
            @throws css::uno::RuntimeException
                if attaching as script listener to the manager fails with a RuntimeException itself
        */
        void registerEventAttacherManager(
            const css::uno::Reference< css::script::XEventAttacherManager >& _rxManager );

        /** registers an XEventAttacherManager whose events should not be monitored and handled anymore

            @param _rxManager
                the XEventAttacherManager which was previously registered. Must not ne <NULL/>.

            @throws css::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws css::lang::DisposedException
                if the instance is already disposed
            @throws css::uno::RuntimeException
                if removing as script listener from the manager fails with a RuntimeException itself
        */
        void revokeEventAttacherManager(
            const css::uno::Reference< css::script::XEventAttacherManager >& _rxManager );

        /** disposes the scripting environment instance
        */
        void dispose();

    private:
        ::osl::Mutex            m_aMutex;
        rtl::Reference<FormScriptListener> m_pScriptListener;
        FmFormModel&            m_rFormModel;
        bool                    m_bDisposed;

        void impl_registerOrRevoke_throw( const css::uno::Reference< css::script::XEventAttacherManager >& _rxManager, bool _bRegister );
        // callback for FormScriptListener
        void doFireScriptEvent( const css::script::ScriptEvent& _rEvent, css::uno::Any* _pSynchronousResult );

    };

}


#endif // INCLUDED_SVX_SOURCE_INC_FMSCRIPTINGENV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
