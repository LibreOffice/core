/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include "dx_config.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/anytostring.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <cppuhelper/exc_hlp.hxx>

using namespace com::sun::star;

namespace dxcanvas
{
    DXCanvasItem::DXCanvasItem() :
        ConfigItem(
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "Office.Canvas/DXCanvas" )),
            CONFIG_MODE_IMMEDIATE_UPDATE ),
        maValues(),
        maMaxTextureSize(),
        mbBlacklistCurrentDevice(false),
        mbValuesDirty(false)
    {
        try
        {
            uno::Sequence< ::rtl::OUString > aName(1);
            aName[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DeviceBlacklist" ));

            uno::Sequence< uno::Any > aProps( GetProperties( aName ));
            uno::Sequence< sal_Int32 > aValues;

            if( aProps.getLength() > 0 &&
                (aProps[0] >>= aValues) )
            {
                const sal_Int32* pValues = aValues.getConstArray();
                const sal_Int32 nNumEntries( aValues.getLength()*sizeof(sal_Int32)/sizeof(DeviceInfo) );
                for( sal_Int32 i=0; i<nNumEntries; ++i )
                {
                    DeviceInfo aInfo;
                    aInfo.nVendorId         = *pValues++;
                    aInfo.nDeviceId         = *pValues++;
                    aInfo.nDeviceSubSysId   = *pValues++;
                    aInfo.nDeviceRevision   = *pValues++;
                    aInfo.nDriverId         = *pValues++;
                    aInfo.nDriverVersion    = *pValues++;
                    aInfo.nDriverSubVersion = *pValues++;
                    aInfo.nDriverBuildId    = *pValues++;
                    maValues.insert(aInfo);
                }
            }

            aName[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BlacklistCurrentDevice" ));
            aProps = GetProperties( aName );
            if( aProps.getLength() > 0 )
                aProps[0] >>= mbBlacklistCurrentDevice;

            aName[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxTextureSize" ));
            aProps = GetProperties( aName );
            if( aProps.getLength() > 0 )
                maMaxTextureSize.reset( aProps[0].get<sal_Int32>() );
            else
                maMaxTextureSize.reset();
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( rtl::OUStringToOString(
                            comphelper::anyToString( cppu::getCaughtException() ),
                            RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    DXCanvasItem::~DXCanvasItem()
    {
        if( !mbValuesDirty )
            return;

        try
        {
            uno::Sequence< ::rtl::OUString > aName(1);
            aName[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DeviceBlacklist" ));

            uno::Sequence< sal_Int32 > aValues( sizeof(DeviceInfo)/sizeof(sal_Int32)*maValues.size() );

            sal_Int32* pValues = aValues.getArray();
            ValueSet::const_iterator aIter( maValues.begin() );
            const ValueSet::const_iterator aEnd( maValues.end() );
            while( aIter != aEnd )
            {
                const DeviceInfo& rInfo( *aIter );
                *pValues++ = rInfo.nVendorId;
                *pValues++ = rInfo.nDeviceId;
                *pValues++ = rInfo.nDeviceSubSysId;
                *pValues++ = rInfo.nDeviceRevision;
                *pValues++ = rInfo.nDriverId;
                *pValues++ = rInfo.nDriverVersion;
                *pValues++ = rInfo.nDriverSubVersion;
                *pValues++ = rInfo.nDriverBuildId;
                ++aIter;
            }

            uno::Sequence< uno::Any > aValue(1);
            aValue[0] <<= aValues;
            PutProperties( aName, aValue );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( rtl::OUStringToOString(
                            comphelper::anyToString( cppu::getCaughtException() ),
                            RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    void DXCanvasItem::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& ) {}
    void DXCanvasItem::Commit() {}

    bool DXCanvasItem::isDeviceUsable( const DeviceInfo& rDeviceInfo ) const
    {
        return maValues.find(rDeviceInfo) == maValues.end();
    }

    bool DXCanvasItem::isBlacklistCurrentDevice() const
    {
        return mbBlacklistCurrentDevice;
    }

    void DXCanvasItem::blacklistDevice( const DeviceInfo& rDeviceInfo )
    {
        mbValuesDirty = true;
        maValues.insert(rDeviceInfo);
    }

    void DXCanvasItem::adaptMaxTextureSize( basegfx::B2IVector& io_maxTextureSize ) const
    {
        if( maMaxTextureSize )
        {
            io_maxTextureSize.setX(
                std::min( *maMaxTextureSize,
                          io_maxTextureSize.getX() ));
            io_maxTextureSize.setY(
                std::min( *maMaxTextureSize,
                          io_maxTextureSize.getY() ));
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
