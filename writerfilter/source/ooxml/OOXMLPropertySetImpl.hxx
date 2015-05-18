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
#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSETIMPL_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSETIMPL_HXX

#include <vector>
#include "OOXMLPropertySet.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include <com/sun/star/embed/XEmbeddedObject.hpp>

namespace com {
namespace sun {
namespace star {
namespace drawing {
class XShape;
}}}}

namespace writerfilter {
namespace ooxml
{

class OOXMLValue : public Value
{
public:
    typedef std::shared_ptr<OOXMLValue> Pointer_t;
    OOXMLValue();
    virtual ~OOXMLValue();

    virtual int getInt() const SAL_OVERRIDE;
    virtual bool getBool() const;
    virtual OUString getString() const SAL_OVERRIDE;
    virtual css::uno::Any getAny() const SAL_OVERRIDE;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() SAL_OVERRIDE;
    virtual writerfilter::Reference<Stream>::Pointer_t getStream() SAL_OVERRIDE;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const;
};

class OOXMLPropertyImpl : public OOXMLProperty
{
public:
    enum Type_t { SPRM, ATTRIBUTE };
private:
    Id mId;
    mutable OOXMLValue::Pointer_t mpValue;
    Type_t meType;

public:
    typedef std::shared_ptr<OOXMLProperty> Pointer_t;

    OOXMLPropertyImpl(Id id, OOXMLValue::Pointer_t pValue, Type_t eType);
    OOXMLPropertyImpl(const OOXMLPropertyImpl & rSprm);
    virtual ~OOXMLPropertyImpl();

