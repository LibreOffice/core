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

#ifndef INCLUDED_WW8_RESOURCE_MODEL_IMPL_HXX
#define INCLUDED_WW8_RESOURCE_MODEL_IMPL_HXX

#include <doctok/WW8Document.hxx>

#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#endif
#include <WW8StructBase.hxx>

#ifndef INCLUDED_OUTPUT_WITH_DEPTH_HXX
#include <resourcemodel/OutputWithDepth.hxx>
#endif

#include <map>

namespace writerfilter {
namespace doctok
{
using namespace ::std;

class WW8PropertiesReference : public writerfilter::Reference<Properties>
{
    WW8PropertySet::Pointer_t mpPropSet;

public:
    
    WW8PropertiesReference(WW8PropertySet::Pointer_t pPropSet)
    : mpPropSet(pPropSet)
    {
    }

    ~WW8PropertiesReference()
    {
    }

    virtual void resolve(Properties & rHandler);

    virtual string getType() const;
};

class WW8TableReference : public writerfilter::Reference<Table>
{
public:
    WW8TableReference()
    {
    }

    ~WW8TableReference()
    {
    }

    virtual void resolve(Table & rHandler);
    
    virtual string getType() const;
};

class WW8BinaryObjReference : public writerfilter::Reference<BinaryObj>, 
                              public WW8StructBase
{
public:
    typedef boost::shared_ptr<WW8BinaryObjReference> Pointer_t;
    WW8BinaryObjReference(WW8Stream & rStream, sal_uInt32 nOffset, 
                          sal_uInt32 nCount);    
    WW8BinaryObjReference(WW8StructBase & rParent, sal_uInt32 nOffset, 
                          sal_uInt32 nCount); 
    WW8BinaryObjReference(WW8StructBase * pParent, sal_uInt32 nOffset, 
                          sal_uInt32 nCount); 
    WW8BinaryObjReference(WW8StructBase * pParent);

    WW8BinaryObjReference()
    : WW8StructBase(WW8StructBase::Sequence())
    {
    }

    ~WW8BinaryObjReference()
    {
    }

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();

    virtual void resolve(BinaryObj & rHandler);

    virtual string getType() const;

    virtual WW8BinaryObjReference * clone() { return new WW8BinaryObjReference(*this); }
};

class WW8Sprm : public Sprm
{
    WW8Property::Pointer_t mpProperty;
    WW8BinaryObjReference::Pointer_t mpBinary;

public:
    WW8Sprm(WW8Property::Pointer_t pProperty)
    : mpProperty(pProperty)
    {        
    }

    WW8Sprm(WW8BinaryObjReference::Pointer_t pBinary)
    : mpBinary(pBinary)
    {
    }

    WW8Sprm()
    {
    }

    WW8Sprm(const WW8Sprm & rSprm)
    : Sprm(rSprm), mpProperty(rSprm.mpProperty), mpBinary(rSprm.mpBinary)
    {
    }

    virtual ~WW8Sprm()
    {
    }

    virtual Value::Pointer_t getValue();
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual writerfilter::Reference<Stream>::Pointer_t getStream();
    virtual writerfilter::Reference<Properties>::Pointer_t getProps();
    virtual Kind getKind();

    virtual sal_uInt32 getId() const;    
    virtual string toString() const;
    virtual string getName() const;

    virtual WW8Sprm * clone() const { return new WW8Sprm(*this); }
};

class WW8Value : public Value
{
public:
    WW8Value() {}
    virtual ~WW8Value() {}

    virtual string toString() const;
    virtual int getInt() const;
    virtual ::rtl::OUString getString() const;
    virtual uno::Any getAny() const;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties();
    virtual writerfilter::Reference<Stream>::Pointer_t getStream();
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual WW8Value * clone() const = 0;
};

class WW8IntValue : public WW8Value
{
    int mValue;
public:
    WW8IntValue(int value) : mValue(value) {}
    virtual ~WW8IntValue() {}

    virtual int getInt() const;
    virtual ::rtl::OUString getString() const;
    virtual uno::Any getAny() const;

    virtual string toString() const;

    virtual WW8Value * clone() const { return new WW8IntValue(*this); }
};

/**
   Creates value from an integer.

   @param value   integer to create value from.
*/
WW8Value::Pointer_t createValue(int value);

ostream & operator << (ostream & o, const WW8Value & rValue);

class WW8StringValue : public WW8Value
{
    ::rtl::OUString mString;

public:
    WW8StringValue(::rtl::OUString string_) : mString(string_) {}
    virtual ~WW8StringValue() {}

    virtual int getInt() const;
    virtual ::rtl::OUString getString() const;
    virtual uno::Any getAny() const;

    virtual string toString() const;

    virtual WW8Value * clone() const { return new WW8StringValue(*this); }
};

/**
   Creates value from a string.

   @param rStr   string to create value from.
*/
WW8Value::Pointer_t createValue(const rtl::OUString & rStr);

class WW8PropertiesValue : public WW8Value
{
    mutable writerfilter::Reference<Properties>::Pointer_t mRef;

public:
    WW8PropertiesValue(writerfilter::Reference<Properties>::Pointer_t rRef)
    : mRef(rRef)
    {
    }

    virtual ~WW8PropertiesValue()
    {
    }

    virtual writerfilter::Reference<Properties>::Pointer_t getProperties();

    virtual string toString() const;

    virtual WW8Value * clone() const { return new WW8PropertiesValue(mRef); }
};

class WW8StreamValue : public WW8Value
{
    mutable writerfilter::Reference<Stream>::Pointer_t mRef;

public:
    WW8StreamValue(writerfilter::Reference<Stream>::Pointer_t rRef)
    : mRef(rRef)
    {
    }

    virtual ~WW8StreamValue()
    {
    }

    virtual writerfilter::Reference<Stream>::Pointer_t getStream();

    virtual string toString() const;

    virtual WW8Value * clone() const { return new WW8StreamValue(mRef); }
};

/**
   Creates value from a properties reference.

   @param rRef   reference to create value from.
*/
WW8Value::Pointer_t createValue(writerfilter::Reference<Properties>::Pointer_t rRef);

/**
   Creates value from another value.

   @param value  the value to copy
*/
WW8Value::Pointer_t createValue(WW8Value::Pointer_t value);

/**
   Creates value from a stream reference.

   @param rRef    reference to the stream
 */
WW8Value::Pointer_t createValue(writerfilter::Reference<Stream>::Pointer_t rRef);

class WW8BinaryObjValue : public WW8Value
{
    mutable writerfilter::Reference<BinaryObj>::Pointer_t mRef;

public:
    WW8BinaryObjValue(writerfilter::Reference<BinaryObj>::Pointer_t rRef)
    : mRef(rRef)
    {
    }

    virtual ~WW8BinaryObjValue()
    {
    }

    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();

    virtual string toString() const;

    virtual WW8Value * clone() const { return new WW8BinaryObjValue(mRef); }
};

/**
   Creates value from a binary object reference.

   @param rRef    reference to the stream
 */
WW8Value::Pointer_t createValue(writerfilter::Reference<BinaryObj>::Pointer_t rRef);

Sprm::Kind SprmKind(sal_uInt32 sprmCode);
 
}}

#endif // INCLUDED_WW8_RESOURCE_MODEL_IMPL_HXX
