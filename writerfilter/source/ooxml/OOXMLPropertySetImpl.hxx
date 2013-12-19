/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
#define INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX

#include <vector>
#include "OOXMLPropertySet.hxx"
#include "OOXMLBinaryObjectReference.hxx"

namespace com {
namespace sun {
namespace star {
namespace drawing {
class XShape;
}}}}

namespace writerfilter {
namespace ooxml
{
using namespace ::std;
using ::com::sun::star::drawing::XShape;

class OOXMLValue : public Value
{
public:
    typedef boost::shared_ptr<OOXMLValue> Pointer_t;
    OOXMLValue();
    virtual ~OOXMLValue();

    virtual sal_Int32 getInt() const;
    virtual bool getBool() const;
    virtual ::rtl::OUString getString() const;
    virtual uno::Any getAny() const;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties();
    virtual writerfilter::Reference<Stream>::Pointer_t getStream();
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual string toString() const;
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
    typedef boost::shared_ptr<OOXMLProperty> Pointer_t;

    OOXMLPropertyImpl(Id id, OOXMLValue::Pointer_t pValue, Type_t eType);
    OOXMLPropertyImpl(const OOXMLPropertyImpl & rSprm);
    virtual ~OOXMLPropertyImpl();

    virtual sal_uInt32 getId() const;
    virtual Value::Pointer_t getValue();
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual writerfilter::Reference<Stream>::Pointer_t getStream();
    virtual writerfilter::Reference<Properties>::Pointer_t getProps();
    virtual string getName() const;
    virtual Kind getKind();
    virtual string toString() const;
    virtual Sprm * clone();
    virtual void resolve(Properties & rProperties);
};

class OOXMLBinaryValue : public OOXMLValue
{
protected:
    mutable OOXMLBinaryObjectReference::Pointer_t mpBinaryObj;
public:
    explicit OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t pBinaryObj);
    virtual ~OOXMLBinaryValue();

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLBooleanValue : public OOXMLValue
{
protected:
    bool mbValue;
public:
    explicit OOXMLBooleanValue(bool bValue);
    explicit OOXMLBooleanValue(const rtl::OUString & rValue);
    virtual ~OOXMLBooleanValue();

    virtual sal_Int32 getInt() const;
    virtual bool getBool() const;
    virtual uno::Any getAny() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLStringValue : public OOXMLValue
{
protected:
    rtl::OUString mStr;
public:
    explicit OOXMLStringValue(const rtl::OUString & rStr);
    virtual ~OOXMLStringValue();

    virtual uno::Any getAny() const;
    virtual rtl::OUString getString() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLInputStreamValue : public OOXMLValue
{
protected:
    uno::Reference<io::XInputStream> mxInputStream;

public:
    explicit OOXMLInputStreamValue(uno::Reference<io::XInputStream> xInputStream);
    virtual ~OOXMLInputStreamValue();

    virtual uno::Any getAny() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

struct OOXMLPropertySetImplCompare
{
    bool operator()(const OOXMLProperty::Pointer_t x,
                    const OOXMLProperty::Pointer_t y) const;
};

class OOXMLPropertySetImpl : public OOXMLPropertySet
{
public:
    typedef vector<OOXMLProperty::Pointer_t> OOXMLProperties_t;
private:
    OOXMLProperties_t mProperties;
    string msType;
public:
    OOXMLPropertySetImpl();
    virtual ~OOXMLPropertySetImpl();

    virtual void resolve(Properties & rHandler);
    virtual string getType() const;
    virtual void add(OOXMLProperty::Pointer_t pProperty);
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual OOXMLPropertySet * clone() const;

    OOXMLProperties_t::iterator begin();
    OOXMLProperties_t::iterator end();
    OOXMLProperties_t::const_iterator begin() const;
    OOXMLProperties_t::const_iterator end() const;

    virtual void setType(const string & rsType);

    virtual string toString();
};

class OOXMLPropertySetValue : public OOXMLValue
{
    OOXMLPropertySet::Pointer_t mpPropertySet;
public:
    OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual ~OOXMLPropertySetValue();

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties();
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLIntegerValue : public OOXMLValue
{
protected:
    sal_Int32 mnValue;
public:
    explicit OOXMLIntegerValue(sal_Int32 nValue);
    explicit OOXMLIntegerValue(const rtl::OUString & rValue);
    virtual ~OOXMLIntegerValue();

    virtual sal_Int32 getInt() const;
    virtual uno::Any getAny() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLHexValue : public OOXMLValue
{
protected:
    sal_uInt32 mnValue;
public:
    explicit OOXMLHexValue(sal_uInt32 nValue);
    explicit OOXMLHexValue(const rtl::OUString & rValue);
    virtual ~OOXMLHexValue();

    virtual sal_Int32 getInt() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLShapeValue : public OOXMLValue
{
protected:
    uno::Reference<XShape> mrShape;
public:
    explicit OOXMLShapeValue(uno::Reference<XShape> rShape);
    virtual ~OOXMLShapeValue();

    virtual uno::Any getAny() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLTableImpl : public OOXMLTable
{
public:
    typedef boost::shared_ptr<OOXMLValue> ValuePointer_t;
private:
    typedef vector<ValuePointer_t> PropertySets_t;
    PropertySets_t mPropertySets;

public:
    OOXMLTableImpl();
    virtual ~OOXMLTableImpl();

    virtual void resolve(Table & rTable);
    virtual void add(ValuePointer_t pPropertySet);
    virtual string getType() const;
    virtual OOXMLTable * clone() const;
};

class OOXMLPropertySetEntryToString : public Properties
{
    Id mnId;
    ::rtl::OUString mStr;

public:
    OOXMLPropertySetEntryToString(Id nId);
    virtual ~OOXMLPropertySetEntryToString();

    virtual void sprm(Sprm & rSprm);
    virtual void attribute(Id nId, Value & rValue);

    const ::rtl::OUString & getString() const;
};

class OOXMLPropertySetEntryToInteger : public Properties
{
    Id mnId;
    int mnValue;
public:
    OOXMLPropertySetEntryToInteger(Id nId);
    virtual ~OOXMLPropertySetEntryToInteger();

    virtual void sprm(Sprm & rSprm);
    virtual void attribute(Id nId, Value & rValue);

    int getValue() const;
};

Sprm::Kind SprmKind(sal_uInt32 nSprmCode);

}  // namespace ooxml
} // namespace writerfilter

#endif // INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
