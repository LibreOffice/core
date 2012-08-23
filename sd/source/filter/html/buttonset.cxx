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

#include "sal/config.h"

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

#include <boost/shared_ptr.hpp>

#include "buttonset.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

class ButtonsImpl
{
public:
    ButtonsImpl( const OUString& rURL );

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

bool ButtonsImpl::getGraphic( const Reference< XGraphicProvider >& xGraphicProvider, const rtl::OUString& rName, Graphic& rGraphic )
{
    Reference< XInputStream > xInputStream( getInputStream( rName ) );
    if( xInputStream.is() && xGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aMediaProperties( 1 );
        aMediaProperties[0].Name = ::rtl::OUString("InputStream"  );
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

typedef std::vector< boost::shared_ptr< ButtonsImpl > > ButtonVector;
class ButtonSetImpl
{
public:
    ButtonSetImpl();

    int getCount() const;

    bool getPreview( int nSet, const std::vector< rtl::OUString >& rButtons, Image& rImage );
    bool exportButton( int nSet, const rtl::OUString& rPath, const rtl::OUString& rName );

    void scanForButtonSets( const OUString& rPath );

    Reference< XGraphicProvider > getGraphicProvider();

    ButtonVector maButtons;
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
    if( aDirectory.open() == osl::FileBase::E_None )
    {
        osl::DirectoryItem aItem;
        while( aDirectory.getNextItem( aItem, 2211 ) == osl::FileBase::E_None )
        {
            osl::FileStatus aStatus( osl_FileStatus_Mask_FileName|osl_FileStatus_Mask_FileURL );
            if( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
            {
                OUString sFileName( aStatus.getFileName() );
                if( sFileName.endsWithIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(".zip" ) ) )
                    maButtons.push_back( boost::shared_ptr< ButtonsImpl >( new ButtonsImpl( aStatus.getFileURL() ) ) );
            }
        }
    }
}

int ButtonSetImpl::getCount() const
{
    return maButtons.size();
}

bool ButtonSetImpl::getPreview( int nSet, const std::vector< rtl::OUString >& rButtons, Image& rImage )
{
    if( (nSet >= 0) && (nSet < static_cast<int>(maButtons.size())))
    {
        ButtonsImpl& rSet = *maButtons[nSet].get();

        std::vector< Graphic > aGraphics;

        VirtualDevice aDev;
        aDev.SetMapMode(MapMode(MAP_PIXEL));

        Size aSize;
        std::vector< rtl::OUString >::const_iterator aIter( rButtons.begin() );
        while( aIter != rButtons.end() )
        {
            Graphic aGraphic;
            if( !rSet.getGraphic( getGraphicProvider(), (*aIter++), aGraphic ) )
                return false;

            aGraphics.push_back(aGraphic);

            Size aGraphicSize( aGraphic.GetSizePixel( &aDev ) );
            aSize.Width() += aGraphicSize.Width();

            if( aSize.Height() < aGraphicSize.Height() )
                aSize.Height() = aGraphicSize.Height();

            if( aIter != rButtons.end() )
                aSize.Width() += 3;
        }

        aDev.SetOutputSizePixel( aSize );

        Point aPos;

        std::vector< Graphic >::iterator aGraphIter( aGraphics.begin() );
        while( aGraphIter != aGraphics.end() )
        {
            Graphic aGraphic( (*aGraphIter++) );

            aGraphic.Draw( &aDev, aPos );

            aPos.X() += aGraphic.GetSizePixel().Width() + 3;
        }

        rImage = Image( aDev.GetBitmapEx( Point(), aSize ) );
        return true;
    }
    return false;
}

bool ButtonSetImpl::exportButton( int nSet, const rtl::OUString& rPath, const rtl::OUString& rName )
{
    if( (nSet >= 0) && (nSet < static_cast<int>(maButtons.size())))
    {
        ButtonsImpl& rSet = *maButtons[nSet].get();

        return rSet.copyGraphic( rName, rPath );
    }
    return false;
}

Reference< XGraphicProvider > ButtonSetImpl::getGraphicProvider()
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
    delete mpImpl;
}

int ButtonSet::getCount() const
{
    return mpImpl->getCount();
}

bool ButtonSet::getPreview( int nSet, const std::vector< rtl::OUString >& rButtons, Image& rImage )
{
    return mpImpl->getPreview( nSet, rButtons, rImage );
}

bool ButtonSet::exportButton( int nSet, const rtl::OUString& rPath, const rtl::OUString& rName )
{
    return mpImpl->exportButton( nSet, rPath, rName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
