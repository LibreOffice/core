/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

        void getValues();
        SettingsConfigItem();
    public:
        virtual ~SettingsConfigItem();

        static SettingsConfigItem* get();

        const rtl::OUString& getValue( const rtl::OUString& rGroup, const rtl::OUString& rKey ) const;
        void setValue( const rtl::OUString& rGroup, const rtl::OUString& rKey, const rtl::OUString& rValue );

        virtual void Commit();
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_INC_CONFIGSETTINGS_HXX

