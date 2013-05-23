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
#ifndef INCLUDED_MEASUREHANDLER_HXX
#define INCLUDED_MEASUREHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{
/** Handler for sprms that contain a measure and a unit
    - Left indent of tables
    - Preferred width of tables
 */
class WRITERFILTER_DLLPRIVATE MeasureHandler : public LoggedProperties
{
    sal_Int32 m_nMeasureValue;
    sal_Int32 m_nUnit;
    sal_Int16 m_nRowHeightSizeType; //table row height type

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    MeasureHandler();
    virtual ~MeasureHandler();

    sal_Int32 getMeasureValue() const;

    sal_Int32 getValue() const { return m_nMeasureValue; }
    sal_Int32 getUnit() const { return m_nUnit; }

    sal_Int16 GetRowHeightSizeType() const { return m_nRowHeightSizeType;}
};
typedef boost::shared_ptr
    < MeasureHandler >  MeasureHandlerPtr;
}}

#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
