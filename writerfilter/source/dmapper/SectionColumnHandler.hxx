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
#ifndef INCLUDED_SECTIONCOLUMNHANDLER_HXX
#define INCLUDED_SECTIONCOLUMNHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>


namespace writerfilter {
namespace dmapper
{
struct _Column
{
    sal_Int32 nWidth;
    sal_Int32 nSpace;
};


class SectionColumnHandler : public LoggedProperties
{
    bool        bEqualWidth;
    sal_Int32   nSpace;
    sal_Int32   nNum;
    bool        bSep;
    std::vector<_Column> aCols;

    _Column   aTempColumn;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    SectionColumnHandler();
    virtual ~SectionColumnHandler();

    bool        IsEqualWidth() const { return bEqualWidth; }
    sal_Int32   GetSpace() const { return nSpace; }
    sal_Int32   GetNum() const { return nNum; }
    bool        IsSeparator() const { return bSep; }

    const std::vector<_Column>& GetColumns() const { return aCols;}

};
typedef boost::shared_ptr< SectionColumnHandler >          SectionColumnHandlerPtr;
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
