/*************************************************************************
 *
 *  $RCSfile: layerupdatehandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-30 15:28:35 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEHANDLER_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATEHANDLER_HXX

#ifndef CONFIGMGR_BACKEND_UPDATESVC_HXX
#include "updatesvc.hxx"
#endif

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEBUILDER_HXX
#include "layerupdatebuilder.hxx"
#endif

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

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
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------
        class LayerUpdateBuilder;

        class LayerUpdateHandler : public UpdateService , Noncopyable
        {
        public:
            explicit
            LayerUpdateHandler(CreationArg _xServiceFactory);

            ~LayerUpdateHandler();

            // XUpdateHandler
            virtual void SAL_CALL
                startUpdate( const OUString& aContext )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                endUpdate(  )
                    throw (backenduno::MalformedDataException, lang::IllegalAccessException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyGroup( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                modifySet( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceGroup( const OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceSet( const OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceGroupFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceSetFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode( const OUString& aName )
                    throw (backenduno::MalformedDataException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                removeGroupNode( const OUString& aName )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                removeSetNode( const OUString& aName )
                    throw (backenduno::MalformedDataException, container::NoSuchElementException, container::ElementExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyPropertyValue( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyPropertyAttributes( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask )
                    throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::PropertyExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                resetProperty( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask )
                    throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::PropertyExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplacePropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                removeProperty( const OUString& aName )
                    throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::PropertyExistException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

        private:
            LayerUpdateBuilder & getUpdateBuilder();

            void checkBuilder();

            void raiseMalformedDataException(sal_Char const * pMsg);
            void raiseNodeChangedBeforeException(sal_Char const * pMsg);
            void raisePropChangedBeforeException(sal_Char const * pMsg);
            void raisePropExistsException(sal_Char const * pMsg);

        private:
            LayerUpdateBuilder                          m_aBuilder;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




