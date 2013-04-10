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



#include "precompiled_vcl.hxx"

#include "vcl/window.hxx"
#include "vcl/arrange.hxx"

#include "window.h"
#include "svdata.hxx"
#include "brdwin.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"

#include <map>
#include <vector>

using namespace com::sun::star;

namespace vcl
{
    struct ExtWindowImpl
    {
        ExtWindowImpl()
        : mbOwnedByParent( false )
        {}
        ~ExtWindowImpl()
        {}

        boost::shared_ptr< WindowArranger >      mxLayout;
        bool                                     mbOwnedByParent;
        rtl::OUString                            maIdentifier;
    };
}

void Window::ImplDeleteOwnedChildren()
{
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        Window* pDeleteCandidate = pChild;
        pChild = pChild->mpWindowImpl->mpNext;
        vcl::ExtWindowImpl* pDelImpl = pDeleteCandidate->ImplGetExtWindowImpl();
        if( pDelImpl && pDelImpl->mbOwnedByParent )
            delete pDeleteCandidate;
    }
}

void Window::ImplFreeExtWindowImpl()
{
    ImplDeleteOwnedChildren();
    if( mpWindowImpl )
    {
        delete mpWindowImpl->mpExtImpl;
        mpWindowImpl->mpExtImpl = NULL;
    }
}

vcl::ExtWindowImpl* Window::ImplGetExtWindowImpl() const
{
    vcl::ExtWindowImpl* pImpl = NULL;
    if( mpWindowImpl )
    {
        if( ! mpWindowImpl->mpExtImpl && ! mpWindowImpl->mbInDtor )
            mpWindowImpl->mpExtImpl = new vcl::ExtWindowImpl();
        pImpl = mpWindowImpl->mpExtImpl;
    }
    return pImpl;
}

void Window::ImplExtResize()
{
    if( mpWindowImpl && mpWindowImpl->mpExtImpl )
    {
        if( mpWindowImpl->mpExtImpl->mxLayout.get() )
            mpWindowImpl->mpExtImpl->mxLayout->setManagedArea( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
    }
}

boost::shared_ptr< vcl::WindowArranger > Window::getLayout()
{
    boost::shared_ptr< vcl::WindowArranger > xRet;
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl )
    {
        if( ! pImpl->mxLayout.get() )
        {
            pImpl->mxLayout.reset( new vcl::LabelColumn() );
            pImpl->mxLayout->setParentWindow( this );
            pImpl->mxLayout->setOuterBorder( -1 );
        }
        xRet = pImpl->mxLayout;
    }

    return xRet;
}

void Window::addWindow( Window* i_pWin, bool i_bTakeOwnership )
{
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl && i_pWin )
    {
        vcl::ExtWindowImpl* pChildImpl = i_pWin->ImplGetExtWindowImpl();
        if( pChildImpl )
        {
            i_pWin->SetParent( this );
            pChildImpl->mbOwnedByParent = i_bTakeOwnership;
        }
    }
}

Window* Window::removeWindow( Window* i_pWin, Window* i_pNewParent )
{
    Window* pRet = NULL;
    if( i_pWin )
    {
        vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
        if( pImpl )
        {
            vcl::ExtWindowImpl* pChildImpl = i_pWin->ImplGetExtWindowImpl();
            if( pChildImpl )
            {
                if( ! i_pNewParent )
                   pChildImpl->mbOwnedByParent = false;
               i_pWin->SetParent( i_pNewParent );
               pRet = i_pWin;
            }
        }
    }
    return pRet;
}

Window* Window::findWindow( const rtl::OUString& i_rIdentifier ) const
{
    if( getIdentifier() == i_rIdentifier )
        return const_cast<Window*>(this);

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        Window* pResult = pChild->findWindow( i_rIdentifier );
        if( pResult )
            return pResult;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return NULL;
}

const rtl::OUString& Window::getIdentifier() const
{
    static rtl::OUString aEmptyStr;

    return (mpWindowImpl && mpWindowImpl->mpExtImpl) ? mpWindowImpl->mpExtImpl->maIdentifier : aEmptyStr;
}

void Window::setIdentifier( const rtl::OUString& i_rIdentifier )
{
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl )
        pImpl->maIdentifier = i_rIdentifier;
}

void Window::setProperties( const uno::Sequence< beans::PropertyValue >& i_rProps )
{
    const beans::PropertyValue* pVals = i_rProps.getConstArray();
    for( sal_Int32 i = 0; i < i_rProps.getLength(); i++ )
    {
        if( pVals[i].Name.equalsAscii( "Enabled" ) )
        {
            sal_Bool bVal = sal_True;
            if( pVals[i].Value >>= bVal )
                Enable( bVal );
        }
        else if( pVals[i].Name.equalsAscii( "Visible" ) )
        {
            sal_Bool bVal = sal_True;
            if( pVals[i].Value >>= bVal )
                Show( bVal );
        }
        else if( pVals[i].Name.equalsAscii( "Text" ) )
        {
            rtl::OUString aText;
            if( pVals[i].Value >>= aText )
                SetText( aText );
        }
    }
}

uno::Sequence< beans::PropertyValue > Window::getProperties() const
{
    uno::Sequence< beans::PropertyValue > aProps( 3 );
    aProps[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) );
    aProps[0].Value = uno::makeAny( sal_Bool( IsEnabled() ) );
    aProps[1].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) );
    aProps[1].Value = uno::makeAny( sal_Bool( IsVisible() ) );
    aProps[2].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) );
    aProps[2].Value = uno::makeAny( rtl::OUString( GetText() ) );

    return aProps;
}




void Window::EnableThemeSupport (void)
{
    mpWindowImpl->mbIsThemingEnabled = sal_True;
}




void Window::DisableThemeSupport (void)
{
    mpWindowImpl->mbIsThemingEnabled = sal_False;
}




ImplBorderWindow* Window::CreateBorderWindow (
    Window* pParent,
    const WinBits nStyle,
    const sal_uInt16 nTypeStyle)
{
    return new ImplBorderWindow(pParent, nStyle, nTypeStyle);
}
