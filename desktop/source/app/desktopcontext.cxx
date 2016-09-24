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

#include <config_features.h>

#include "desktopcontext.hxx"

#include <vcl/svapp.hxx>
#include <svtools/javainteractionhandler.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;


namespace desktop
{

DesktopContext::DesktopContext( const Reference< XCurrentContext > & ctx )
    : m_xNextContext( ctx )
{
}

Any SAL_CALL DesktopContext::getValueByName( const OUString& Name) throw (RuntimeException, std::exception)
{
    Any retVal;

    if ( Name == JAVA_INTERACTION_HANDLER_NAME )
    {
#if HAVE_FEATURE_JAVA
        retVal = makeAny( Reference< XInteractionHandler >( new svt::JavaInteractionHandler()) );
#endif
    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
