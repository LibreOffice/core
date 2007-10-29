/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySetImpl.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:27:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
#define INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX

#include <vector>
#include "OOXMLPropertySet.hxx"

namespace ooxml
{
using namespace ::std;
using namespace doctok;

class OOXMLValue : public Value
{
public:
    typedef auto_ptr<OOXMLValue> Pointer_t;
    OOXMLValue(const rtl::OUString & rValue);
    OOXMLValue();
    virtual ~OOXMLValue();

    virtual int getInt() const;
    virtual bool getBool() const;
    virtual ::rtl::OUString getString() const;
    virtual uno::Any getAny() const;
    virtual doctok::Reference<Properties>::Pointer_t getProperties();
    virtual doctok::Reference<Stream>::Pointer_t getStream();
    virtual doctok::Reference<BinaryObj>::Pointer_t getBinary();
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
    virtual doctok::Reference<BinaryObj>::Pointer_t getBinary();
    virtual doctok::Reference<Stream>::Pointer_t getStream();
    virtual doctok::Reference<Properties>::Pointer_t getProps();
    virtual string getName() const;
    virtual Kind getKind();
    virtual string toString() const;
    virtual Sprm * clone();
    virtual void resolve(doctok::Properties & rProperties);
};

class OOXMLBooleanValue : public OOXMLValue
{
protected:
    bool mbValue;
public:
    explicit OOXMLBooleanValue(bool bValue);
    explicit OOXMLBooleanValue(const rtl::OUString & rValue);
    virtual ~OOXMLBooleanValue();

    virtual int getInt() const;
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
public:
    OOXMLPropertySetImpl();
    virtual ~OOXMLPropertySetImpl();

    virtual void resolve(Properties & rHandler);
    virtual string getType() const;
    virtual void add(OOXMLProperty::Pointer_t pProperty);
    virtual OOXMLPropertySet * clone() const;

    OOXMLProperties_t::iterator begin();
    OOXMLProperties_t::iterator end();
};

class OOXMLPropertySetValue : public OOXMLValue
{
    OOXMLPropertySet::Pointer_t mpPropertySet;
public:
    OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet);
    virtual ~OOXMLPropertySetValue();

    virtual doctok::Reference<Properties>::Pointer_t getProperties();
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

    virtual int getInt() const;
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

    virtual int getInt() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLListValue : public OOXMLIntegerValue
{
public:
    OOXMLListValue();
    OOXMLListValue(sal_Int32 nValue);
    virtual ~OOXMLListValue();
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

Sprm::Kind SprmKind(sal_uInt32 nSprmCode);

}  // namespace ooxml

#endif // INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
