/*************************************************************************
 *
 *  $RCSfile: xmldlg_addfunc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-14 16:39:59 $
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
SAL_DLLEXPORT Reference< io::XInputStreamProvider > SAL_CALL exportDialogModel(
    Reference< container::XNameContainer > const & xDialogModel )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiServiceFactory > xSMgr( ::comphelper::getProcessServiceFactory() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XExtendedDocumentHandler > xHandler( xSMgr->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
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
SAL_DLLEXPORT void SAL_CALL importDialogModel(
    Reference< io::XInputStream > xInput,
    Reference< container::XNameContainer > const & xDialogModel )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiServiceFactory > xSMgr( ::comphelper::getProcessServiceFactory() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XParser > xParser( xSMgr->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
    OSL_ASSERT( xParser.is() );
    if (! xParser.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create sax-parser component!") ),
            Reference< XInterface >() );
    }

    // error handler, entity resolver omitted for this helper function
    xParser->setDocumentHandler( importDialogModel( xDialogModel ) );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUString( RTL_CONSTASCII_USTRINGPARAM("virtual file") );

    xParser->parseStream( source );
}

};
