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

#ifndef _CONTROLCOMMANDRESULT_HXX_
#define _CONTROLCOMMANDRESULT_HXX_

#include <sal/types.h>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

//---------------------------------------------
// declaration
//---------------------------------------------

class CControlCommandResult
{
public:
    CControlCommandResult( sal_Bool bResult = sal_False ) :
        m_bResult( bResult )
    {
    }

    virtual ~CControlCommandResult( )
    {
    }

    sal_Bool SAL_CALL hasResult( ) const
    {
        return m_bResult;
    }

private:
    sal_Bool m_bResult;
};

//---------------------------------------------
//
//---------------------------------------------

class CValueCommandResult : public CControlCommandResult
{
public:
    CValueCommandResult( sal_Bool bResult, const ::com::sun::star::uno::Any& aValue ) :
        CControlCommandResult( bResult ),
        m_aValue( aValue )
    {
    }

    ::com::sun::star::uno::Any SAL_CALL getValue( ) const
    {
        return m_aValue;
    }

private:
    ::com::sun::star::uno::Any m_aValue;
};

//---------------------------------------------
//
//---------------------------------------------

class CLabelCommandResult : public CControlCommandResult
{
public:
    CLabelCommandResult( sal_Bool bResult, const rtl::OUString& aLabel ) :
        CControlCommandResult( bResult ),
        m_aLabel( aLabel )
    {
    }

    rtl::OUString SAL_CALL getLabel( ) const
    {
        return m_aLabel;
    }

private:
    rtl::OUString m_aLabel;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
