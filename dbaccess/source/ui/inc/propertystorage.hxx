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

#ifndef PROPERTYSTORAGE_HXX
#define PROPERTYSTORAGE_HXX

#include <com/sun/star/uno/Any.hxx>

#include <tools/solar.h>

#include <boost/shared_ptr.hpp>
#include <map>

class SfxItemSet;

namespace dbaui
{

    // PropertyStorage
    class SAL_NO_VTABLE PropertyStorage
    {
    public:
        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const = 0;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue ) = 0;

        virtual ~PropertyStorage();
    };

    typedef ::boost::shared_ptr< PropertyStorage >      PPropertyStorage;
    typedef ::std::map< sal_Int32, PPropertyStorage >   PropertyValues;

    // SetItemPropertyStorage
    typedef sal_uInt16  ItemId;

    /** a PropertyStorage implementation which stores the value in an item set
    */
    class SetItemPropertyStorage : public PropertyStorage
    {
    public:
        SetItemPropertyStorage( SfxItemSet& _rItemSet, const ItemId _nItemID )
            :m_rItemSet( _rItemSet )
            ,m_nItemID( _nItemID )
        {
        }

        virtual ~SetItemPropertyStorage()
        {
        }

        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue );

    private:
        SfxItemSet&     m_rItemSet;
        const ItemId    m_nItemID;
    };

} // namespace dbaui

#endif // PROPERTYSTORAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
