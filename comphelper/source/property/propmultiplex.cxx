/*************************************************************************
 *
 *  $RCSfile: propmultiplex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:25:29 $
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

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

//========================================================================
//= OPropertyChangeListener
//========================================================================
//------------------------------------------------------------------------
OPropertyChangeListener::~OPropertyChangeListener()
{
    if (m_pAdapter)
        m_pAdapter->dispose();
}

//------------------------------------------------------------------
void OPropertyChangeListener::setAdapter(OPropertyChangeMultiplexer* pAdapter)
{
    if (m_pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter->release();
        m_pAdapter = NULL;
    }

    if (pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter = pAdapter;
        m_pAdapter->acquire();
    }
}

//========================================================================
//= OPropertyChangeMultiplexer
//========================================================================
//------------------------------------------------------------------
OPropertyChangeMultiplexer::OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxSet)
            :m_xSet(_rxSet)
            ,m_pListener(_pListener)
{
    m_pListener->setAdapter(this);
}

//------------------------------------------------------------------
OPropertyChangeMultiplexer::~OPropertyChangeMultiplexer()
{
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::dispose()
{
    if (m_xSet.is())
    {
         ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener> xPreventDelete(this);

        const ::rtl::OUString* pProperties = m_aProperties.getConstArray();
        for (sal_Int32 i = 0; i < m_aProperties.getLength(); ++i, ++pProperties)
            m_xSet->removePropertyChangeListener(*pProperties, static_cast< ::com::sun::star::beans::XPropertyChangeListener*>(this));

        m_pListener->setAdapter(NULL);

        m_xSet = NULL;
        m_pListener = NULL;
    }
}

// XEventListener
//------------------------------------------------------------------
void SAL_CALL OPropertyChangeMultiplexer::disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException)
{
    if (m_pListener)
        m_pListener->setAdapter(NULL);

    m_xSet = NULL;
    m_pListener = NULL;
}

// XPropertyChangeListener
//------------------------------------------------------------------
void SAL_CALL OPropertyChangeMultiplexer::propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw( ::com::sun::star::uno::RuntimeException)
{
    if (m_pListener)
        m_pListener->_propertyChanged(_rEvent);
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::addProperty(const ::rtl::OUString& _sPropertyName)
{
    if (m_xSet.is())
    {
        m_xSet->addPropertyChangeListener(_sPropertyName, static_cast< ::com::sun::star::beans::XPropertyChangeListener*>(this));
        m_aProperties.realloc(m_aProperties.getLength() + 1);
        m_aProperties.getArray()[m_aProperties.getLength()-1] = _sPropertyName;
    }
}

//.........................................................................
}
//.........................................................................

