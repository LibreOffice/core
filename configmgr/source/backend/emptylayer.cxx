/*************************************************************************
*
*  $RCSfile: emptylayer.cxx,v $
*
*  $Revision: 1.5 $
*
*  last change: $Author: rt $ $Date: 2003-04-17 13:14:41 $
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

#include "emptylayerimpl.hxx"
#include "emptylayer.hxx"

// -----------------------------------------------------------------------------

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------

        uno::Reference< backenduno::XLayer > createEmptyLayer()
        {
            return new EmptyLayer();
        }
        // -----------------------------------------------------------------------------

        bool checkEmptyLayer(uno::Reference< backenduno::XLayer > const & xLayer )
        {
            OSL_ENSURE(xLayer.is(),"Unexpected NULL Layer");
            if (!xLayer.is()) return false;

            RequireEmptyLayer * const pChecker = new RequireEmptyLayer;
            uno::Reference< backenduno::XLayerHandler > xChecker(pChecker);

            try
            {
                xLayer->readData(xChecker);
            }
            catch (uno::Exception &)
            {
                OSL_ENSURE(!pChecker->wasEmpty(), "Checking for empty layer: exception occurred after empty layer was ended");
            }

            return pChecker->wasEmpty();
        }

        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------

        EmptyLayer::~EmptyLayer( )
        {
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL
            EmptyLayer::readData( const uno::Reference< backenduno::XLayerHandler >& aHandler )
                throw (backenduno::MalformedDataException, lang::NullPointerException, lang::WrappedTargetException, uno::RuntimeException)
        {
            if (aHandler.is())
            {
                aHandler->startLayer();
                aHandler->endLayer();
            }
            else
                throw lang::NullPointerException(OUString::createFromAscii("EmptyLayer: Null Handler"),*this);
        }
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------

        RequireEmptyLayer::RequireEmptyLayer()
        : m_bStarted(false)
        , m_bInvalid(false)
        , m_bEmpty(false)
        {
        }
        // -----------------------------------------------------------------------------

        RequireEmptyLayer::~RequireEmptyLayer( )
        {
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::startLayer(  )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            m_bInvalid  = false;
            m_bEmpty    = false;

            if (m_bStarted) fail("Layer started twice");
            m_bStarted  = true;
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::endLayer(  )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            if (!m_bStarted) fail("Layer was not started");
            m_bEmpty    = !m_bInvalid;
            m_bStarted  = false;
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::endNode(  )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::dropNode( const OUString& aName )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::endProperty(  )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::setPropertyValue( const uno::Any& aValue )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void RequireEmptyLayer::fail(sal_Char const * pMsg)
        {
            if (!m_bStarted & !m_bInvalid) pMsg = "Layer was not started";
            m_bInvalid = true;
            m_bStarted = false;

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

