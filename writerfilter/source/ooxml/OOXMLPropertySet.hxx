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

#include <vector>
#include "OOXMLBinaryObjectReference.hxx"
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <dmapper/resourcemodel.hxx>

namespace writerfilter::ooxml
{
class OOXMLValue : public Value
{
public:
    typedef tools::SvRef<OOXMLValue> Pointer_t;
    OOXMLValue();
    virtual ~OOXMLValue() override;

    OOXMLValue(OOXMLValue const&) = default;
    OOXMLValue(OOXMLValue&&) = default;
    OOXMLValue& operator=(OOXMLValue const&) = default;
    OOXMLValue& operator=(OOXMLValue&&) = default;

    virtual int getInt() const override;
    ;
    virtual OUString getString() const override;
    virtual css::uno::Any getAny() const override;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() override;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const;
};

class OOXMLProperty final : public Sprm
{
public:
    typedef tools::SvRef<OOXMLProperty> Pointer_t;
    enum Type_t
    {
        SPRM,
        ATTRIBUTE
    };

private:
    Id mId;
    mutable OOXMLValue::Pointer_t mpValue;
    Type_t meType;

public:
    OOXMLProperty(Id id, OOXMLValue::Pointer_t pValue, Type_t eType);
    OOXMLProperty(const OOXMLProperty& rSprm) = delete;
    virtual ~OOXMLProperty() override;

