/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerdefaultremover.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "layerdefaultremover.hxx"

// -----------------------------------------------------------------------------

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        // -----------------------------------------------------------------------------

        LayerDefaultRemover::LayerDefaultRemover(uno::Reference< backenduno::XLayerHandler > const & _xResultHandler)
            :m_xResultHandler(_xResultHandler)
        {
        }
        // -----------------------------------------------------------------------------

        LayerDefaultRemover::~LayerDefaultRemover( )
        {
        }
        // -----------------------------------------------------------------------------

        bool LayerDefaultRemover::hasPendingProperty()
        {
            return m_aPropName.Name.getLength()!=0;
        }
        // -----------------------------------------------------------------------------

        void LayerDefaultRemover::clearPendingProperty()
        {
            m_aPropName = PropertyStruct();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::startLayer(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            clearPendingProperty();
            m_xResultHandler->startLayer();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::endLayer(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if (!m_aNodeStack.empty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal node started operation";
                raiseMalformedDataException(pMsg);
            }
            m_xResultHandler->endLayer();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if (aAttributes == 0 && !bClear)
            {
                m_aNodeStack.push_back(aName);
            }
            else
            {
                playBackNodeStack();
                m_xResultHandler->overrideNode(aName,aAttributes,bClear);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addOrReplaceNode(aName, aAttributes);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addOrReplaceNodeFromTemplate(aName,aTemplate,aAttributes);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::endNode(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if (m_aNodeStack.empty())
            {
                m_xResultHandler->endNode();
            }
            else
            {
                m_aNodeStack.pop_back();
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::dropNode( const rtl::OUString& aName )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->dropNode(aName);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addProperty (aName,aAttributes,aType);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addPropertyWithValue(aName,aAttributes,aValue);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if (aAttributes != 0 || bClear)
            {
                m_aPropName.Name=rtl::OUString();
                playBackNodeStack();
                m_xResultHandler->overrideProperty(aName,aAttributes,aType,bClear);
            }
            else
            {
                m_aPropName.Name = aName;
                m_aPropName.Type = aType;
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::endProperty(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                clearPendingProperty();
            }
            else
                m_xResultHandler->endProperty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::setPropertyValue( const uno::Any& aValue )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack(true);
            m_xResultHandler->setPropertyValue(aValue);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack(true);
            m_xResultHandler->setPropertyValueForLocale(aValue,aLocale);
        }
        // -----------------------------------------------------------------------------

        void LayerDefaultRemover::playBackNodeStack(bool bPlayProperty)
        {
            if (!bPlayProperty && hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if ( !hasPendingProperty() && bPlayProperty && !m_aNodeStack.empty() )
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal Operation: Operation requires a started property";
                raiseMalformedDataException(pMsg);
            }
            if (!m_aNodeStack.empty())
            {
                for (std::vector<rtl::OUString>::iterator aIter = m_aNodeStack.begin();
                     aIter != m_aNodeStack.end(); aIter++)
                {
                    m_xResultHandler->overrideNode(*aIter, 0,false);
                }
                m_aNodeStack.clear();
            }
            if (bPlayProperty)
            {
                if (hasPendingProperty())
                {
                    m_xResultHandler->overrideProperty(m_aPropName.Name,0,m_aPropName.Type,false);
                    clearPendingProperty();
                }
            }
        }
        // -----------------------------------------------------------------------------

        void LayerDefaultRemover::raiseMalformedDataException(sal_Char const * pMsg)
        {
            OSL_ASSERT(pMsg);
            rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

            throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
        }

        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
    } // namespace

    // -----------------------------------------------------------------------------
} // namespace

