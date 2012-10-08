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
        catch( const uno::Exception& )
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
        catch( const uno::Exception& )
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
