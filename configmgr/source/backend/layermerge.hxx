/*************************************************************************
 *
 *  $RCSfile: layermerge.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:16:17 $
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

#ifndef CONFIGMGR_BACKEND_LAYERMERGE_HXX
#define CONFIGMGR_BACKEND_LAYERMERGE_HXX

#ifndef CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX
#include "mergedcomponentdata.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX
#include "componentdatahelper.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERHANDLER_HPP_
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    class OTreeNodeFactory;
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

        namespace backenduno = ::com::sun::star::configuration::backend;
        using backenduno::MalformedDataException;
// -----------------------------------------------------------------------------

        typedef ::cppu::WeakImplHelper1<backenduno::XLayerHandler> LayerMergeHandler_Base;

        class LayerMergeHandler
        :   public LayerMergeHandler_Base
        {
        public:
            typedef uno::Reference< lang::XMultiServiceFactory > ServiceFactory;

            explicit
            LayerMergeHandler(ServiceFactory const & _xServiceFactory , MergedComponentData & _rData, ITemplateDataProvider* aTemplateProvider = NULL);
            virtual ~LayerMergeHandler();

        // prepare merging
            void prepareLayer();
            bool prepareSublayer(OUString const & aLocale);

        // checking the result
            bool isDone() const { return m_aContext.isDone(); }
            bool isInSublayer() const { return m_bSublayer; }

            MergedComponentData &       result();
            MergedComponentData const & result() const;

        // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            void propagateAttributes(ISubtree & _rParent);

            node::Attributes makePropertyAttributes(sal_Int16 aSchemaAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void checkPropertyType(uno::Type const & _aType)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyPropertyValue(uno::Any const & _aValue)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyPropertyValue(uno::Any const & _aValue, OUString const & _aLocale)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyAttributes(INode * pNode, sal_Int16 aNodeAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void overrideLayerRoot( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            bool startOverride(INode * pNode, sal_Bool bClear) /* check if writable, mark merged */
                CFG_NOTHROW( );

            void implAddOrReplaceNode(const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void ensureUnchanged(INode const * pNode) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void setLocalizedValue(ISubtree * pProperty, uno::Any const & _aValue, OUString const & _aLocale)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void setValueAndCheck(ValueNode & _rValueNode, uno::Any const & _aValue)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
       private:
           void skipNode()          { ++m_nSkipping; }
           bool isSkipping() const  { return m_nSkipping != 0; }
           bool leaveSkippedNode()  { return m_nSkipping && m_nSkipping--; }
       private:
            struct Converter;
            MergedComponentData &   m_rData;
            DataBuilderContext      m_aContext;
            ComponentDataFactory    m_aFactory;
            OUString                m_aLocale;
            INode *                 m_pProperty;
            Converter *             m_pConverter;

            sal_uInt32              m_nSkipping;
            bool                    m_bSublayer;
        };
// -----------------------------------------------------------------------------

        /// change attributes and states of the tree to make it a proper default layer
        void promoteToDefault(MergedComponentData & _rTree);
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




