/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmldlg_addfunc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:11:51 $
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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmldlg_imexp.hxx>


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

//==================================================================================================
class InputStreamProvider
    : public ::cppu::WeakImplHelper1< io::XInputStreamProvider >
{
    ByteSequence _bytes;

public:
    inline InputStreamProvider( ByteSequence const & rBytes )
        : _bytes( rBytes )
        {}

    // XInputStreamProvider
    virtual Reference< io::XInputStream > SAL_CALL createInputStream()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
Reference< io::XInputStream > InputStreamProvider::createInputStream()
    throw (RuntimeException)
{
    return ::xmlscript::createInputStream( _bytes );
}

//==================================================================================================
Reference< io::XInputStreamProvider > SAL_CALL exportDialogModel(
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSMgr( xContext->getServiceManager() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XExtendedDocumentHandler > xHandler( xSMgr->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ), xContext ), UNO_QUERY );
    OSL_ASSERT( xHandler.is() );
    if (! xHandler.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create sax-writer component!") ),
            Reference< XInterface >() );
    }

    ByteSequence aBytes;

    Reference< io::XActiveDataSource > xSource( xHandler, UNO_QUERY );
    xSource->setOutputStream( createOutputStream( &aBytes ) );
    exportDialogModel( xHandler, xDialogModel );

    return new InputStreamProvider( aBytes );
}

//==================================================================================================
void SAL_CALL importDialogModel(
    Reference< io::XInputStream > xInput,
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSMgr( xContext->getServiceManager() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XParser > xParser( xSMgr->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ), xContext ), UNO_QUERY );
    OSL_ASSERT( xParser.is() );
    if (! xParser.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create sax-parser component!") ),
            Reference< XInterface >() );
    }

    // error handler, entity resolver omitted for this helper function
    xParser->setDocumentHandler( importDialogModel( xDialogModel, xContext ) );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUString( RTL_CONSTASCII_USTRINGPARAM("virtual file") );

    xParser->parseStream( source );
}

}
