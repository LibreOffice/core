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

    virtual int getInt() const override;
    ;
    virtual OUString getString() const override;
    virtual css::uno::Any getAny() const override;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() override;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
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

    virtual sal_uInt32 getId() const override;
    virtual Value::Pointer_t getValue() override;
    virtual writerfilter::Reference<Properties>::Pointer_t getProps() override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string getName() const override;
    virtual std::string toString() const override;
#endif
    virtual void resolve(Properties & rProperties) override;
};

class OOXMLBinaryValue : public OOXMLValue
{
protected:
    mutable OOXMLBinaryObjectReference::Pointer_t mpBinaryObj;
public:
    explicit OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t pBinaryObj);
    virtual ~OOXMLBinaryValue();

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
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

    virtual int getInt() const override;
    virtual css::uno::Any getAny() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLStringValue : public OOXMLValue
{
protected:
    OUString mStr;
public:
    explicit OOXMLStringValue(const OUString & rStr);
    virtual ~OOXMLStringValue();

    virtual css::uno::Any getAny() const override;
    virtual OUString getString() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLInputStreamValue : public OOXMLValue
{
protected:
    css::uno::Reference<css::io::XInputStream> mxInputStream;

public:
    explicit OOXMLInputStreamValue(css::uno::Reference<css::io::XInputStream> xInputStream);
    virtual ~OOXMLInputStreamValue();

    virtual css::uno::Any getAny() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
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

    virtual void resolve(Properties & rHandler) override;
    virtual void add(OOXMLProperty::Pointer_t pProperty) override;
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet) override;
    virtual OOXMLPropertySet * clone() const override;

    OOXMLProperties_t::iterator begin();
    OOXMLProperties_t::iterator end();
    OOXMLProperties_t::const_iterator begin() const;
    OOXMLProperties_t::const_iterator end() const;

#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() override;
#endif
};

class OOXMLPropertySetValue : public OOXMLValue
{
    OOXMLPropertySet::Pointer_t mpPropertySet;
public:
    explicit OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual ~OOXMLPropertySetValue();

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLIntegerValue : public OOXMLValue
{
protected:
    sal_Int32 mnValue;
    explicit OOXMLIntegerValue(sal_Int32 nValue);
public:
    static OOXMLValue::Pointer_t Create (sal_Int32 nValue);
    virtual ~OOXMLIntegerValue();

    virtual int getInt() const override;
    virtual css::uno::Any getAny() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLHexValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;
public:
    explicit OOXMLHexValue(sal_uInt32 nValue);
    explicit OOXMLHexValue(const char * pValue);
    virtual ~OOXMLHexValue();

    virtual int getInt() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

/// Handles OOXML's ST_UniversalMeasure value.
class OOXMLUniversalMeasureValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;
public:
    explicit OOXMLUniversalMeasureValue(const char * pValue);
    virtual ~OOXMLUniversalMeasureValue();

    virtual int getInt() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLShapeValue : public OOXMLValue
{
protected:
    css::uno::Reference<css::drawing::XShape> mrShape;
public:
    explicit OOXMLShapeValue(css::uno::Reference<css::drawing::XShape> rShape);
    virtual ~OOXMLShapeValue();

    virtual css::uno::Any getAny() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLStarMathValue : public OOXMLValue
{
protected:
    css::uno::Reference< css::embed::XEmbeddedObject > component;
public:
    explicit OOXMLStarMathValue( css::uno::Reference< css::embed::XEmbeddedObject > component );
    virtual ~OOXMLStarMathValue();

    virtual css::uno::Any getAny() const override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue * clone() const override;
};

class OOXMLPropertySetEntryToString : public Properties
{
    Id mnId;
    OUString mStr;

public:
    explicit OOXMLPropertySetEntryToString(Id nId);
    virtual ~OOXMLPropertySetEntryToString();

    virtual void sprm(Sprm & rSprm) override;
    virtual void attribute(Id nId, Value & rValue) override;

    const OUString & getString() const { return mStr;}
};

class OOXMLPropertySetEntryToInteger : public Properties
{
    Id mnId;
    int mnValue;
public:
    explicit OOXMLPropertySetEntryToInteger(Id nId);
    virtual ~OOXMLPropertySetEntryToInteger();

    virtual void sprm(Sprm & rSprm) override;
    virtual void attribute(Id nId, Value & rValue) override;

    int getValue() const { return mnValue;}
};

class OOXMLPropertySetEntryToBool : public Properties
{
    Id mnId;
    bool mValue;
public:
    explicit OOXMLPropertySetEntryToBool(Id nId);
    virtual ~OOXMLPropertySetEntryToBool();

    virtual void sprm(Sprm & rSprm) override;
    virtual void attribute(Id nId, Value & rValue) override;

    bool getValue() const { return mValue; }
};

}  // namespace ooxml
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPROPERTYSETIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
