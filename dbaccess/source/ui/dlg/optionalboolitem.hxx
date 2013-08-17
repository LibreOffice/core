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

#ifndef DBACCESS_OPTIONALBOOLITEM_HXX
#define DBACCESS_OPTIONALBOOLITEM_HXX

#include <svl/poolitem.hxx>

#include <boost/optional.hpp>

namespace dbaui
{

    // OptionalBoolItem
    class OptionalBoolItem : public SfxPoolItem
    {
        ::boost::optional< bool >   m_aValue;

    public:
        TYPEINFO();
        OptionalBoolItem( sal_Int16 nWhich );
        OptionalBoolItem( const OptionalBoolItem& _rSource );

        virtual int              operator==( const SfxPoolItem& _rItem ) const;
        virtual SfxPoolItem*     Clone( SfxItemPool* _pPool = NULL ) const;

        bool    HasValue() const                { return !!m_aValue; }
        void    ClearValue()                    { m_aValue.reset(); }
        bool    GetValue() const                { return *m_aValue; }
        void    SetValue( const bool _bValue )  { m_aValue.reset( _bValue ); }

        const ::boost::optional< bool >&
            GetFullValue() const { return m_aValue; }
    };

} // namespace dbaui

#endif // DBACCESS_OPTIONALBOOLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
