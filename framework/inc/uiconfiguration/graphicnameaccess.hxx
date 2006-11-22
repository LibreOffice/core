/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphicnameaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:41:06 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HXX_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
