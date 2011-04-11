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
#include "precompiled_unotools.hxx"

#include "rtl/instance.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>

#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <unotools/accelcfg.hxx>
#include <unotools/xmlaccelcfg.hxx>
#include <unotools/pathoptions.hxx>
#include "itemholder1.hxx"


using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::xml::sax;

using ::rtl::OUString;


static SvtAcceleratorConfig_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

class SvtAcceleratorConfig_Impl
{
public:

    SvtAcceleratorItemList aList;
    bool            bModified;

                    SvtAcceleratorConfig_Impl()
                        : bModified( sal_False )
                    {}

                    SvtAcceleratorConfig_Impl( Reference< XInputStream >& xInputStream );
    bool            Commit( Reference< XOutputStream >& xOutputStream );
};

// -----------------------------------------------------------------------

SvtAcceleratorConfig_Impl::SvtAcceleratorConfig_Impl( Reference< XInputStream >& rInputStream )
        : bModified( false )
{
    Reference< XParser > xParser( ::comphelper::getProcessServiceFactory()->createInstance(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"))),
                                  UNO_QUERY);

    // connect stream to input stream to the parser
    InputSource aInputSource;
    aInputSource.aInputStream = rInputStream;

    // get filter
    Reference< XDocumentHandler > xFilter( new OReadAccelatorDocumentHandler( aList ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );
    xParser->parseStream( aInputSource );
}

bool SvtAcceleratorConfig_Impl::Commit( Reference< XOutputStream >& rOutputStream )
{
    Reference< XDocumentHandler > xWriter;

    xWriter = Reference< XDocumentHandler >( ::comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))), UNO_QUERY) ;

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( rOutputStream );
    try
    {
        OWriteAccelatorDocumentHandler aWriteHandler( aList, xWriter );
        aWriteHandler.WriteAcceleratorDocument();
        rOutputStream->flush();
        return true;
    }
    catch ( RuntimeException& )
    {
    }
    catch ( SAXException& )
    {
    }
    catch ( ::com::sun::star::io::IOException& )
    {
    }

    return false;
}

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

SvtAcceleratorConfiguration::SvtAcceleratorConfiguration()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( LocalSingleton::get() );
    if ( !pOptions )
    {
        SvStream* pStream = GetDefaultStream( STREAM_STD_READ );
        ::utl::OInputStreamWrapper aHelper( *pStream );
        com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xOut( &aHelper );

        try
        {
            pOptions = new SvtAcceleratorConfig_Impl( xOut );
        }
        catch ( RuntimeException& )
        {
            pOptions = new SvtAcceleratorConfig_Impl();
        }
        catch( SAXException& )
        {
            pOptions = new SvtAcceleratorConfig_Impl();
        }
        catch( ::com::sun::star::io::IOException& )
        {
            pOptions = new SvtAcceleratorConfig_Impl();
        }

        if (pOptions)
            ItemHolder1::holdConfigItem(E_ACCELCFG);

        delete pStream;
    }

    ++nRefCount;
    pImp = pOptions;
}

SvtAcceleratorConfiguration* SvtAcceleratorConfiguration::CreateFromStream( SvStream& rStream )
{
    SvtAcceleratorConfiguration* pRet = new SvtAcceleratorConfiguration;
    ::utl::OInputStreamWrapper aHelper( rStream );
    com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xOut( &aHelper );
    try
    {
        pRet->pImp = new SvtAcceleratorConfig_Impl( xOut );
    }
    catch ( RuntimeException& )
    {
        DELETEZ( pRet );
    }
    catch( SAXException& )
    {
        DELETEZ( pRet );
    }
    catch( ::com::sun::star::io::IOException& )
    {
        DELETEZ( pRet );
    }

    return pRet;
}

// -----------------------------------------------------------------------

SvtAcceleratorConfiguration::~SvtAcceleratorConfiguration()
{
    if ( pImp == pOptions )
    {
        // Global access, must be guarded (multithreading)
        ::osl::MutexGuard aGuard( LocalSingleton::get() );
        if ( !--nRefCount )
        {
            if ( pImp->bModified )
            {
                String aUserConfig = SvtPathOptions().GetUserConfigPath();
                INetURLObject aObj( aUserConfig );
                aObj.insertName( String::CreateFromAscii("GlobalKeyBindings.xml") );
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READWRITE|STREAM_TRUNC );
                ::utl::OOutputStreamWrapper aHelper( *pStream );
                com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOut( &aHelper );
                pImp->Commit( xOut );
                delete pStream;
            }

            DELETEZ( pOptions );
        }
    }
    else
    {
        delete pImp;
    }
}

::rtl::OUString SvtAcceleratorConfiguration::GetCommand( const ::com::sun::star::awt::KeyEvent& rKeyEvent )
{
    sal_Int16 nCode=rKeyEvent.KeyCode, nModifier=rKeyEvent.Modifiers;
    if ( !nCode )
        nCode = rKeyEvent.KeyFunc;

    std::list< SvtAcceleratorConfigItem>::const_iterator p;
    for ( p = pImp->aList.begin(); p != pImp->aList.end(); ++p )
        if ( p->nCode == nCode && p->nModifier == nModifier )
            return p->aCommand;

    return ::rtl::OUString();
}

const SvtAcceleratorItemList& SvtAcceleratorConfiguration::GetItems()
{
    return pImp->aList;
}

void SvtAcceleratorConfiguration::SetCommand( const SvtAcceleratorConfigItem& rItem )
{
    std::list< SvtAcceleratorConfigItem>::iterator p;
    for ( p = pImp->aList.begin(); p != pImp->aList.end(); ++p )
        if ( p->nCode == rItem.nCode && p->nModifier == rItem.nModifier )
        {
            p->aCommand = rItem.aCommand;
            return;
        }

    pImp->aList.push_back( rItem );

}

void SvtAcceleratorConfiguration::SetItems( const SvtAcceleratorItemList& rItems, bool bClear )
{
    if ( bClear )
    {
        pImp->aList = rItems;
    }
    else
    {
        std::list< SvtAcceleratorConfigItem>::const_iterator p;
        for ( p = rItems.begin(); p != rItems.end(); ++p )
            SetCommand( *p );
    }
}

String SvtAcceleratorConfiguration::GetStreamName()
{
    return String::CreateFromAscii("KeyBindings.xml");
}

SvStream* SvtAcceleratorConfiguration::GetDefaultStream( StreamMode nMode )
{
    String aUserConfig = SvtPathOptions().GetUserConfigPath();
    INetURLObject aObj( aUserConfig );
    aObj.insertName( GetStreamName() );
    return ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), nMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
