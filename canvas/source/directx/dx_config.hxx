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



#ifndef _DXCANVAS_DXCONFIG_HXX
#define _DXCANVAS_DXCONFIG_HXX

#include <unotools/configitem.hxx>
#include <boost/optional.hpp>
#include <set>

namespace basegfx { class B2IVector; }

namespace dxcanvas
{
    /** Provide DX canvas config data
     */
    class DXCanvasItem : public ::utl::ConfigItem
    {
    public:
        DXCanvasItem();

        struct DeviceInfo
        {
            sal_Int32 nVendorId;
            sal_Int32 nDeviceId;
            sal_Int32 nDeviceSubSysId;
            sal_Int32 nDeviceRevision;

            sal_Int32 nDriverId;
            sal_Int32 nDriverVersion;
            sal_Int32 nDriverSubVersion;
            sal_Int32 nDriverBuildId;

            bool operator<( const DeviceInfo& rRHS ) const
            {
                return nVendorId != rRHS.nVendorId ? nVendorId < rRHS.nVendorId :
                    (nDeviceId != rRHS.nDeviceId ? nDeviceId < rRHS.nDeviceId :
                     (nDeviceSubSysId != rRHS.nDeviceSubSysId ? nDeviceSubSysId < rRHS.nDeviceSubSysId :
                      (nDeviceRevision != rRHS.nDeviceRevision ? nDeviceRevision < rRHS.nDeviceRevision :
                       (nDriverId != rRHS.nDriverId ? nDriverId < rRHS.nDriverId :
                        (nDriverVersion != rRHS.nDriverVersion ? nDriverVersion < rRHS.nDriverVersion :
                         (nDriverSubVersion != rRHS.nDriverSubVersion ? nDriverSubVersion < rRHS.nDriverSubVersion :
                          (nDriverBuildId != rRHS.nDriverBuildId ? nDriverBuildId < rRHS.nDriverBuildId : false)))))));
            }
        };

        ~DXCanvasItem();

        bool isDeviceUsable( const DeviceInfo& rDeviceInfo ) const;
        bool isBlacklistCurrentDevice() const;
        void blacklistDevice( const DeviceInfo& rDeviceInfo );
        void adaptMaxTextureSize( basegfx::B2IVector& io_maxTextureSize ) const;
            virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
            virtual void            Commit();

    private:
        typedef std::set< DeviceInfo > ValueSet;
        ValueSet                   maValues;
        boost::optional<sal_Int32> maMaxTextureSize;
        bool                       mbBlacklistCurrentDevice;
        bool                       mbValuesDirty;
    };
}

#endif /* #ifndef _DXCANVAS_DXCONFIG_HXX */
