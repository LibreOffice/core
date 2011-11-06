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


#include "vbabookmark.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaBookmark::SwVbaBookmark( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const css::uno::Reference< frame::XModel >& rModel, const rtl::OUString& rName ) throw ( css::uno::RuntimeException ) :
    SwVbaBookmark_BASE( rParent, rContext ), mxModel( rModel ), maName( rName ), mbValid( sal_True )
{
    uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( mxModel, uno::UNO_QUERY_THROW );
    mxBookmark.set( xBookmarksSupplier->getBookmarks()->getByName( maName ), uno::UNO_QUERY_THROW );
}

SwVbaBookmark::~SwVbaBookmark()
{
}

void SwVbaBookmark::checkVality() throw ( uno::RuntimeException )
{
    if( !mbValid )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("The bookmark is not valid" ) ), uno::Reference< uno::XInterface >() );
}

void SAL_CALL SwVbaBookmark::Delete() throw ( uno::RuntimeException )
{
    checkVality();
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    xTextDocument->getText()->removeTextContent( mxBookmark );
    mbValid = sal_False;
}

void SAL_CALL SwVbaBookmark::Select() throw ( uno::RuntimeException )
{
    checkVality();
    uno::Reference< text::XTextViewCursorSupplier > xViewCursorSupplier( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xViewCursorSupplier->getViewCursor()->gotoRange( mxBookmark->getAnchor(),sal_False );
}

rtl::OUString SAL_CALL SwVbaBookmark::getName() throw ( uno::RuntimeException )
{
    return maName;
}

void SAL_CALL SwVbaBookmark::setName( const rtl::OUString& _name ) throw ( uno::RuntimeException )
{
    uno::Reference< container::XNamed > xNamed( mxBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( _name );
}

uno::Any SAL_CALL SwVbaBookmark::Range() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextContent > xTextContent( mxBookmark, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XRange>(  new SwVbaRange( this, mxContext, xTextDocument, xTextRange->getStart(), xTextRange->getEnd(), xTextRange->getText() ) ) );
}

rtl::OUString&
SwVbaBookmark::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaBookmark") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaBookmark::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Bookmark" ) );
    }
    return aServiceNames;
}

