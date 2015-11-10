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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_DBREGISTERSETTINGS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_DBREGISTERSETTINGS_HXX

#include <sal/config.h>

#include <map>

#include <svl/poolitem.hxx>


namespace svx
{


    struct DatabaseRegistration
    {
        OUString sLocation;
        bool            bReadOnly;

        DatabaseRegistration()
            :sLocation()
            ,bReadOnly( true )
        {
        }

        DatabaseRegistration( const OUString& _rLocation, const bool _bReadOnly )
            :sLocation( _rLocation )
            ,bReadOnly( _bReadOnly )
        {
        }

        bool operator==( const DatabaseRegistration& _rhs ) const
        {
            return  ( sLocation == _rhs.sLocation );
                // do not take the read-only-ness into account, this is not maintained everywhere, but only
                // properly set when filling the struct from the XDatabaseRegistrations data
        }

        bool operator!=( const DatabaseRegistration& _rhs ) const
        {
            return !( this->operator==( _rhs ) );
        }
    };

    typedef ::std::map< OUString, DatabaseRegistration > DatabaseRegistrations;

    class DatabaseMapItem : public SfxPoolItem
    {
    protected:
        DatabaseRegistrations   m_aRegistrations;

    public:
        TYPEINFO_OVERRIDE();

        DatabaseMapItem( sal_uInt16 _nId, const DatabaseRegistrations& _rRegistrations );

        virtual bool             operator==( const SfxPoolItem& ) const override;
        virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

        const DatabaseRegistrations&
                                getRegistrations() const { return m_aRegistrations; }
    };


}

#endif // INCLUDED_CUI_SOURCE_OPTIONS_DBREGISTERSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
