/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graphicnameaccess.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_

#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/container/XNameContainer.hpp>
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HXX_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#include <cppuhelper/implbase1.hxx>

namespace framework
{
    class GraphicNameAccess : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
    {
        public:
            GraphicNameAccess();
            virtual ~GraphicNameAccess();

            void addElement( const rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rElement );
            sal_uInt32 size() const;

            // XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::uno::RuntimeException);

            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements()
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                throw(::com::sun::star::uno::RuntimeException);

        private:
            typedef BaseHash< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > NameGraphicHashMap;
            NameGraphicHashMap m_aNameToElementMap;
            ::com::sun::star::uno::Sequence< rtl::OUString > m_aSeq;
    };
}

#endif // __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
