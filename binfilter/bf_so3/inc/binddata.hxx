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

#ifndef _BINDDATA_HXX
#define _BINDDATA_HXX "$Revision: 1.4 $"

#include <tools/solar.h>
#include <tools/list.hxx>

namespace binfilter
{

class SvBindingTransportFactory;
DECLARE_LIST (SvBindingTransportFactoryList, SvBindingTransportFactory*)

class SvLockBytesFactory;
DECLARE_LIST (SvLockBytesFactoryList, SvLockBytesFactory*)

class SvBindingData_Impl;

/*========================================================================
 *
 * SvBindingData interface.
 *
 *======================================================================*/
struct SvBindingData
{
    /** Representation.
    */
    SvBindingTransportFactoryList  m_aTransportFactories;
    SvLockBytesFactoryList         m_aLockBytesFactories;

    SvBindingData_Impl            *m_pImpl;

    /** Construction/Destruction.
    */
    SvBindingData (void);
    ~SvBindingData (void);

    /** Get/Delete.
    */
    static SvBindingData* Get    (void);
    static void           Delete (void);

    /** HasHttpCache.
    */
    BOOL HasHttpCache (void);

    /** ShouldUseFtpProxy.
    */
    BOOL ShouldUseFtpProxy (const String &rUrl);
};

#define BAPP() (SvBindingData::Get())

}

#endif /* _BINDDATA_HXX */



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
