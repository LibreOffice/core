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
#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSET_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSET_HXX

#include <dmapper/resourcemodel.hxx>

namespace writerfilter {
namespace ooxml
{

class OOXMLProperty : public Sprm
{
public:
    typedef std::shared_ptr<OOXMLProperty> Pointer_t;

    virtual ~OOXMLProperty();

    virtual sal_uInt32 getId() const SAL_OVERRIDE = 0;
    virtual Value::Pointer_t getValue() SAL_OVERRIDE = 0;
    virtual writerfilter::Reference<Properties>::Pointer_t getProps() SAL_OVERRIDE = 0;
#ifdef DEBUG_WRITERFILTER
    virtual std::string getName() const SAL_OVERRIDE = 0;
    virtual std::string toString() const SAL_OVERRIDE = 0;
#endif
    virtual void resolve(Properties & rProperties) = 0;

    virtual Sprm * clone() = 0;
};

class OOXMLPropertySet : public writerfilter::Reference<Properties>
{
public:
    typedef std::shared_ptr<OOXMLPropertySet> Pointer_t;

    virtual ~OOXMLPropertySet();

    virtual void resolve(Properties & rHandler) SAL_OVERRIDE = 0;
    virtual void add(OOXMLProperty::Pointer_t pProperty) = 0;
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet) = 0;
    virtual OOXMLPropertySet * clone() const = 0;
    virtual void setType(const std::string & rsType) = 0;

#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() = 0;
#endif
};

class OOXMLTable : public writerfilter::Reference<Table>
{
public:
    virtual ~OOXMLTable();

    virtual void resolve(Table & rTable) SAL_OVERRIDE = 0;
    virtual OOXMLTable * clone() const = 0;
};
}}

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
