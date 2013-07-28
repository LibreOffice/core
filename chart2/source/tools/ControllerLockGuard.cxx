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

#include "ControllerLockGuard.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

ControllerLockGuard::ControllerLockGuard( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

ControllerLockGuard::~ControllerLockGuard()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

ControllerLockHelper::ControllerLockHelper( const Reference< frame::XModel > & xModel ) :
        m_xModel( xModel )
{}

ControllerLockHelper::~ControllerLockHelper()
{}

void ControllerLockHelper::lockControllers()
{
    if( m_xModel.is())
        m_xModel->lockControllers();
}

void ControllerLockHelper::unlockControllers()
{
    if( m_xModel.is())
        m_xModel->unlockControllers();
}

ControllerLockHelperGuard::ControllerLockHelperGuard( ControllerLockHelper & rHelper ) :
        m_rHelper( rHelper )
{
    m_rHelper.lockControllers();
}

ControllerLockHelperGuard::~ControllerLockHelperGuard()
{
    m_rHelper.unlockControllers();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
