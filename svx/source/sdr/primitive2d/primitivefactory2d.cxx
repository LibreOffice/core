/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/primitiveFactory2d.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// UNO API helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        uno::Reference< uno::XInterface > SAL_CALL XPrimitiveFactory2DProvider_createInstance(
            const uno::Reference< lang::XMultiServiceFactory >& /*rSMgr*/) throw( uno::Exception )
        {
            return *(new PrimitiveFactory2D());
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// UNO API helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        PrimitiveFactory2D::PrimitiveFactory2D()
        :   PrimitiveFactory2DImplBase(m_aMutex)
        {
        }

        Primitive2DSequence SAL_CALL PrimitiveFactory2D::createPrimitivesFromXShape(
            const uno::Reference< drawing::XShape >& xShape,
            const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException)
        {
            Primitive2DSequence aRetval;

            if(xShape.is())
            {
                SdrObject* pSource = GetSdrObjectFromXShape(xShape);

                if(pSource)
                {
                    const sdr::contact::ViewContact& rSource(pSource->GetViewContact());
                    aRetval = rSource.getViewIndependentPrimitive2DSequence();
                }
            }

            return aRetval;
        }

        Primitive2DSequence SAL_CALL PrimitiveFactory2D::createPrimitivesFromXDrawPage(
            const uno::Reference< drawing::XDrawPage >& xDrawPage,
            const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException)
        {
            Primitive2DSequence aRetval;

            if(xDrawPage.is())
            {
                SdrPage* pSource = GetSdrPageFromXDrawPage(xDrawPage);

                if(pSource)
                {
                    const sdr::contact::ViewContact& rSource(pSource->GetViewContact());

                    aRetval = rSource.getViewIndependentPrimitive2DSequence();
                }
            }

            return aRetval;
        }

        rtl::OUString PrimitiveFactory2D::getImplementationName_Static()
        {
            static rtl::OUString aRetval(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.graphic.PrimitiveFactory2D"));
            return aRetval;
        }

        uno::Sequence< rtl::OUString > PrimitiveFactory2D::getSupportedServiceNames_Static()
        {
            static uno::Sequence< rtl::OUString > aSeq;
            osl::Mutex aMutex;
            osl::MutexGuard aGuard( aMutex );

            if(!aSeq.getLength())
            {
                aSeq.realloc(1L);
                aSeq.getArray()[0L] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.PrimitiveFactory2D"));
            }

            return aSeq;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
