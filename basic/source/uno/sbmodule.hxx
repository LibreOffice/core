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

#ifndef SBMODULE_HXX
#define SBMODULE_HXX

#include <comphelper/componentmodule.hxx>

//........................................................................
namespace basic
{
//........................................................................

    class BasicModule : public ::comphelper::OModule
    {
        friend struct BasicModuleCreator;
        typedef ::comphelper::OModule BaseClass;

    public:
        static BasicModule& getInstance();

    private:
        BasicModule();
    };

    /* -------------------------------------------------------------------- */
    class BasicModuleClient : public ::comphelper::OModuleClient
    {
    private:
        typedef ::comphelper::OModuleClient BaseClass;

    public:
        BasicModuleClient() : BaseClass( BasicModule::getInstance() )
        {
        }
    };

    /* -------------------------------------------------------------------- */
    template < class TYPE >
    class OAutoRegistration : public ::comphelper::OAutoRegistration< TYPE >
    {
    private:
        typedef ::comphelper::OAutoRegistration< TYPE > BaseClass;

    public:
        OAutoRegistration() : BaseClass( BasicModule::getInstance() )
        {
        }
    };

    /* -------------------------------------------------------------------- */
    template < class TYPE >
    class OSingletonRegistration : public ::comphelper::OSingletonRegistration< TYPE >
    {
    private:
        typedef ::comphelper::OSingletonRegistration< TYPE > BaseClass;

    public:
        OSingletonRegistration() : BaseClass( BasicModule::getInstance() )
        {
        }
    };

//........................................................................
} // namespace basic
//........................................................................

#endif // SBMODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
