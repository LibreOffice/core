/*************************************************************************
*
*  $RCSfile: layerwriter.hxx,v $
*
*  $Revision: 1.8 $
*
*  last change: $Author: hr $ $Date: 2003-03-19 16:19:58 $
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

#include <drafts/com/sun/star/configuration/backend/XLayerHandler.hpp>

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace xml
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace beans     = ::com::sun::star::beans;
        namespace container = ::com::sun::star::container;

        namespace sax           = ::com::sun::star::xml::sax;
        namespace backenduno    = ::drafts::com::sun::star::configuration::backend;

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
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                throw (backenduno::MalformedDataException, lang::IllegalAccessException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                throw (backenduno::MalformedDataException, container::NoSuchElementException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                throw (backenduno::MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
                throw (backenduno::MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException);

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
        private:
            typedef Stack< OUString > TagStack;
            uno::Reference< uno::XInterface > m_xTCV;
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




