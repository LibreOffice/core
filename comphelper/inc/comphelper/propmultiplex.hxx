/*************************************************************************
 *
 *  $RCSfile: propmultiplex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:23:21 $
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
#define _COMPHELPER_PROPERTY_MULTIPLEX_HXX_

#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#include <comphelper/propstate.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//=========================================================================
//= property helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

//==================================================================
//= OPropertyChangeListener
//==================================================================
/// simple listener adapter for property sets
class OPropertyChangeListener
{
    friend class OPropertyChangeMultiplexer;

    OPropertyChangeMultiplexer* m_pAdapter;
    ::osl::Mutex&               m_rMutex;

public:
    OPropertyChangeListener(::osl::Mutex& _rMutex)
        :m_rMutex(_rMutex) ,m_pAdapter(NULL) { }
    virtual ~OPropertyChangeListener();

    virtual void _propertyChanged(const  ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException) = 0;

protected:
    void setAdapter(OPropertyChangeMultiplexer* _pAdapter);
};

//==================================================================
//= OPropertyChangeMultiplexer
//==================================================================
/// multiplexer for property changes
class OPropertyChangeMultiplexer    :public cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>
{
    friend class OPropertyChangeListener;
     ::com::sun::star::uno::Sequence< ::rtl::OUString >     m_aProperties;
     ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>   m_xSet;
    OPropertyChangeListener*                    m_pListener;


    virtual ~OPropertyChangeMultiplexer();
public:
    OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxSet);

// XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException);

// XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);


    void addProperty(const ::rtl::OUString& aPropertyName);
    void dispose();
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_MULTIPLEX_HXX_

