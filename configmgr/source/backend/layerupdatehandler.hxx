/*************************************************************************
 *
 *  $RCSfile: layerupdatehandler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:17:13 $
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
        namespace backenduno = ::com::sun::star::configuration::backend;

        using backenduno::TemplateIdentifier;
        using backenduno::MalformedDataException;
// -----------------------------------------------------------------------------
        class LayerUpdateBuilder;

        class LayerUpdateHandler : public UpdateService , Noncopyable
        {
        public:
            explicit
            LayerUpdateHandler(CreationArg _xContext);

            ~LayerUpdateHandler();

            // XUpdateHandler
            virtual void SAL_CALL
                startUpdate(  )
                    throw ( MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endUpdate(  )
                    throw ( MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyNode( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, sal_Int16 aAttributes, const TemplateIdentifier& aTemplate )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeNode( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyProperty( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValue( )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValueForLocale( const OUString& aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetProperty( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplacePropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeProperty( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            LayerUpdateBuilder & getUpdateBuilder();

            void checkBuilder(bool _bForProperty = false);

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




