/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerwriter.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:36:30 $
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

#ifndef CONFIGMGR_XML_LAYERWRITER_HXX
#define CONFIGMGR_XML_LAYERWRITER_HXX

#ifndef CONFIGMGR_XML_WRITERSVC_HXX
#include "writersvc.hxx"
#endif

#ifndef CONFIGMGR_XML_ELEMENTFORMATTER_HXX
#include "elementformatter.hxx"
#endif

#ifndef CONFIGMGR_STACK_HXX_
#include "stack.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERHANDLER_HPP_
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#endif

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace xml
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

        namespace sax           = ::com::sun::star::xml::sax;
        namespace backenduno    = ::com::sun::star::configuration::backend;

        // -----------------------------------------------------------------------------


        class LayerWriter : public LayerWriterService_Base
        {
        public:
            explicit
                LayerWriter(CreationArg _xContext);
            virtual ~LayerWriter();

            // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                           uno::RuntimeException);

        private:
            bool isInElement() const;
            void checkInElement(bool bInElement, bool bInProperty = false);

            void startNode();
            void startProp(uno::Type const & _aType, bool bNeedType);

            void endElement();

            void writeValue(uno::Any const & _aValue);
            void writeValue(uno::Any const & _aValue, OUString const & _aLocale);

            void outputValue(uno::Any const & _aValue);

            void raiseMalformedDataException(sal_Char const * pMsg);
            void raiseIllegalTypeException(sal_Char const * pMsg);

            void prepareAddOrReplaceElement(
                rtl::OUString const & name, sal_Int16 attributes);

        private:
            typedef Stack< OUString > TagStack;
            uno::Reference< com::sun::star::script::XTypeConverter > m_xTCV;
            TagStack            m_aTagStack;
            ElementFormatter    m_aFormatter;
            uno::Type           m_aPropertyType;
            bool                m_bInProperty;
            bool                m_bStartedDocument;
        };
        // -----------------------------------------------------------------------------
    } // namespace xml
    // -----------------------------------------------------------------------------

} // namespace configmgr
#endif




