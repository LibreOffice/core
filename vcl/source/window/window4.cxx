/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_vcl.hxx"

#include "vcl/window.hxx"
#include "vcl/window.h"
#include "vcl/arrange.hxx"

#include <map>

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
        rtl::OUString                            maName;
        std::map< rtl::OUString, Window* >       maNameToWindow;
        std::map< Window*, rtl::OUString >       maWindowToName;

        void insertName( const rtl::OUString& i_rName, Window* i_pWin );
        void removeWindow( Window* );
        Window* findName( const rtl::OUString& ) const;
    };

    void ExtWindowImpl::insertName( const rtl::OUString& i_rName, Window* i_pWin )
    {
        OSL_ENSURE( maNameToWindow.find( i_rName ) == maNameToWindow.end(), "duplicate named window inserted" );
        maNameToWindow[ i_rName ] = i_pWin;
        maWindowToName[ i_pWin ] = i_rName;
    }

    void ExtWindowImpl::removeWindow( Window* i_pWin )
    {
        std::map< Window*, rtl::OUString >::iterator it = maWindowToName.find( i_pWin );
        if( it != maWindowToName.end() )
        {
            maNameToWindow.erase( it->second );
            maWindowToName.erase( it );
        }
    }

    Window* ExtWindowImpl::findName( const rtl::OUString& i_rName ) const
    {
        std::map< rtl::OUString, Window* >::const_iterator it = maNameToWindow.find( i_rName );
        return it != maNameToWindow.end() ? it->second : NULL;
    }
}

static rtl::OUString getLastNameToken( const rtl::OUString& i_rName )
{
    sal_Int32 nIndex = i_rName.lastIndexOf( sal_Unicode('/') );
    if( nIndex != -1 )
    {
        // if this is not an empty token, that is the name
        if( nIndex < i_rName.getLength()-1 )
            return i_rName.copy( nIndex+1 );
        // we need to search backward
        const sal_Unicode* pStr = i_rName.getStr();
        while( nIndex >= 0 && pStr[nIndex] == '/' )
            nIndex--;
        if( nIndex < 0 ) // give up
            return rtl::OUString();
        // search backward to next '/' or beginning
        sal_Int32 nBeginIndex = nIndex-1;
        while( nBeginIndex >= 0 && pStr[nBeginIndex] != '/' )
            nBeginIndex--;
        return i_rName.copy( nBeginIndex+1, nIndex-nBeginIndex );
    }
    return rtl::OUString( i_rName );
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

void Window::addWindow( Window* i_pWin, const rtl::OUString& i_rName, bool i_bTakeOwnership )
{
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl && i_pWin )
    {
        vcl::ExtWindowImpl* pChildImpl = i_pWin->ImplGetExtWindowImpl();
        if( pChildImpl )
        {
            i_pWin->SetParent( this );
            pChildImpl->mbOwnedByParent = i_bTakeOwnership;
            rtl::OUString aName( getLastNameToken( i_rName ) );
            if( aName.getLength() )
            {
                pImpl->insertName( aName, i_pWin );
            }
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
               pImpl->removeWindow( i_pWin );
               i_pWin->SetParent( i_pNewParent );
               pRet = i_pWin;
            }
        }
    }
    return pRet;
}

Window* Window::removeWindow( const rtl::OUString& i_rName, Window* i_pNewParent )
{
    Window* pRet = NULL;
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl )
    {
        rtl::OUString aName( getLastNameToken( i_rName ) );
        pRet = pImpl->findName( aName );
        pRet = removeWindow( pRet, i_pNewParent );
    }
    return pRet;
}

Window* Window::findWindow( const rtl::OUString& i_rName ) const
{
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    Window* pSearch = const_cast<Window*>(this);
    if( pImpl )
    {
        sal_Int32 nIndex = 0;
        while( nIndex && pSearch )
        {
            rtl::OUString aName( i_rName.getToken( 0, '/', nIndex ) );
            if( aName.getLength() )
            {
                pSearch = pImpl->findName( aName );
                if( pSearch )
                {
                    pImpl = pSearch->ImplGetExtWindowImpl();
                    if( ! pImpl )
                        pSearch = NULL;
                }
            }
        }
    }
    else
        pSearch = NULL;
    return pSearch != this ? pSearch : NULL;
}

