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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ucbhelper/activedatastreamer.hxx"

using namespace com::sun::star;

namespace ucbhelper
{
// ActiveDataStreamer Implementation.

// XActiveDataStreamer methods.

// virtual
void SAL_CALL ActiveDataStreamer::setStream( const uno::Reference< io::XStream >& xStream )
    throw( uno::RuntimeException,
           std::exception )
{
    m_xStream = xStream;
}

// virtual
uno::Reference< io::XStream > SAL_CALL ActiveDataStreamer::getStream()
    throw( uno::RuntimeException,
           std::exception )
{
    return m_xStream;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
