/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySetImpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-02-21 12:26:58 $
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
class OOXMLPropertySprm : public OOXMLProperty
{
    Id mId;
    mutable Value::Pointer_t mpValue;

public:
    typedef boost::shared_ptr<OOXMLProperty> Pointer_t;

    OOXMLPropertySprm(Id id, Value::Pointer_t pValue);
    OOXMLPropertySprm(const OOXMLPropertySprm & rSprm);
    virtual ~OOXMLPropertySprm();

    virtual sal_uInt32 getId() const;
    virtual Value::Pointer_t getValue();
    virtual doctok::Reference<BinaryObj>::Pointer_t getBinary();
    virtual doctok::Reference<Stream>::Pointer_t getStream();
    virtual doctok::Reference<Properties>::Pointer_t getProps();
    virtual string getName() const;
    virtual string toString() const;
    virtual Sprm * clone();
};

class OOXMLPropertySetImpl : public OOXMLPropertySet
{
    typedef vector<OOXMLProperty::Pointer_t> OOXMLProperties_t;
    OOXMLProperties_t mProperties;
public:
    OOXMLPropertySetImpl();
    virtual ~OOXMLPropertySetImpl();
    virtual void resolve(Properties & rHandler);
    virtual string getType() const;
    virtual void add(OOXMLProperty::Pointer_t pProperty);
};

}

#endif // INCLUDED_OOXML_PROPERTY_SET_IMPL_HXX
