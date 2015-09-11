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

#ifndef INCLUDED_COMPHELPER_PROPERTYSETINFO_HXX
#define INCLUDED_COMPHELPER_PROPERTYSETINFO_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

struct PropertyMapEntry
{
    OUString maName;
    sal_Int32 mnHandle;
    com::sun::star::uno::Type maType;
    sal_Int16 mnAttributes;
    sal_uInt8 mnMemberId;
};

typedef std::map<OUString, PropertyMapEntry const *> PropertyMap;

class PropertyMapImpl;

/** this class implements a XPropertySetInfo that is initialized with arrays of PropertyMapEntry.
    It is used by the class PropertySetHelper.
*/
class COMPHELPER_DLLPUBLIC PropertySetInfo : public ::cppu::WeakImplHelper< ::com::sun::star::beans::XPropertySetInfo >
{
private:
    PropertyMapImpl* mpMap;
public:
    PropertySetInfo() throw();
    PropertySetInfo( PropertyMapEntry const * pMap ) throw();
    PropertySetInfo(css::uno::Sequence<css::beans::Property> const &) throw();
    virtual ~PropertySetInfo() throw();

    /** returns a stl map with all PropertyMapEntry pointer.<p>
        The key is the property name.
    */
    const PropertyMap& getPropertyMap() const throw();

    /** adds an array of PropertyMapEntry to this instance.<p>
        The end is marked with a PropertyMapEntry where mpName equals NULL</p>
    */
    void add( PropertyMapEntry const * pMap ) throw();

    /** removes an already added PropertyMapEntry which string in mpName equals to aName */
    void remove( const OUString& aName ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

}

#endif // _UTL_PROPERTSETINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
