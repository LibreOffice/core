/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filprp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:47:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif
#ifndef _FILPRP_HXX_
#include "filprp.hxx"
#endif

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;


#ifndef _FILINL_HXX_
#include "filinl.hxx"
#endif


XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider ),
      m_count( 0 ),
      m_seq( 0 )
{
    m_pMyShell->m_pProvider->acquire();

    shell::ContentMap::iterator it = m_pMyShell->m_aContent.find( aUnqPath );

    shell::PropertySet& properties = *(it->second.properties);
    shell::PropertySet::iterator it1 = properties.begin();

    m_seq.realloc( properties.size() );

    while( it1 != properties.end() )
    {
        m_seq[ m_count++ ] = beans::Property( it1->getPropertyName(),
                                              it1->getHandle(),
                                              it1->getType(),
                                              it1->getAttributes() );
        ++it1;
    }
}


XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const Sequence< beans::Property >& seq )
    : m_pMyShell( pMyShell ),
      m_count( seq.getLength() ),
      m_seq( seq )
{
    m_pMyShell->m_pProvider->acquire();
}


XPropertySetInfo_impl::~XPropertySetInfo_impl()
{
    m_pMyShell->m_pProvider->release();
}


void SAL_CALL
XPropertySetInfo_impl::acquire(
                  void )
  throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfo_impl::release(
              void )
  throw()
{
  OWeakObject::release();
}



XTYPEPROVIDER_IMPL_2( XPropertySetInfo_impl,
                         lang::XTypeProvider,
                      beans::XPropertySetInfo )


Any SAL_CALL
XPropertySetInfo_impl::queryInterface(
                     const Type& rType )
  throw( RuntimeException )
{
  Any aRet = cppu::queryInterface( rType,
                    SAL_STATIC_CAST( lang::XTypeProvider*,this),
                    SAL_STATIC_CAST( beans::XPropertySetInfo*,this) );
  return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


beans::Property SAL_CALL
XPropertySetInfo_impl::getPropertyByName(
                     const rtl::OUString& aName )
  throw( beans::UnknownPropertyException,
     RuntimeException)
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return m_seq[i];

  throw beans::UnknownPropertyException();
}



Sequence< beans::Property > SAL_CALL
XPropertySetInfo_impl::getProperties(
                    void )
  throw( RuntimeException )
{
  return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfo_impl::hasPropertyByName(
                     const rtl::OUString& aName )
  throw( RuntimeException )
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return true;
  return false;
}
