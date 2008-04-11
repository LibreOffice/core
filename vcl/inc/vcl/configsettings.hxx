/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configsettings.hxx,v $
 * $Revision: 1.3 $
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
#ifndef VCL_INC_CONFIGSETTINGS_HXX
#define VCL_INC_CONFIGSETTINGS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/configitem.hxx>
#include <vcl/dllapi.h>

#include <hash_map>

//........................................................................
namespace vcl
{
//........................................................................

    typedef std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash > OUStrMap;
    class SmallOUStrMap : public OUStrMap { public: SmallOUStrMap() : OUStrMap(1) {} };

    //====================================================================
    //= SettingsConfigItem
    //====================================================================
    class VCL_DLLPUBLIC SettingsConfigItem : public ::utl::ConfigItem
    {

        std::hash_map< rtl::OUString, SmallOUStrMap, rtl::OUStringHash > m_aSettings;

        virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames );
        virtual void Commit();

        void getValues();
        SettingsConfigItem();
    public:
        virtual ~SettingsConfigItem();

        static SettingsConfigItem* get();

        const rtl::OUString& getValue( const rtl::OUString& rGroup, const rtl::OUString& rKey ) const;
        void setValue( const rtl::OUString& rGroup, const rtl::OUString& rKey, const rtl::OUString& rValue );
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_INC_CONFIGSETTINGS_HXX

