/*************************************************************************
*
*  $RCSfile: layerdefaultremover.cxx,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: rt $ $Date: 2003-04-17 13:15:44 $
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

        LayerDefaultRemover::LayerDefaultRemover(ResultHandler const & _xResultHandler)
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
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            clearPendingProperty();
            m_xResultHandler->startLayer();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::endLayer(  )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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

        void SAL_CALL LayerDefaultRemover::overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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

        void SAL_CALL LayerDefaultRemover::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addOrReplaceNode(aName, aAttributes);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addOrReplaceNodeFromTemplate(aName,aTemplate,aAttributes);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::endNode(  )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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

        void SAL_CALL LayerDefaultRemover::dropNode( const OUString& aName )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->dropNode(aName);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addProperty (aName,aAttributes,aType);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack();
            m_xResultHandler->addPropertyWithValue(aName,aAttributes,aValue);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (hasPendingProperty())
            {
                sal_Char const * pMsg =
                "LayerDefaultRemover: Illegal property started operation";
                raiseMalformedDataException(pMsg);
            }
            if (aAttributes != 0 || bClear)
            {
                m_aPropName.Name=OUString();
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
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
        {
            playBackNodeStack(true);
            m_xResultHandler->setPropertyValue(aValue);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerDefaultRemover::setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
            throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
                for (NodeStack::iterator aIter = m_aNodeStack.begin();
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
            OUString sMsg = OUString::createFromAscii(pMsg);

            throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
        }

        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
    } // namespace

    // -----------------------------------------------------------------------------
} // namespace

