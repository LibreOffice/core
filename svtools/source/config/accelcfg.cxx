/*************************************************************************
 *
 *  $RCSfile: accelcfg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:05:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef GCC
#pragma hdrstop
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>

#include "accelcfg.hxx"
#include "xmlaccelcfg.hxx"
#include "pathoptions.hxx"


using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::xml::sax;


static SvtAcceleratorConfig_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

class SvtAcceleratorConfig_Impl
{
public:

    SvtAcceleratorItemList aList;
    bool            bModified;

                    SvtAcceleratorConfig_Impl()
                        : bModified( FALSE )
                    {}

                    SvtAcceleratorConfig_Impl( Reference< XInputStream >& xInputStream );
    bool            Commit( Reference< XOutputStream >& xOutputStream );
};

// -----------------------------------------------------------------------

SvtAcceleratorConfig_Impl::SvtAcceleratorConfig_Impl( Reference< XInputStream >& rInputStream )
        : bModified( false )
{
    Reference< XParser > xParser( ::comphelper::getProcessServiceFactory()->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )),
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
            ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;

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

SvtAcceleratorConfiguration::SvtAcceleratorConfiguration()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
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
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
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
    for ( p = pImp->aList.begin(); p != pImp->aList.end(); p++ )
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
    for ( p = pImp->aList.begin(); p != pImp->aList.end(); p++ )
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
        for ( p = rItems.begin(); p != rItems.end(); p++ )
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
