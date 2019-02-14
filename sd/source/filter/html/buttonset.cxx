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

#include <sal/config.h>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XStream.hpp>

#include <osl/file.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/oslfile2streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <unotools/pathoptions.hxx>

#include <memory>

#include "buttonset.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

class ButtonsImpl
{
public:
    explicit ButtonsImpl( const OUString& rURL );

    Reference< XInputStream > getInputStream( const OUString& rName );

    bool getGraphic( const Reference< XGraphicProvider >& xGraphicProvider, const OUString& rName, Graphic& rGraphic );

    bool copyGraphic( const OUString& rName, const OUString& rPath );

private:
    Reference< XStorage > mxStorage;
};

ButtonsImpl::ButtonsImpl( const OUString& rURL )
{
    try
    {
        mxStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL( ZIP_STORAGE_FORMAT_STRING, rURL, ElementModes::READ );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::ButtonsImpl::ButtonsImpl(), exception caught!" );
    }
}

Reference< XInputStream > ButtonsImpl::getInputStream( const OUString& rName )
{
    Reference< XInputStream > xInputStream;
    if( mxStorage.is() ) try
    {
        Reference< XStream > xStream( mxStorage->openStreamElement( rName, ElementModes::READ ) );
        if( xStream.is() )
            xInputStream = xStream->getInputStream();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::ButtonsImpl::getInputStream(), exception caught!" );
    }
    return xInputStream;
}

bool ButtonsImpl::getGraphic( const Reference< XGraphicProvider >& xGraphicProvider, const OUString& rName, Graphic& rGraphic )
{
    Reference< XInputStream > xInputStream( getInputStream( rName ) );
    if( xInputStream.is() && xGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aMediaProperties( 1 );
        aMediaProperties[0].Name = "InputStream";
        aMediaProperties[0].Value <<= xInputStream;
        Reference< XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties  ) );

        if( xGraphic.is() )
        {
            rGraphic = Graphic( xGraphic );
            return true;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::ButtonsImpl::getGraphic(), exception caught!" );
    }
    return false;
}

bool ButtonsImpl::copyGraphic( const OUString& rName, const OUString& rPath )
{
    Reference< XInputStream > xInput( getInputStream( rName ) );
    if( xInput.is() ) try
    {
        osl::File::remove( rPath );
        osl::File aOutputFile( rPath );
        if( aOutputFile.open( osl_File_OpenFlag_Write|osl_File_OpenFlag_Create ) == osl::FileBase::E_None )
        {
            Reference< XOutputStream > xOutput( new comphelper::OSLOutputStreamWrapper( aOutputFile ) );
            comphelper::OStorageHelper::CopyInputToOutput( xInput, xOutput );
            return true;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::ButtonsImpl::copyGraphic(), exception caught!" );
    }

    return false;
}

class ButtonSetImpl
{
public:
    ButtonSetImpl();

    int getCount() const;

    bool getPreview( int nSet, const std::vector< OUString >& rButtons, Image& rImage );
    bool exportButton( int nSet, const OUString& rPath, const OUString& rName );

    void scanForButtonSets( const OUString& rPath );

    Reference< XGraphicProvider > const & getGraphicProvider();

    std::vector< std::shared_ptr< ButtonsImpl > >  maButtons;
    Reference< XGraphicProvider > mxGraphicProvider;
};

ButtonSetImpl::ButtonSetImpl()
{
    const OUString sSubPath( "/wizard/web/buttons"  );

    OUString sSharePath( SvtPathOptions().GetConfigPath() );
    sSharePath += sSubPath;
    scanForButtonSets( sSharePath );

    OUString sUserPath( SvtPathOptions().GetUserConfigPath() );
    sUserPath += sSubPath;
    scanForButtonSets( sUserPath );
}

void ButtonSetImpl::scanForButtonSets( const OUString& rPath )
{
    osl::Directory aDirectory( rPath );
    if( aDirectory.open() != osl::FileBase::E_None )
        return;

    osl::DirectoryItem aItem;
    while( aDirectory.getNextItem( aItem, 2211 ) == osl::FileBase::E_None )
    {
        osl::FileStatus aStatus( osl_FileStatus_Mask_FileName|osl_FileStatus_Mask_FileURL );
        if( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
        {
            OUString sFileName( aStatus.getFileName() );
            if( sFileName.endsWithIgnoreAsciiCase( ".zip" ) )
                maButtons.push_back( std::make_shared< ButtonsImpl >( aStatus.getFileURL() ) );
        }
    }
}

int ButtonSetImpl::getCount() const
{
    return maButtons.size();
}

bool ButtonSetImpl::getPreview( int nSet, const std::vector< OUString >& rButtons, Image& rImage )
{
    if( (nSet >= 0) && (nSet < static_cast<int>(maButtons.size())))
    {
        ButtonsImpl& rSet = *maButtons[nSet].get();

        std::vector< Graphic > aGraphics;

        ScopedVclPtrInstance< VirtualDevice > pDev;
        pDev->SetMapMode(MapMode(MapUnit::MapPixel));

        Size aSize;
        std::vector< OUString >::const_iterator aIter( rButtons.begin() );
        while( aIter != rButtons.end() )
        {
            Graphic aGraphic;
            if( !rSet.getGraphic( getGraphicProvider(), (*aIter++), aGraphic ) )
                return false;

            aGraphics.push_back(aGraphic);

            Size aGraphicSize( aGraphic.GetSizePixel( pDev ) );
            aSize.AdjustWidth(aGraphicSize.Width() );

            if( aSize.Height() < aGraphicSize.Height() )
                aSize.setHeight( aGraphicSize.Height() );

            if( aIter != rButtons.end() )
                aSize.AdjustWidth(3 );
        }

        pDev->SetOutputSizePixel( aSize );

        Point aPos;

        for( Graphic& aGraphic : aGraphics )
        {
            aGraphic.Draw( pDev, aPos );

            aPos.AdjustX(aGraphic.GetSizePixel().Width() + 3 );
        }

        rImage = Image( pDev->GetBitmapEx( Point(), aSize ) );
        return true;
    }
    return false;
}

bool ButtonSetImpl::exportButton( int nSet, const OUString& rPath, const OUString& rName )
{
    if( (nSet >= 0) && (nSet < static_cast<int>(maButtons.size())))
    {
        ButtonsImpl& rSet = *maButtons[nSet].get();

        return rSet.copyGraphic( rName, rPath );
    }
    return false;
}

Reference< XGraphicProvider > const & ButtonSetImpl::getGraphicProvider()
{
    if( !mxGraphicProvider.is() )
    {
        Reference< XComponentContext > xComponentContext = ::comphelper::getProcessComponentContext();
        mxGraphicProvider = GraphicProvider::create(xComponentContext);
    }
    return mxGraphicProvider;
}

ButtonSet::ButtonSet()
: mpImpl( new ButtonSetImpl() )
{
}

ButtonSet::~ButtonSet()
{
}

int ButtonSet::getCount() const
{
    return mpImpl->getCount();
}

bool ButtonSet::getPreview( int nSet, const std::vector< OUString >& rButtons, Image& rImage )
{
    return mpImpl->getPreview( nSet, rButtons, rImage );
}

bool ButtonSet::exportButton( int nSet, const OUString& rPath, const OUString& rName )
{
    return mpImpl->exportButton( nSet, rPath, rName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
