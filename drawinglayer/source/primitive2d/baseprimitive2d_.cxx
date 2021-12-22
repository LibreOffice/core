/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <drawinglayer/primitive2d/baseprimitive2d_.hxx>
#include <basegfx/range/b2drange.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <basegfx/utils/canvastools.hxx>

using namespace css;

namespace drawinglayer::primitive2d
{
    BasePrimitive2D_::BasePrimitive2DImplBase_::BasePrimitive2DImplBase_(
        drawinglayer::primitive2d::BasePrimitive2D_& rSource)
    :   m_pSource(&rSource)
    {
    }

    BasePrimitive2D_::BasePrimitive2DImplBase_::~BasePrimitive2DImplBase_()
    {
    }

    void BasePrimitive2D_::BasePrimitive2DImplBase_::acquire() noexcept
    {
        OWeakObject::acquire();
    }

    void BasePrimitive2D_::BasePrimitive2DImplBase_::release() noexcept
    {
        if (osl_atomic_decrement(&m_refCount) != 0)
            return;

        // ensure no other references are created, via the weak connection point, from now on
        disposeWeakConnectionPoint();

        // restore reference count:
        osl_atomic_increment(&m_refCount);

        // call parent, this will delete the instance
        OWeakObject::release();
    }

    css::uno::Any BasePrimitive2D_::BasePrimitive2DImplBase_::queryInterface(css::uno::Type const& rType)
    {
        css::uno::Any aReturn = ::cppu::queryInterface(
            rType, static_cast<uno::XWeak*>(this), static_cast<lang::XComponent*>(this),
            static_cast<lang::XTypeProvider*>(this), static_cast<graphic::XPrimitive2D*>(this),
            static_cast<util::XAccounting*>(this));
        if (aReturn.hasValue())
            return aReturn;
        return OWeakObject::queryInterface(rType);
    }

    void BasePrimitive2D_::BasePrimitive2DImplBase_::dispose()
    {
        // forget data source
        m_pSource = nullptr;
    }

    void BasePrimitive2D_::BasePrimitive2DImplBase_::addEventListener(
        css::uno::Reference<css::lang::XEventListener> const&)
    {
        assert(false);
    }

