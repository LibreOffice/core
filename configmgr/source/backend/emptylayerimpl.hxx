/*************************************************************************
 *
 *  $RCSfile: emptylayerimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:47 $
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

#ifndef CONFIGMGR_BACKEND_EMPTYLAYERIMPL_HXX
#define CONFIGMGR_BACKEND_EMPTYLAYERIMPL_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif // _CPPUHELPER_IMPLBASE1_HXX_

#include <drafts/com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <drafts/com/sun/star/configuration/backend/XLayer.hpp>

// -----------------------------------------------------------------------------
namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        // -----------------------------------------------------------------------------
        using rtl::OUString;
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace beans     = ::com::sun::star::beans;
        namespace container = ::com::sun::star::container;
        namespace backenduno    = ::drafts::com::sun::star::configuration::backend;

        // -----------------------------------------------------------------------------

        class EmptyLayer : public cppu::WeakImplHelper1<backenduno::XLayer>
        {
        public:
            virtual ~EmptyLayer();

            // XLayer
        public:
            virtual void SAL_CALL
                readData( const uno::Reference< backenduno::XLayerHandler >& aHandler )
                throw (uno::RuntimeException);
        };
        // -----------------------------------------------------------------------------

        class RequireEmptyLayer : public cppu::WeakImplHelper1<backenduno::XLayerHandler>
        {
        public:
            RequireEmptyLayer();
            virtual ~RequireEmptyLayer();

            bool wasEmpty()     const { return m_bEmpty; }
            bool wasInvalid()   const { return m_bInvalid; }
            // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

        private:
            void failNotEmpty() { fail("layer is not empty"); }
            void fail(sal_Char const * pMsg);

        private:
            bool m_bStarted;
            bool m_bInvalid;
            bool m_bEmpty;
        };
        // -----------------------------------------------------------------------------
    } // namespace xml
    // -----------------------------------------------------------------------------

} // namespace configmgr
#endif
