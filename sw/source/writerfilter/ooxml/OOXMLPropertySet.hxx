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
#include <variant>

namespace writerfilter::ooxml
{
class OOXMLPropertySet;

class OOXMLValue final : public Value
{
public:
    OOXMLValue() {}
    ~OOXMLValue();

    static OOXMLValue createBoolean(bool);
    static OOXMLValue createBoolean(std::string_view);
    static OOXMLValue createInteger(int);
    static OOXMLValue createUniversalMeasure(std::string_view pValue, sal_uInt32 npPt);
    /// Handles OOXML's ST_Coordinate value.
    static OOXMLValue createEmuMeasure(std::string_view);
    /// Handles OOXML's ST_TwipsMeasure value.
    static OOXMLValue createTwipsMeasure(std::string_view);
    /// Handles OOXML's ST_HpsMeasure value.
    static OOXMLValue createHpsMeasure(std::string_view);
    static OOXMLValue createMeasurementOrPercent(std::string_view);
    static OOXMLValue createHex(sal_uInt32);
    static OOXMLValue createHex(std::string_view);
    static OOXMLValue createHexColor(std::string_view);
    static OOXMLValue createString(const OUString&);
    static OOXMLValue createPropertySet(const tools::SvRef<OOXMLPropertySet>&);
    static OOXMLValue createBinary(const tools::SvRef<writerfilter::Reference<BinaryObj>>&);
    static OOXMLValue createInputStream(const css::uno::Reference<css::io::XInputStream>&);
    static OOXMLValue createShape(const css::uno::Reference<css::drawing::XShape>&);
    static OOXMLValue createStarMath(const css::uno::Reference<css::embed::XEmbeddedObject>&);

    OOXMLValue(OOXMLValue const&) = default;
    OOXMLValue(OOXMLValue&&) = default;
    OOXMLValue& operator=(OOXMLValue const&) = default;
    OOXMLValue& operator=(OOXMLValue&&) = default;

    bool hasValue() const { return maData.index() != 0; }
    virtual int getInt() const override;
    virtual OUString getString() const override;
    virtual css::uno::Any getAny() const override;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() const override;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() const override;
#ifdef DBG_UTIL
    virtual std::string toString() const override;
#endif
    OOXMLValue* clone() const;

private:
    typedef std::variant<std::monostate, //
                         bool, //
                         sal_Int32, // integer
                         int, // universal measure
                         int, // measure or percent
                         sal_uInt32, // hex
                         OUString,
                         tools::SvRef<OOXMLPropertySet>, // aka OOXMLPropertySet::Pointer_t
                         tools::SvRef<writerfilter::Reference<BinaryObj>>,
                         css::uno::Reference<css::io::XInputStream>,
                         css::uno::Reference<css::drawing::XShape>,
                         css::uno::Reference<css::embed::XEmbeddedObject> // StarMath
                         >
        VariantType;

    OOXMLValue(VariantType aData);

    VariantType maData;
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
    mutable OOXMLValue maValue;
    Type_t meType;

public:
    OOXMLProperty(Id id, const OOXMLValue& aValue, Type_t eType);
    OOXMLProperty(const OOXMLProperty& rSprm) = delete;
    virtual ~OOXMLProperty() override;

    sal_uInt32 getId() const override;
    const OOXMLValue* getValue() const override { return maValue.hasValue() ? &maValue : nullptr; }
    writerfilter::Reference<Properties>::Pointer_t getProps() override;
#ifdef DBG_UTIL
    std::string getName() const override;
    std::string toString() const override;
#endif
    void resolve(Properties& rProperties);
};

bool GetBooleanValue(std::string_view pValue);

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
    void add(Id id, const OOXMLValue& pValue, OOXMLProperty::Type_t eType);
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
    OOXMLTable();
    virtual ~OOXMLTable() override;

    OOXMLTable(OOXMLTable const&) = default;
    OOXMLTable(OOXMLTable&&) = default;
    OOXMLTable& operator=(OOXMLTable const&) = default;
    OOXMLTable& operator=(OOXMLTable&&) = default;

    void resolve(Table& rTable) override;
    void add(const OOXMLValue& pPropertySet);
    OOXMLTable* clone() const;

private:
    typedef std::vector<OOXMLValue> PropertySets_t;
    PropertySets_t mPropertySets;
};

class OOXMLPropertySetEntryToString final : public Properties
{
    Id mnId;
    OUString mStr;

public:
    explicit OOXMLPropertySetEntryToString(Id nId);
    virtual ~OOXMLPropertySetEntryToString() override;

    virtual void sprm(Sprm& rSprm) override;
    virtual void attribute(Id nId, const Value& rValue) override;

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
    virtual void attribute(Id nId, const Value& rValue) override;

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
    virtual void attribute(Id nId, const Value& rValue) override;

    bool getValue() const { return mValue; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
