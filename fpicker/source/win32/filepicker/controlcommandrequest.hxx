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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_CONTROLCOMMANDREQUEST_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_CONTROLCOMMANDREQUEST_HXX

#include <sal/types.h>


// declaration


class CControlCommandRequest
{
public:
    explicit CControlCommandRequest( sal_Int16 aControlId ) :
        m_aControlId( aControlId )
    {
    }

    virtual ~CControlCommandRequest( )
    {
    }

    sal_Int16 SAL_CALL getControlId( ) const
    {
        return m_aControlId;
    }

private:
    sal_Int16 m_aControlId;
};


class CValueControlCommandRequest : public CControlCommandRequest
{
public:
    CValueControlCommandRequest(
        sal_Int16 aControlId,
        sal_Int16 aControlAction ) :
        CControlCommandRequest( aControlId ),
        m_aControlAction( aControlAction )
    {
    }

    sal_Int16 SAL_CALL getControlAction( ) const
    {
        return m_aControlAction;
    }

private:
    sal_Int16 m_aControlAction;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
