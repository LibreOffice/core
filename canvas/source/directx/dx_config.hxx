/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_config.hxx,v $
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

    private:
        typedef std::set< DeviceInfo > ValueSet;
        ValueSet                   maValues;
        boost::optional<sal_Int32> maMaxTextureSize;
        bool                       mbBlacklistCurrentDevice;
        bool                       mbValuesDirty;
    };
}

#endif /* #ifndef _DXCANVAS_DXCONFIG_HXX */
