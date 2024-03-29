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

#include <ucbhelper/commandenvironment.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

namespace com::sun::star::task { class XInteractionHandler; }
namespace com::sun::star::ucb { class XProgressHandler; }

namespace ucbhelper
{
// struct CommandEnvironment_Impl.

struct CommandEnvironment_Impl
{
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler > m_xProgressHandler;

    CommandEnvironment_Impl( const Reference< XInteractionHandler >& rxInteractionHandler,
                             const Reference< XProgressHandler >& rxProgressHandler )
        : m_xInteractionHandler( rxInteractionHandler )
        , m_xProgressHandler( rxProgressHandler ) {}
};

// CommandEnvironment Implementation.

CommandEnvironment::CommandEnvironment(
        const Reference< XInteractionHandler >& rxInteractionHandler,
        const Reference< XProgressHandler >& rxProgressHandler )
    : m_pImpl( new CommandEnvironment_Impl( rxInteractionHandler,
                                            rxProgressHandler ) )
{
}

// virtual
CommandEnvironment::~CommandEnvironment()
{
}

// XCommandEnvironment methods.

// virtual
Reference< XInteractionHandler > SAL_CALL CommandEnvironment::getInteractionHandler()
{
    return m_pImpl->m_xInteractionHandler;
}

// virtual
Reference< XProgressHandler > SAL_CALL CommandEnvironment::getProgressHandler()
{
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
