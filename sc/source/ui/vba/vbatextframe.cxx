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


#include <vbahelper/helperdecl.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/text/XText.hpp>
#include "vbatextframe.hxx"
#include "vbacharacters.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaTextFrame::ScVbaTextFrame( uno::Sequence< uno::Any> const & args, uno::Reference< uno::XComponentContext> const & xContext ) throw ( lang::IllegalArgumentException ) :  ScVbaTextFrame_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext, getXSomethingFromArgs< drawing::XShape >( args, 1, false ) )
{
}

// Methods
uno::Any SAL_CALL
ScVbaTextFrame::Characters() throw (uno::RuntimeException)
{
    uno::Reference< text::XSimpleText > xSimpleText( m_xShape, uno::UNO_QUERY_THROW );
    ScVbaPalette aPalette( SfxObjectShell::Current() );
    uno::Any aStart( sal_Int32( 1 ) );
    uno::Any aLength(sal_Int32( -1 ) );
    return uno::makeAny( uno::Reference< ov::excel::XCharacters >( new ScVbaCharacters( this, mxContext, aPalette, xSimpleText, aStart, aLength, sal_True ) ) );
}

rtl::OUString&
ScVbaTextFrame::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaTextFrame") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaTextFrame::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.TextFrame" ) );
    }
    return aServiceNames;
}

namespace textframe
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaTextFrame, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaTextFrame",
    "ooo.vba.excel.TextFrame" );
}
