/*************************************************************************
 *
 *  $RCSfile: filprp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sb $ $Date: 2000-10-24 13:15:23 $
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


XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const uno::Sequence< beans::Property >& seq )
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
  throw( uno::RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfo_impl::release(
              void )
  throw( uno::RuntimeException )
{
  OWeakObject::release();
}


uno::Any SAL_CALL
XPropertySetInfo_impl::queryInterface(
                     const uno::Type& rType )
  throw( uno::RuntimeException )
{
  uno::Any aRet = cppu::queryInterface( rType,
                    SAL_STATIC_CAST( beans::XPropertySetInfo*,this) );
  return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


beans::Property SAL_CALL
XPropertySetInfo_impl::getPropertyByName(
                     const rtl::OUString& aName )
  throw( beans::UnknownPropertyException,
     uno::RuntimeException)
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return m_seq[i];

  throw beans::UnknownPropertyException();
}



uno::Sequence< beans::Property > SAL_CALL
XPropertySetInfo_impl::getProperties(
                    void )
  throw( uno::RuntimeException )
{
  return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfo_impl::hasPropertyByName(
                     const rtl::OUString& aName )
  throw( uno::RuntimeException )
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return true;
  return false;
}
