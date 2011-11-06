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



#include "vclxfixedline.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

DBG_NAME( VCLXFixedLine )

VCLXFixedLine::VCLXFixedLine()
  : VCLXWindow()
{
    DBG_CTOR( VCLXFixedLine, NULL );
}

VCLXFixedLine::~VCLXFixedLine()
{
    DBG_DTOR( VCLXFixedLine, NULL );
}

IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXFixedLine, VCLXWindow );

void SAL_CALL VCLXFixedLine::dispose() throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
    }

    VCLXWindow::dispose();
}

::com::sun::star::awt::Size SAL_CALL VCLXFixedLine::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    return awt::Size( 8, 8 );
}

void VCLXFixedLine::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    ::vos::OClearableGuard aGuard( GetMutex() );
/*
    switch ( _rVclWindowEvent.GetId() )
    {
        default:
            aGuard.clear();
*/
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
/*
            break;
    }
*/
}

void SAL_CALL VCLXFixedLine::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                VCLXWindow::setProperty( PropertyName, Value );
//        }
    }
}

Any SAL_CALL VCLXFixedLine::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Any aReturn;
    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                aReturn = VCLXWindow::getProperty( PropertyName );
  //      }
    }
    return aReturn;
}

} // namespace layoutimpl
