/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: emptylayer.cxx,v $
 * $Revision: 1.9 $
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
                throw lang::NullPointerException(rtl::OUString::createFromAscii("EmptyLayer: Null Handler"),*this);
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

        void SAL_CALL RequireEmptyLayer::overrideNode( const rtl::OUString& /*aName*/, sal_Int16 /*aAttributes*/, sal_Bool /*bClear*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addOrReplaceNode( const rtl::OUString& /*aName*/, sal_Int16 /*aAttributes*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addOrReplaceNodeFromTemplate( const rtl::OUString& /*aName*/, const backenduno::TemplateIdentifier& /*aTemplate*/, sal_Int16 /*aAttributes*/)
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

        void SAL_CALL RequireEmptyLayer::dropNode( const rtl::OUString& /*aName*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addProperty( const rtl::OUString& /*aName*/, sal_Int16 /*aAttributes*/, const uno::Type& /*aType*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::addPropertyWithValue( const rtl::OUString& /*aName*/, sal_Int16 /*aAttributes*/, const uno::Any& /*aValue*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::overrideProperty( const rtl::OUString& /*aName*/, sal_Int16 /*aAttributes*/, const uno::Type& /*aType*/, sal_Bool /*bClear*/ )
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

        void SAL_CALL RequireEmptyLayer::setPropertyValue( const uno::Any& /*aValue*/ )
            throw (backenduno::MalformedDataException, uno::RuntimeException)
        {
            failNotEmpty();
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL RequireEmptyLayer::setPropertyValueForLocale( const uno::Any& /*aValue*/, const rtl::OUString& /*aLocale*/ )
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
            rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

            throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
        }

        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
    } // namespace

    // -----------------------------------------------------------------------------
} // namespace

