/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configsettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:49:50 $
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
#ifndef VCL_INC_CONFIGSETTINGS_HXX
#define VCL_INC_CONFIGSETTINGS_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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

