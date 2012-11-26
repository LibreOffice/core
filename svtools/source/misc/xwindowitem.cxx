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
#include "precompiled_svtools.hxx"

#include "svtools/xwindowitem.hxx"

#include <vcl/window.hxx>


using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////

XWindowItem::XWindowItem() :
    SfxPoolItem()
{
}


XWindowItem::XWindowItem( sal_uInt16 nWhichId, Window * pWin ) :
    SfxPoolItem( nWhichId )
{
    if (pWin)
    {
        m_xWin = uno::Reference< awt::XWindow >( pWin->GetComponentInterface(), uno::UNO_QUERY );
        // the assertion can't possibly fails since VCLXWindow implements XWindow...
        DBG_ASSERT( m_xWin.is(), "failed to get XWindow" );
    }
}


XWindowItem::XWindowItem( sal_uInt16 nWhichId, uno::Reference< awt::XWindow > & rxWin ) :
    SfxPoolItem( nWhichId ),
    m_xWin( rxWin )
{
}


XWindowItem::XWindowItem( const XWindowItem &rItem ) :
    SfxPoolItem( Which() ),
    m_xWin( rItem.m_xWin )
{
}


XWindowItem::~XWindowItem()
{
}


SfxPoolItem * XWindowItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new XWindowItem( *this );
}


int XWindowItem::operator == ( const SfxPoolItem & rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const XWindowItem * pItem = dynamic_cast< const XWindowItem * >(&rAttr);
    return pItem ? m_xWin == pItem->m_xWin : 0;
}


//////////////////////////////////////////////////////////////////////


