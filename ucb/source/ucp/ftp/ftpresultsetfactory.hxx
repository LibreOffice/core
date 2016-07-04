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
#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPRESULTSETFACTORY_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPRESULTSETFACTORY_HXX

#include "ftpresultsetbase.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <vector>

namespace ftp {

class ResultSetBase;

class ResultSetFactory
{
public:
    ResultSetFactory(const css::uno::Reference<css::uno::XComponentContext >&  rxContext,
                      const css::uno::Reference<css::ucb::XContentProvider >&  xProvider,
                      const css::uno::Sequence<css::beans::Property>& seq,
                      const std::vector<FTPDirentry>& dirvec);

    ResultSetBase* createResultSet();
private:
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    css::uno::Reference< css::ucb::XContentProvider >         m_xProvider;
    css::uno::Sequence< css::beans::Property >                m_seq;
    std::vector<FTPDirentry>                                  m_dirvec;
};

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
