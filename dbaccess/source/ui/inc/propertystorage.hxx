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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSTORAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSTORAGE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <map>
#include <memory>

class SfxItemSet;

namespace dbaui
{

    /** a PropertyStorage implementation which stores the value in an item set
    */
    class SetItemPropertyStorage
    {
    public:
        SetItemPropertyStorage( SfxItemSet& _rItemSet, const sal_uInt16 _nItemID )
            :m_rItemSet( _rItemSet )
            ,m_nItemID( _nItemID )
        {
        }

        void getPropertyValue( css::uno::Any& _out_rValue ) const;
        void setPropertyValue( const css::uno::Any& _rValue );

    private:
        SfxItemSet&       m_rItemSet;
        const sal_uInt16  m_nItemID;
    };

    typedef ::std::map< sal_Int32, std::shared_ptr< SetItemPropertyStorage > >   PropertyValues;

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSTORAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
