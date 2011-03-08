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

#ifndef _CONTROLCOMMANDREQUEST_HXX_
#define _CONTROLCOMMANDREQUEST_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

//---------------------------------------------
// declaration
//---------------------------------------------

class CControlCommandRequest
{
public:
    CControlCommandRequest( sal_Int16 aControlId ) :
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

//---------------------------------------------
//
//---------------------------------------------

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