    virtual sal_uInt32 getId() const SAL_OVERRIDE;
    virtual Value::Pointer_t getValue() SAL_OVERRIDE;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() SAL_OVERRIDE;
    virtual writerfilter::Reference<Stream>::Pointer_t getStream() SAL_OVERRIDE;
    virtual writerfilter::Reference<Properties>::Pointer_t getProps() SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string getName() const SAL_OVERRIDE;
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual Sprm * clone() SAL_OVERRIDE;
    virtual void resolve(Properties & rProperties) SAL_OVERRIDE;
};

class OOXMLBinaryValue : public OOXMLValue
{
protected:
    mutable OOXMLBinaryObjectReference::Pointer_t mpBinaryObj;
public:
    explicit OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t pBinaryObj);
    virtual ~OOXMLBinaryValue();

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLBooleanValue : public OOXMLValue
{
protected:
    bool mbValue;
    explicit OOXMLBooleanValue(bool bValue);
public:
    static OOXMLValue::Pointer_t Create (bool bValue);
    static OOXMLValue::Pointer_t Create (const char *pValue);

    virtual ~OOXMLBooleanValue();

    virtual int getInt() const SAL_OVERRIDE;
    virtual bool getBool() const SAL_OVERRIDE;
    virtual css::uno::Any getAny() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLStringValue : public OOXMLValue
{
protected:
    OUString mStr;
public:
    explicit OOXMLStringValue(const OUString & rStr);
    virtual ~OOXMLStringValue();

    virtual css::uno::Any getAny() const SAL_OVERRIDE;
    virtual OUString getString() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLInputStreamValue : public OOXMLValue
{
protected:
    css::uno::Reference<css::io::XInputStream> mxInputStream;

public:
    explicit OOXMLInputStreamValue(css::uno::Reference<css::io::XInputStream> xInputStream);
    virtual ~OOXMLInputStreamValue();

    virtual css::uno::Any getAny() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

struct OOXMLPropertySetImplCompare
{
    bool operator()(const OOXMLProperty::Pointer_t& x,
                    const OOXMLProperty::Pointer_t& y) const;
};

class OOXMLPropertySetImpl : public OOXMLPropertySet
{
public:
    typedef std::vector<OOXMLProperty::Pointer_t> OOXMLProperties_t;
private:
    OOXMLProperties_t mProperties;
    OString maType;
public:
    OOXMLPropertySetImpl();
    virtual ~OOXMLPropertySetImpl();

    virtual void resolve(Properties & rHandler) SAL_OVERRIDE;
    virtual void add(OOXMLProperty::Pointer_t pProperty) SAL_OVERRIDE;
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet) SAL_OVERRIDE;
    virtual OOXMLPropertySet * clone() const SAL_OVERRIDE;

    OOXMLProperties_t::iterator begin();
    OOXMLProperties_t::iterator end();
    OOXMLProperties_t::const_iterator begin() const;
    OOXMLProperties_t::const_iterator end() const;

    virtual void setType(const std::string & rsType) SAL_OVERRIDE;

#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() SAL_OVERRIDE;
#endif
};

class OOXMLPropertySetValue : public OOXMLValue
{
    OOXMLPropertySet::Pointer_t mpPropertySet;
public:
    explicit OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual ~OOXMLPropertySetValue();

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLIntegerValue : public OOXMLValue
{
protected:
    sal_Int32 mnValue;
    explicit OOXMLIntegerValue(sal_Int32 nValue);
public:
    static OOXMLValue::Pointer_t Create (sal_Int32 nValue);
    virtual ~OOXMLIntegerValue();

    virtual int getInt() const SAL_OVERRIDE;
    virtual css::uno::Any getAny() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLHexValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;
public:
    explicit OOXMLHexValue(sal_uInt32 nValue);
    explicit OOXMLHexValue(const char * pValue);
    virtual ~OOXMLHexValue();

    virtual int getInt() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

/// Handles OOXML's ST_UniversalMeasure value.
class OOXMLUniversalMeasureValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;
public:
    explicit OOXMLUniversalMeasureValue(const char * pValue);
    virtual ~OOXMLUniversalMeasureValue();

    virtual int getInt() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue* clone() const SAL_OVERRIDE;
};

class OOXMLShapeValue : public OOXMLValue
{
protected:
    css::uno::Reference<css::drawing::XShape> mrShape;
public:
    explicit OOXMLShapeValue(css::uno::Reference<css::drawing::XShape> rShape);
    virtual ~OOXMLShapeValue();

    virtual css::uno::Any getAny() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLStarMathValue : public OOXMLValue
{
protected:
    css::uno::Reference< css::embed::XEmbeddedObject > component;
public:
    explicit OOXMLStarMathValue( css::uno::Reference< css::embed::XEmbeddedObject > component );
    virtual ~OOXMLStarMathValue();

    virtual css::uno::Any getAny() const SAL_OVERRIDE;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const SAL_OVERRIDE;
#endif
    virtual OOXMLValue * clone() const SAL_OVERRIDE;
};

class OOXMLTableImpl : public OOXMLTable
{
public:
    typedef std::shared_ptr<OOXMLValue> ValuePointer_t;
private:
    typedef std::vector<ValuePointer_t> PropertySets_t;
    PropertySets_t mPropertySets;

public:
    OOXMLTableImpl();
    virtual ~OOXMLTableImpl();

    virtual void resolve(Table & rTable) SAL_OVERRIDE;
    void add(ValuePointer_t pPropertySet);
    virtual OOXMLTable * clone() const SAL_OVERRIDE;
};

class OOXMLPropertySetEntryToString : public Properties
{
    Id mnId;
    OUString mStr;

public:
    explicit OOXMLPropertySetEntryToString(Id nId);
    virtual ~OOXMLPropertySetEntryToString();

    virtual void sprm(Sprm & rSprm) SAL_OVERRIDE;
    virtual void attribute(Id nId, Value & rValue) SAL_OVERRIDE;

    const OUString & getString() const { return mStr;}
};

class OOXMLPropertySetEntryToInteger : public Properties
{
    Id mnId;
    int mnValue;
public:
    explicit OOXMLPropertySetEntryToInteger(Id nId);
    virtual ~OOXMLPropertySetEntryToInteger();

    virtual void sprm(Sprm & rSprm) SAL_OVERRIDE;
    virtual void attribute(Id nId, Value & rValue) SAL_OVERRIDE;

    int getValue() const { return mnValue;}
};

class OOXMLPropertySetEntryToBool : public Properties
{
    Id mnId;
    bool mValue;
public:
    explicit OOXMLPropertySetEntryToBool(Id nId);
    virtual ~OOXMLPropertySetEntryToBool();

    virtual void sprm(Sprm & rSprm) SAL_OVERRIDE;
    virtual void attribute(Id nId, Value & rValue) SAL_OVERRIDE;

    bool getValue() const { return mValue; }
};

}
}

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSETIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
