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

#ifndef COMMANDDESCRIPTIONPROVIDER_HXX
#define COMMANDDESCRIPTIONPROVIDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ICommandDescriptionProvider
    //====================================================================
    class SAL_NO_VTABLE ICommandDescriptionProvider
    {
    public:
        virtual ::rtl::OUString getCommandDescription( const ::rtl::OUString& _rCommandURL ) const = 0;

        virtual ~ICommandDescriptionProvider() { }
    };

    typedef ::boost::shared_ptr< const ICommandDescriptionProvider >  PCommandDescriptionProvider;

    //=====================================================================
    //= factory
    //=====================================================================
    PCommandDescriptionProvider
        createDocumentCommandDescriptionProvider(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument
        );

//........................................................................
} // namespace frm
//........................................................................

#endif // COMMANDDESCRIPTIONPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
