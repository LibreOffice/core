/*************************************************************************
 *
 *  $RCSfile: component.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:09 $
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
#ifndef _CPPUHELPER_COMPONENT_HXX_
#define _CPPUHELPER_COMPONENT_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>


/** */ //for docpp
namespace cppu
{

/**
 * The helper implementation for a basic broadcaster. This implementation
 * supports aggregation and weak references.
 *
 * @author  Markus Meyer
 * @since   98/04/12
 */
class OComponentHelper : public ::cppu::WeakAggImplHelper1< ::com::sun::star::lang::XComponent >
{
public:
    /**
     * Create an object that implements XComponent.
     *
     * @param rMutex    the mutex used to protect multi thread access.
     *                  The lifetime must be longer than the lifetime
     *                  of this object.
     */
    OComponentHelper( ::osl::Mutex & rMutex );
    /**
     * If dispose is not previous called, first acquire is called to protect against
     * double delete and than call dispose.<BR> Note in this situation no destructor
     * of derived classes are called.
     */
    ~OComponentHelper();

    // XInterface
    virtual void SAL_CALL release() throw();

    // XTypeProvider getImplementationId() has to be implemented separately!
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException) = 0;

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /**
     * Called in the dispose method after the listeners are notified.
     * In this situation rBHelper.bDisposed is false
     * and rBHelper.bDisposing is true.
     */
    virtual void SAL_CALL disposing();

    /**
     * Contains a mutex, a listener container and the dispose states.
     * Subclasses should only modify the listener container.
     */
    OBroadcastHelper    rBHelper;
private:
                        OComponentHelper( const OComponentHelper & );
    OComponentHelper &  operator = ( const OComponentHelper & );
};

}

#endif

