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
#include "precompiled_toolkit.hxx"
#include "toolkit/awt/xsimpleanimation.hxx"
#include "toolkit/helper/property.hxx"
#include <tools/debug.hxx>
#include <vcl/throbber.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= XSimpleAnimation
    //====================================================================
    DBG_NAME( XSimpleAnimation )

    //--------------------------------------------------------------------
    XSimpleAnimation::XSimpleAnimation()
    {
        DBG_CTOR( XSimpleAnimation, NULL );
    }

    //--------------------------------------------------------------------
    XSimpleAnimation::~XSimpleAnimation()
    {
        DBG_DTOR( XSimpleAnimation, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::start() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::stop() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& rImageList )
        throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->setImageList( rImageList );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSimpleAnimation::setProperty( const ::rtl::OUString& PropertyName, const uno::Any& Value )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
        {
            VCLXWindow::setProperty( PropertyName, Value );
            return;
        }

        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            case BASEPROPERTY_STEP_TIME: {
                sal_Int32   nStepTime( 0 );
                if ( Value >>= nStepTime )
                    pThrobber->setStepTime( nStepTime );

                break;
            }
            case BASEPROPERTY_REPEAT: {
                sal_Bool bRepeat( sal_True );
                if ( Value >>= bRepeat )
                    pThrobber->setRepeat( bRepeat );
                break;
            }
            default:
                VCLXWindow::setProperty( PropertyName, Value );
        }
    }

    //--------------------------------------------------------------------
    uno::Any SAL_CALL XSimpleAnimation::getProperty( const ::rtl::OUString& PropertyName )
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
            return VCLXWindow::getProperty( PropertyName );

        uno::Any aReturn;
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
        case BASEPROPERTY_STEP_TIME:
            aReturn <<= pThrobber->getStepTime();
            break;
        case BASEPROPERTY_REPEAT:
            aReturn <<= pThrobber->getRepeat();
            break;
        default:
            aReturn = VCLXWindow::getProperty( PropertyName );
        }
        return aReturn;
    }

//........................................................................
}   // namespace toolkit
//........................................................................