    sal_uInt32 getId() const override;
    Value::Pointer_t getValue() override;
    writerfilter::Reference<Properties>::Pointer_t getProps() override;
#ifdef DBG_UTIL
    std::string getName() const override;
    std::string toString() const override;
#endif
    void resolve(Properties& rProperties);
};

class OOXMLBinaryValue final : public OOXMLValue
{
    mutable OOXMLBinaryObjectReference::Pointer_t mpBinaryObj;

public:
    explicit OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t pBinaryObj);
    virtual ~OOXMLBinaryValue() override;

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLBooleanValue final : public OOXMLValue
{
    bool mbValue;
    explicit OOXMLBooleanValue(bool bValue);

public:
    static OOXMLValue::Pointer_t const& Create(bool bValue);
    static OOXMLValue::Pointer_t const& Create(std::string_view pValue);

    virtual ~OOXMLBooleanValue() override;

    OOXMLBooleanValue(OOXMLBooleanValue const&) = default;
    OOXMLBooleanValue(OOXMLBooleanValue&&) = default;
    OOXMLBooleanValue& operator=(OOXMLBooleanValue const&) = delete; // due to const mbValue
    OOXMLBooleanValue& operator=(OOXMLBooleanValue&&) = delete; // due to const mbValue

    virtual int getInt() const override;
    virtual css::uno::Any getAny() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLStringValue final : public OOXMLValue
{
    OUString mStr;

public:
    explicit OOXMLStringValue(OUString sStr);
    virtual ~OOXMLStringValue() override;

    OOXMLStringValue(OOXMLStringValue const&) = default;
    OOXMLStringValue(OOXMLStringValue&&) = default;
    OOXMLStringValue& operator=(OOXMLStringValue const&) = delete; // due to const mStr
    OOXMLStringValue& operator=(OOXMLStringValue&&) = delete; // due to const mStr

    virtual css::uno::Any getAny() const override;
    virtual OUString getString() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLInputStreamValue final : public OOXMLValue
{
    css::uno::Reference<css::io::XInputStream> mxInputStream;

public:
    explicit OOXMLInputStreamValue(css::uno::Reference<css::io::XInputStream> xInputStream);
    virtual ~OOXMLInputStreamValue() override;

    virtual css::uno::Any getAny() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLPropertySet final : public writerfilter::Reference<Properties>
{
public:
    typedef std::vector<OOXMLProperty::Pointer_t> OOXMLProperties_t;
    typedef tools::SvRef<OOXMLPropertySet> Pointer_t;

private:
    OOXMLProperties_t mProperties;
    void add(const OOXMLProperty::Pointer_t& pProperty);

public:
    OOXMLPropertySet();
    virtual ~OOXMLPropertySet() override;

    OOXMLPropertySet(OOXMLPropertySet const&) = default;
    OOXMLPropertySet(OOXMLPropertySet&&) = default;
    OOXMLPropertySet& operator=(OOXMLPropertySet const&) = default;
    OOXMLPropertySet& operator=(OOXMLPropertySet&&) = default;

    void resolve(Properties& rHandler) override;
    void add(Id id, const OOXMLValue::Pointer_t& pValue, OOXMLProperty::Type_t eType);
    void add(const OOXMLPropertySet::Pointer_t& pPropertySet);
    OOXMLPropertySet* clone() const;

    OOXMLProperties_t::iterator begin();
    OOXMLProperties_t::iterator end();
    OOXMLProperties_t::const_iterator begin() const;
    OOXMLProperties_t::const_iterator end() const;

#ifdef DBG_UTIL
    std::string toString();
#endif
};

class OOXMLValue;

class OOXMLTable final : public writerfilter::Reference<Table>
{
public:
    typedef tools::SvRef<OOXMLValue> ValuePointer_t;
    OOXMLTable();
    virtual ~OOXMLTable() override;

    OOXMLTable(OOXMLTable const&) = default;
    OOXMLTable(OOXMLTable&&) = default;
    OOXMLTable& operator=(OOXMLTable const&) = default;
    OOXMLTable& operator=(OOXMLTable&&) = default;

    void resolve(Table& rTable) override;
    void add(const ValuePointer_t& pPropertySet);
    OOXMLTable* clone() const;

private:
    typedef std::vector<ValuePointer_t> PropertySets_t;
    PropertySets_t mPropertySets;
};

class OOXMLPropertySetValue final : public OOXMLValue
{
    OOXMLPropertySet::Pointer_t mpPropertySet;

public:
    explicit OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual ~OOXMLPropertySetValue() override;

    OOXMLPropertySetValue(OOXMLPropertySetValue const&) = default;
    OOXMLPropertySetValue(OOXMLPropertySetValue&&) = default;
    OOXMLPropertySetValue& operator=(OOXMLPropertySetValue const&)
        = delete; // due to const mpPropertySet
    OOXMLPropertySetValue& operator=(OOXMLPropertySetValue&&)
        = delete; // due to const mpPropertySet

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLIntegerValue final : public OOXMLValue
{
    sal_Int32 mnValue;
    explicit OOXMLIntegerValue(sal_Int32 nValue);

public:
    static OOXMLValue::Pointer_t Create(sal_Int32 nValue);
    virtual ~OOXMLIntegerValue() override;

    OOXMLIntegerValue(OOXMLIntegerValue const&) = default;
    OOXMLIntegerValue(OOXMLIntegerValue&&) = default;
    OOXMLIntegerValue& operator=(OOXMLIntegerValue const&) = delete; // due to const mnValue
    OOXMLIntegerValue& operator=(OOXMLIntegerValue&&) = delete; // due to const mnValue

    virtual int getInt() const override;
    virtual css::uno::Any getAny() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLHexValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;

public:
    explicit OOXMLHexValue(sal_uInt32 nValue);
    explicit OOXMLHexValue(std::string_view pValue);
    virtual ~OOXMLHexValue() override;

    OOXMLHexValue(OOXMLHexValue const&) = default;
    OOXMLHexValue(OOXMLHexValue&&) = default;
    OOXMLHexValue& operator=(OOXMLHexValue const&) = default;
    OOXMLHexValue& operator=(OOXMLHexValue&&) = default;

    virtual int getInt() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLHexColorValue final : public OOXMLHexValue
{
public:
    explicit OOXMLHexColorValue(std::string_view pValue);
};

class OOXMLUniversalMeasureValue : public OOXMLValue
{
private:
    int mnValue;

public:
    OOXMLUniversalMeasureValue(std::string_view pValue, sal_uInt32 npPt);
    virtual ~OOXMLUniversalMeasureValue() override;

    OOXMLUniversalMeasureValue(OOXMLUniversalMeasureValue const&) = default;
    OOXMLUniversalMeasureValue(OOXMLUniversalMeasureValue&&) = default;
    OOXMLUniversalMeasureValue& operator=(OOXMLUniversalMeasureValue const&) = default;
    OOXMLUniversalMeasureValue& operator=(OOXMLUniversalMeasureValue&&) = default;

    virtual int getInt() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
};

/// npPt is quotient defining how much units are in 1 pt
template <sal_uInt32 npPt> class OOXMLNthPtMeasureValue final : public OOXMLUniversalMeasureValue
{
public:
    explicit OOXMLNthPtMeasureValue(std::string_view pValue)
        : OOXMLUniversalMeasureValue(pValue, npPt)
    {
    }
    virtual OOXMLValue* clone() const override { return new OOXMLNthPtMeasureValue<npPt>(*this); }
};

/// Handles OOXML's ST_TwipsMeasure value.
typedef OOXMLNthPtMeasureValue<20> OOXMLTwipsMeasureValue;

/// Handles OOXML's ST_HpsMeasure value.
typedef OOXMLNthPtMeasureValue<2> OOXMLHpsMeasureValue;

class OOXMLMeasurementOrPercentValue final : public OOXMLValue
{
    int mnValue;

public:
    explicit OOXMLMeasurementOrPercentValue(std::string_view pValue);

    virtual int getInt() const override;
    virtual OOXMLValue* clone() const override { return new OOXMLMeasurementOrPercentValue(*this); }
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
};

class OOXMLShapeValue final : public OOXMLValue
{
    css::uno::Reference<css::drawing::XShape> mrShape;

public:
    explicit OOXMLShapeValue(css::uno::Reference<css::drawing::XShape> xShape);
    virtual ~OOXMLShapeValue() override;

    virtual css::uno::Any getAny() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLStarMathValue final : public OOXMLValue
{
    css::uno::Reference<css::embed::XEmbeddedObject> component;

public:
    explicit OOXMLStarMathValue(css::uno::Reference<css::embed::XEmbeddedObject> component);
    virtual ~OOXMLStarMathValue() override;

    virtual css::uno::Any getAny() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    virtual OOXMLValue* clone() const override;
};

class OOXMLPropertySetEntryToString final : public Properties
{
    Id mnId;
    OUString mStr;

public:
    explicit OOXMLPropertySetEntryToString(Id nId);
    virtual ~OOXMLPropertySetEntryToString() override;

    virtual void sprm(Sprm& rSprm) override;
    virtual void attribute(Id nId, Value& rValue) override;

    const OUString& getString() const { return mStr; }
};

class OOXMLPropertySetEntryToInteger final : public Properties
{
    Id mnId;
    int mnValue;

public:
    explicit OOXMLPropertySetEntryToInteger(Id nId);
    virtual ~OOXMLPropertySetEntryToInteger() override;

    virtual void sprm(Sprm& rSprm) override;
    virtual void attribute(Id nId, Value& rValue) override;

    int getValue() const { return mnValue; }
};

class OOXMLPropertySetEntryToBool final : public Properties
{
    Id mnId;
    bool mValue;

public:
    explicit OOXMLPropertySetEntryToBool(Id nId);
    virtual ~OOXMLPropertySetEntryToBool() override;

    virtual void sprm(Sprm& rSprm) override;
    virtual void attribute(Id nId, Value& rValue) override;

    bool getValue() const { return mValue; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
