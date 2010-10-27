/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef SVX_FMSCRIPTINGENV_HXX
#define SVX_FMSCRIPTINGENV_HXX

/** === begin UNO includes === **/
#include <com/sun/star/script/XEventAttacherManager.hpp>
/** === end UNO includes === **/
#include <rtl/ref.hxx>

class FmFormModel;
//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= IFormScriptingEnvironment
    //====================================================================
    /** describes the interface implemented by a component which handles scripting requirements
        in a form/control environment.
    */
    class SAL_NO_VTABLE IFormScriptingEnvironment : public ::rtl::IReference
    {
    public:
        /** registers an XEventAttacherManager whose events should be monitored and handled

            @param _rxManager
                the XEventAttacherManager to monitor. Must not be <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if attaching as script listener to the manager fails with a RuntimeException itself
        */
        virtual void registerEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** registers an XEventAttacherManager whose events should not be monitored and handled anymore

            @param _rxManager
                the XEventAttacherManager which was previously registered. Must not ne <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if removing as script listener from the manager fails with a RuntimeException itself
        */
        virtual void revokeEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** disposes the scripting environment instance
        */
        virtual void dispose() = 0;

        virtual ~IFormScriptingEnvironment();
    };
    typedef ::rtl::Reference< IFormScriptingEnvironment >   PFormScriptingEnvironment;

    //====================================================================
    /** creates a default component implementing the IFormScriptingEnvironment interface
    */
    PFormScriptingEnvironment   createDefaultFormScriptingEnvironment( FmFormModel& _rFormModel );

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FMSCRIPTINGENV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
