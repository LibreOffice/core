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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
