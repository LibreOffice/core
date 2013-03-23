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
#if 1

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE CellMarginHandler : public LoggedProperties
{
private:
    sal_Int32   m_nValue;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    sal_Int32   m_nLeftMargin;
    bool        m_bLeftMarginValid;
    sal_Int32   m_nRightMargin;
    bool        m_bRightMarginValid;
    sal_Int32   m_nTopMargin;
    bool        m_bTopMarginValid;
    sal_Int32   m_nBottomMargin;
    bool        m_bBottomMarginValid;

public:
    CellMarginHandler( );
    virtual ~CellMarginHandler();

    ::boost::shared_ptr<TablePropertyMap>            getProperties();

};
typedef boost::shared_ptr< CellMarginHandler >          CellMarginHandlerPtr;
}}

#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
