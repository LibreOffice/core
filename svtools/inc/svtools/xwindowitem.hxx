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


#ifndef _XWINDOWITEM_HXX_
#define _XWINDOWITEM_HXX_


#include "svtools/svtdllapi.h"

#include <svl/poolitem.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/XWindow.hpp>

class Window;

//////////////////////////////////////////////////////////////////////

class SVT_DLLPUBLIC XWindowItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xWin;

    // disallow use assignment operator
    XWindowItem & operator = ( const XWindowItem & );

public:
    XWindowItem();
    explicit XWindowItem( sal_uInt16 nWhich, Window * pWin );
    XWindowItem( sal_uInt16 nWhich, com::sun::star::uno::Reference< com::sun::star::awt::XWindow > & rxWin );
    XWindowItem( const XWindowItem &rItem );
    ~XWindowItem();

    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual int operator == ( const SfxPoolItem& rAttr ) const;

    Window *        GetWindowPtr() const    { return VCLUnoHelper::GetWindow( m_xWin ); }
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >  GetXWindow() const  { return m_xWin; }
};

//////////////////////////////////////////////////////////////////////

#endif

