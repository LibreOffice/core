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
#pragma once

#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <IUpdateHelper.hxx>

namespace dbaui
{
    class OParameterUpdateHelper : public IUpdateHelper
    {
        css::uno::Reference< css::sdbc::XPreparedStatement >  m_xPrepared;
        css::uno::Reference< css::sdbc::XParameters >         m_xParameters;

    public:
        explicit OParameterUpdateHelper(const css::uno::Reference< css::sdbc::XPreparedStatement >& _xPrepared)
           :m_xPrepared(_xPrepared)
           ,m_xParameters(_xPrepared,css::uno::UNO_QUERY)
        {
        }
        virtual ~OParameterUpdateHelper() {}
        virtual void updateString(sal_Int32 _nPos, const OUString& _sValue) override
        {
            m_xParameters->setString(_nPos, _sValue);
        }
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue) override
        {
            m_xParameters->setDouble(_nPos, _nValue);
        }
        virtual void updateDate(sal_Int32 _nPos,const css::util::Date& _nValue) override
        {
            m_xParameters->setDate(_nPos, _nValue);
        }
        virtual void updateTime(sal_Int32 _nPos,const css::util::Time& _nValue) override
        {
            m_xParameters->setTime(_nPos, _nValue);
        }
        virtual void updateTimestamp(sal_Int32 _nPos,const css::util::DateTime& _nValue) override
        {
            m_xParameters->setTimestamp(_nPos, _nValue);
        }
        virtual void updateInt(sal_Int32 _nPos, sal_Int32 _nValue) override
        {
            m_xParameters->setInt(_nPos, _nValue);
        }
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType) override
        {
            m_xParameters->setNull(_nPos,sqlType);
        }
        virtual void insertRow() override
        {
            m_xPrepared->executeUpdate();
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
