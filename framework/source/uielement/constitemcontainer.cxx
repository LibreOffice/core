/*************************************************************************
 *
 *  $RCSfile: constitemcontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:50:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_CONSTITEMCONTAINER_HXX_
#include <uielement/constitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_ROOTITEMCONTAINER_HXX_
#include <uielement/rootitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_ITEMCONTAINER_HXX_
#include <uielement/itemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

const char WRONG_TYPE_EXCEPTION[] = "Type must be com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >";

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_4     (   ConstItemContainer                                              ,
                            OWeakObject                                                     ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider         ),
                            DIRECT_INTERFACE( ::com::sun::star::container::XElementAccess   ),
                            DIRECT_INTERFACE( ::com::sun::star::container::XIndexAccess     ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XUnoTunnel            )
                        )

DEFINE_XTYPEPROVIDER_4  (   ConstItemContainer                          ,
                            ::com::sun::star::lang::XTypeProvider       ,
                            ::com::sun::star::container::XIndexAccess   ,
                            ::com::sun::star::container::XElementAccess ,
                            ::com::sun::star::lang::XUnoTunnel
                        )

ConstItemContainer::ConstItemContainer() : ::cppu::OWeakObject()
{
}

ConstItemContainer::ConstItemContainer( const RootItemContainer& rRootItemContainer, sal_Bool bFastCopy )
{
    ShareGuard( rRootItemContainer.m_aShareMutex );

    // If bFastCopy is set the onwer of the root item container will transfer ownership to us. So
    // it is possible to copy only the root part.
    if ( bFastCopy )
        m_aItemVector = rRootItemContainer.m_aItemVector;
    else
        copyItemContainer( rRootItemContainer.m_aItemVector );
}

ConstItemContainer::ConstItemContainer( const ItemContainer& rItemContainer )
{
    ShareGuard( rItemContainer.m_aShareMutex );
    copyItemContainer( rItemContainer.m_aItemVector );
}

ConstItemContainer::ConstItemContainer( const Reference< XIndexAccess >& rSourceContainer, sal_Bool bFastCopy )
{
    try
    {
        sal_Int32 nCount = rSourceContainer->getCount();
        if ( bFastCopy )
        {
            for ( sal_Int32 i = 0; i < nCount; i++ )
            {
                Sequence< PropertyValue > aPropSeq;
                Any a = rSourceContainer->getByIndex( i );
                if ( a >>= aPropSeq )
                    m_aItemVector.push_back( aPropSeq );
            }
        }
        else
        {
            for ( sal_Int32 i = 0; i < nCount; i++ )
            {
                Sequence< PropertyValue > aPropSeq;
                Any a = rSourceContainer->getByIndex( i );
                if ( a >>= aPropSeq )
                {
                    sal_Int32 nContainerIndex = -1;
                    Reference< XIndexAccess > xIndexAccess;
                    for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
                    {
                        if ( aPropSeq[j].Name.equalsAscii( "ItemDescriptorContainer" ))
                        {
                            aPropSeq[j].Value >>= xIndexAccess;
                            nContainerIndex = j;
                            break;
                        }
                    }

                    if ( xIndexAccess.is() && nContainerIndex >= 0 )
                        aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

                    m_aItemVector.push_back( aPropSeq );
                }
            }
        }
    }
    catch ( IndexOutOfBoundsException& )
    {
    }
}

ConstItemContainer::~ConstItemContainer()
{
}

// private
void ConstItemContainer::copyItemContainer( const std::vector< Sequence< PropertyValue > >& rSourceVector )
{
    for ( sal_uInt32 i = 0; i < rSourceVector.size(); i++ )
    {
        sal_Int32 nContainerIndex = -1;
        Sequence< PropertyValue > aPropSeq( rSourceVector[i] );
        Reference< XIndexAccess > xIndexAccess;
        for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
        {
            if ( aPropSeq[j].Name.equalsAscii( "ItemDescriptorContainer" ))
            {
                aPropSeq[j].Value >>= xIndexAccess;
                nContainerIndex = j;
                break;
            }
        }

        if ( xIndexAccess.is() && nContainerIndex >= 0 )
            aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

        m_aItemVector.push_back( aPropSeq );
    }
}

Reference< XIndexAccess > ConstItemContainer::deepCopyContainer( const Reference< XIndexAccess >& rSubContainer )
{
    Reference< XIndexAccess > xReturn;
    if ( rSubContainer.is() )
    {
        ItemContainer*      pSource = ItemContainer::GetImplementation( rSubContainer );
        ConstItemContainer* pSubContainer( 0 );
        if ( pSource )
            pSubContainer = new ConstItemContainer( *pSource );
        else
            pSubContainer = new ConstItemContainer( rSubContainer );
        xReturn = Reference< XIndexAccess >( static_cast< OWeakObject* >( pSubContainer ), UNO_QUERY );
    }

    return xReturn;
}

// XUnoTunnel
sal_Int64 ConstItemContainer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( ConstItemContainer::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

const Sequence< sal_Int8 >& ConstItemContainer::GetUnoTunnelId() throw()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

ConstItemContainer* ConstItemContainer::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY );
    return xUT.is() ? (ConstItemContainer*)xUT->getSomething( ConstItemContainer::GetUnoTunnelId() ) : NULL;
}

// XElementAccess
sal_Bool SAL_CALL ConstItemContainer::hasElements()
throw ( RuntimeException )
{
    return ( m_aItemVector.size() != 0 );
}

// XIndexAccess
sal_Int32 SAL_CALL ConstItemContainer::getCount()
throw ( RuntimeException )
{
    return m_aItemVector.size();
}

Any SAL_CALL ConstItemContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if ( sal_Int32( m_aItemVector.size()) > Index )
        return makeAny( m_aItemVector[Index] );
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

} // namespace framework
