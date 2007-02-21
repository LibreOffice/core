/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySetImpl.cxx,v $
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

 #include "OOXMLPropertySetImpl.hxx"

namespace ooxml
{

using namespace doctok;

OOXMLPropertySprm::OOXMLPropertySprm(Id id, Value::Pointer_t pValue)
: mId(id), mpValue(pValue)
{
}

OOXMLPropertySprm::OOXMLPropertySprm(const OOXMLPropertySprm & rSprm)
: OOXMLProperty(), mId(rSprm.mId), mpValue(rSprm.mpValue)
{
}

OOXMLPropertySprm::~OOXMLPropertySprm()
{
}

sal_uInt32 OOXMLPropertySprm::getId() const
{
    return mId;
}

Value::Pointer_t OOXMLPropertySprm::getValue()
{
    return mpValue;
}

doctok::Reference<BinaryObj>::Pointer_t OOXMLPropertySprm::getBinary()
{
    return doctok::Reference<BinaryObj>::Pointer_t();
}

doctok::Reference<Stream>::Pointer_t OOXMLPropertySprm::getStream()
{
    return doctok::Reference<Stream>::Pointer_t();
}

doctok::Reference<Properties>::Pointer_t OOXMLPropertySprm::getProps()
{
    return doctok::Reference<Properties>::Pointer_t();
}

string OOXMLPropertySprm::getName() const
{
    return "??";
}

string OOXMLPropertySprm::toString() const
{
    return "OOXMLPropertySprm";
}

Sprm * OOXMLPropertySprm::clone()
{
    return new OOXMLPropertySprm(*this);
}

/**
   class OOXMLPropertySetImpl
*/

OOXMLPropertySetImpl::OOXMLPropertySetImpl()
{
}

OOXMLPropertySetImpl::~OOXMLPropertySetImpl()
{
}

void OOXMLPropertySetImpl::resolve(Properties & rHandler)
{
    OOXMLProperties_t::iterator aIt = mProperties.begin();
    while (aIt != mProperties.end())
    {
        OOXMLProperty::Pointer_t pProp = *aIt;

        rHandler.sprm(*pProp);

        aIt++;
    }
}

string OOXMLPropertySetImpl::getType() const
{
    return "OOXMLPropertySetImpl";
}

void OOXMLPropertySetImpl::add(OOXMLProperty::Pointer_t pProperty)
{
    mProperties.push_back(pProperty);

}

}
