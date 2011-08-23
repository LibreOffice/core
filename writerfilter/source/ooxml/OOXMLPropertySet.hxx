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
#ifndef INCLUDED_OOXML_PROPERTY_SET_HXX
#define INCLUDED_OOXML_PROPERTY_SET_HXX

#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
namespace ooxml
{

class OOXMLProperty : public Sprm
{
public:
    typedef boost::shared_ptr<OOXMLProperty> Pointer_t;

    virtual ~OOXMLProperty();

    virtual sal_uInt32 getId() const = 0;
    virtual Value::Pointer_t getValue() = 0;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() = 0;
    virtual writerfilter::Reference<Stream>::Pointer_t getStream() = 0;
    virtual writerfilter::Reference<Properties>::Pointer_t getProps() = 0;
    virtual string getName() const = 0;
    virtual string toString() const = 0;
    virtual void resolve(Properties & rProperties) = 0;

    virtual Sprm * clone() = 0;
};

class OOXMLPropertySet : public writerfilter::Reference<Properties>
{    
public:
    typedef boost::shared_ptr<OOXMLPropertySet> Pointer_t;

    virtual ~OOXMLPropertySet();

    virtual void resolve(Properties & rHandler) = 0;
    virtual string getType() const = 0;
    virtual void add(OOXMLProperty::Pointer_t pProperty) = 0;
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet) = 0;
    virtual OOXMLPropertySet * clone() const = 0;
    virtual void setType(const string & rsType) = 0;
    
    virtual string toString() = 0;
};

class OOXMLTable : public writerfilter::Reference<Table>
{
public:
    virtual ~OOXMLTable();

    virtual void resolve(Table & rTable) = 0;
    virtual OOXMLTable * clone() const = 0;
};
}}

#endif // INCLUDED_OOXML_PROPERTY_SET_HXX
