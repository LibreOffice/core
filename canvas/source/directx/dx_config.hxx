/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_config.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:54:34 $
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
