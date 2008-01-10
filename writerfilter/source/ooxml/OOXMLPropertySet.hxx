/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySet.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:59:04 $
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
#ifndef INCLUDED_OOXML_PROPERTY_SET_HXX
#define INCLUDED_OOXML_PROPERTY_SET_HXX

#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
namespace ooxml
{

class OOXMLProperty : public Sprm
{
public:
    typedef boost::shared_ptr<OOXMLProperty> Pointer_t;

    virtual ~OOXMLProperty();

    virtual sal_uInt32 getId() const = 0;
    virtual Value::Pointer_t getValue() = 0;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() = 0;
    virtual writerfilter::Reference<Stream>::Pointer_t getStream() = 0;
    virtual writerfilter::Reference<Properties>::Pointer_t getProps() = 0;
    virtual string getName() const = 0;
    virtual string toString() const = 0;
    virtual void resolve(Properties & rProperties) = 0;

    virtual Sprm * clone() = 0;
};

class OOXMLPropertySet : public writerfilter::Reference<Properties>
{
public:
    typedef boost::shared_ptr<OOXMLPropertySet> Pointer_t;

    virtual ~OOXMLPropertySet();

    virtual void resolve(Properties & rHandler) = 0;
    virtual string getType() const = 0;
    virtual void add(OOXMLProperty::Pointer_t pProperty) = 0;
    virtual void add(OOXMLPropertySet::Pointer_t pPropertySet) = 0;
    virtual OOXMLPropertySet * clone() const = 0;
    virtual void setType(const string & rsType) = 0;

    virtual string toString() = 0;
};

class OOXMLTable : public writerfilter::Reference<Table>
{
public:
    virtual ~OOXMLTable();

    virtual void resolve(Table & rTable) = 0;
    virtual OOXMLTable * clone() const = 0;
};
}}

#endif // INCLUDED_OOXML_PROPERTY_SET_HXX