    void BasePrimitive2D_::BasePrimitive2DImplBase_::removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const&)
    {
        assert(false);
    }

    css::uno::Sequence<css::uno::Type> BasePrimitive2D_::BasePrimitive2DImplBase_::getTypes()
    {
        static const css::uno::Sequence<uno::Type> aTypeList{
            cppu::UnoType<uno::XWeak>::get(), cppu::UnoType<lang::XComponent>::get(),
            cppu::UnoType<lang::XTypeProvider>::get(), cppu::UnoType<graphic::XPrimitive2D>::get(),
            cppu::UnoType<util::XAccounting>::get()
        };

        return aTypeList;
    }

    css::uno::Sequence<sal_Int8> SAL_CALL BasePrimitive2D_::BasePrimitive2DImplBase_::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> SAL_CALL
        BasePrimitive2D_::BasePrimitive2DImplBase_::getDecomposition(
            const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
    {
        css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> aRetval;

        if(m_pSource)
        {
            const auto aViewInformation = drawinglayer::geometry::createViewInformation2D(rViewParameters);
            const drawinglayer::primitive2d::Primitive2D_VectorSharedPtr& rVector(
                m_pSource->getB2DDecomposition(aViewInformation));

            if(rVector && !rVector->empty())
            {
                aRetval.realloc(rVector->size());
                ::css::uno::Reference<::css::graphic::XPrimitive2D>* pTarget(aRetval.getArray());

                for(auto& rSource : *rVector)
                {
                    *pTarget++ = rSource ? rSource->getUnoPrimitive2D() : nullptr;
                }
            }
        }

        return aRetval;
    }

    css::geometry::RealRectangle2D SAL_CALL
        BasePrimitive2D_::BasePrimitive2DImplBase_::getRange(
            const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
    {
        if(m_pSource)
        {
            const auto aViewInformation = drawinglayer::geometry::createViewInformation2D(rViewParameters);
            return basegfx::unotools::rectangle2DFromB2DRectangle(m_pSource->getB2DRange(aViewInformation));
        }

        return basegfx::unotools::rectangle2DFromB2DRectangle(basegfx::B2DRange());
    }

    sal_Int64 SAL_CALL BasePrimitive2D_::BasePrimitive2DImplBase_::estimateUsage()
    {
        if(m_pSource)
        {
            return m_pSource->estimateUsage();
        }

        return 0;
    }
};

namespace drawinglayer::primitive2d
{
    BasePrimitive2D_::BasePrimitive2D_()
    :   m_pImplBase(nullptr)
    {
    }

    BasePrimitive2D_::~BasePrimitive2D_()
    {
        if(m_pImplBase)
        {
            // data source for UNO API gets deleted, so also
            // dispose here
            m_pImplBase->dispose();
            m_pImplBase->release();
        }
    }

    ::css::uno::Reference<::css::graphic::XPrimitive2D> BasePrimitive2D_::getUnoPrimitive2D() const
    {
        if(!m_pImplBase)
        {
            BasePrimitive2D_* pThat(const_cast<BasePrimitive2D_*>(this));
            pThat->m_pImplBase = new BasePrimitive2DImplBase_(*pThat);

            // acquire once locally to hold until destructor
            pThat->m_pImplBase->acquire();
        }

        return ::css::uno::Reference<::css::graphic::XPrimitive2D>(m_pImplBase);
    }

    bool BasePrimitive2D_::operator==(const BasePrimitive2D_& rPrimitive) const
    {
        return (getPrimitive2DID() == rPrimitive.getPrimitive2DID());
    }

    sal_Int64 BasePrimitive2D_::estimateUsage() const
    {
        return 0;
    }

    Primitive2D_SharedPtr BasePrimitive2D_::getKnownImplementation(const ::css::uno::Reference<::css::graphic::XPrimitive2D>& rSource)
    {
        if(rSource.is())
        {
            BasePrimitive2D_::BasePrimitive2DImplBase_* pKnown(
                dynamic_cast<BasePrimitive2D_::BasePrimitive2DImplBase_*>(rSource.get()));

            if(pKnown)
            {
                BasePrimitive2D_* pBase(pKnown->getKnownImplementation());

                if(pBase)
                {
                    return pBase->getptr();
                }
            }
        }

        return Primitive2D_SharedPtr();
    }
}

namespace drawinglayer::primitive2d
{
    TestPrimitive2D::TestPrimitive2D()
    :   BasePrimitive2D_(),
        m_aDecomposition()
    {
    }

    TestPrimitive2D::~TestPrimitive2D()
    {
    }

    bool TestPrimitive2D::operator==(const BasePrimitive2D_& rPrimitive) const
    {
        if(BasePrimitive2D_::operator==(rPrimitive))
        {
            return m_aDecomposition == static_cast< const TestPrimitive2D& >(rPrimitive).m_aDecomposition;
        }

        return false;
    }

    sal_uInt32 TestPrimitive2D::getPrimitive2DID() const
    {
        return 123456;
    }

    const Primitive2D_VectorSharedPtr& TestPrimitive2D::getB2DDecomposition(
        const geometry::ViewInformation2D& /*rViewInformation*/) const
    {
        if(!m_aDecomposition)
        {
            Primitive2D_Vector* pVector(new Primitive2D_Vector());
            pVector->push_back(TestPrimitive2D::create());
            pVector->push_back(TestPrimitive2D::create());
            const_cast<TestPrimitive2D*>(this)->m_aDecomposition.reset(pVector);
        }

        return m_aDecomposition;
    }

    basegfx::B2DRange TestPrimitive2D::getB2DRange(
        const geometry::ViewInformation2D& /*rViewInformation*/) const
    {
        return basegfx::B2DRange();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
