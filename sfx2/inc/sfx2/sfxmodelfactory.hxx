/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxmodelfactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:46:00 $
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

#ifndef SFX2_SFXMODELFACTORY_HXX
#define SFX2_SFXMODELFACTORY_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
/** === end UNO includes === **/

//........................................................................
namespace sfx2
{
//........................................................................

    #define SFXMODEL_EMBEDDED_OBJECT            (sal_uInt64)(0x0001)
    #define SFXMODEL_DISABLE_EMBEDDED_SCRIPTS   (sal_uInt64)(0x0002)

    typedef ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ( SAL_CALL * SfxModelFactoryFunc ) (
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory,
        const sal_uInt64 _nCreationFlags
    );

    //====================================================================
    //= createSfxModelFactory
    //====================================================================
    /** creates a XSingleServiceFactory which can be used to created instances
        of classes derived from SfxBaseModel

        In opposite to the default implementations from module cppuhelper, this
        factory evaluates certain creation arguments (passed to createInstanceWithArguments)
        and passes them to the factory function of the derived class.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
        SFX2_DLLPUBLIC createSfxModelFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
            const ::rtl::OUString& _rImplementationName,
            const SfxModelFactoryFunc _pComponentFactoryFunc,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames
        );

//........................................................................
} // namespace sfx2
//........................................................................

#endif // SFX2_SFXMODELFACTORY_HXX
