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


#include "rtl/instance.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>

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
    Reference< XParser > xParser = Parser::create( ::comphelper::getProcessComponentContext() );

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
            ::rtl::OUString("com.sun.star.xml.sax.Writer")), UNO_QUERY) ;

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
                aObj.insertName( rtl::OUString("GlobalKeyBindings.xml") );
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READWRITE|STREAM_TRUNC );
                com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > xOut( new utl::OOutputStreamWrapper( *pStream ) );
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

String SvtAcceleratorConfiguration::GetStreamName()
{
    return rtl::OUString("KeyBindings.xml");
}

SvStream* SvtAcceleratorConfiguration::GetDefaultStream( StreamMode nMode )
{
    String aUserConfig = SvtPathOptions().GetUserConfigPath();
    INetURLObject aObj( aUserConfig );
    aObj.insertName( GetStreamName() );
    return ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), nMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
