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

#ifndef __UNOXWRAPPER_H_
#define __UNOXWRAPPER_H_

#include "resource.h"       // main symbols


#include <com/sun/star/accessibility/XAccessible.hpp>
#include "UAccCOM.h"

/**
 * CUNOXWrapper implements IUNOXWrapper interface.
 */
class ATL_NO_VTABLE CUNOXWrapper : public IUNOXWrapper
{
public:
    CUNOXWrapper()
    {   }

public:
    // IUNOXWrapper

    STDMETHOD(put_XInterface)(hyper pXInterface)
    {
        pUNOInterface = (com::sun::star::accessibility::XAccessible*)pXInterface;
        return S_OK;
    }

    STDMETHOD(put_XSubInterface)(hyper)
    {
        return S_OK;
    }

protected:

    com::sun::star::accessibility::XAccessible* pUNOInterface;
};

#endif //__UNOXWRAPPER_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
