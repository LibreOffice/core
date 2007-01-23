/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lazydelete.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:07:54 $
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

#ifndef LAZYDELETE_CXX
#define LAZYDELETE_CXX

#include <window.hxx>
#include <menu.hxx>
#include <lazydelete.hxx>

namespace vcl {

LazyDeletorBase::LazyDeletorBase()
{
}

LazyDeletorBase::~LazyDeletorBase()
{
}

// instantiate instance pointers for LazyDeletor<Window,Menu>
template<> LazyDeletor<Window>* LazyDeletor<Window>::s_pOneInstance = NULL;
template<> LazyDeletor<Menu>* LazyDeletor<Menu>::s_pOneInstance = NULL;

// a list for all LazyeDeletor<T> singletons
static std::vector< LazyDeletorBase* > lcl_aDeletors;

void LazyDelete::addDeletor( LazyDeletorBase* i_pDel )
{
    lcl_aDeletors.push_back( i_pDel );
}

void LazyDelete::flush()
{
    unsigned int nCount = lcl_aDeletors.size();
    for( unsigned int i = 0; i < nCount; i++ )
        delete lcl_aDeletors[i];
    lcl_aDeletors.clear();
}

// specialized is_less function for Window
template<> bool LazyDeletor<Window>::is_less( Window* left, Window* right )
{
    return (left != right && right->IsChild( left, TRUE )) ? true : false;
}

// specialized is_less function for Menu
template<> bool LazyDeletor<Menu>::is_less( Menu* left, Menu* right )
{
    while( left && left != right )
        left = left->ImplGetStartedFrom();
    return left != NULL;
}

}

#endif

