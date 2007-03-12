/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySetImpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-03-12 10:43:33 $
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

#include <set>
#include "OOXMLPropertySet.hxx"

namespace ooxml
{
using namespace ::std;
using namespace doctok;

class OOXMLValue : public Value
{
public:
    typedef auto_ptr<OOXMLValue> Pointer_t;
    OOXMLValue();
    virtual ~OOXMLValue();

    virtual int getInt() const;
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
    Id mId;
    mutable OOXMLValue::Pointer_t mpValue;

public:
    enum Type_t { SPRM, ATTRIBUTE } meType;
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
    virtual string toString() const;
    virtual Sprm * clone();
    virtual void resolve(doctok::Properties & rProperties);
};

class OOXMLBooleanValue : public OOXMLValue
{
protected:
    bool mbValue;
public:
    OOXMLBooleanValue(bool bValue);
    virtual ~OOXMLBooleanValue();

    virtual int getInt() const;
    virtual uno::Any getAny() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLStringValue : public OOXMLValue
{
protected:
    rtl::OUString mStr;
public:
    OOXMLStringValue(const rtl::OUString & rStr);
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
    typedef set<OOXMLProperty::Pointer_t, OOXMLPropertySetImplCompare> OOXMLProperties_t;
    OOXMLProperties_t mProperties;
public:
    OOXMLPropertySetImpl();
    virtual ~OOXMLPropertySetImpl();

    virtual void resolve(Properties & rHandler);
    virtual string getType() const;
    virtual void add(OOXMLProperty::Pointer_t pProperty);
    virtual OOXMLPropertySet * clone() const;
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
    OOXMLIntegerValue(sal_Int32 nValue);
    virtual ~OOXMLIntegerValue();

    virtual int getInt() const;
    virtual string toString() const;
    virtual OOXMLValue * clone() const;
};

class OOXMLListValue : public OOXMLIntegerValue
{
public:
    OOXMLListValue();
    virtual ~OOXMLListValue();
};

}

#endif // INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
