/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"

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
