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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_TOOLS_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_TOOLS_HXX

#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


#include "Section.hxx"
#include "strings.hxx"

namespace reportdesign
{
    /** uses the XChild interface to get the section from any child of it.
     *
     * \param _xReportComponent A report component which is a child of the section.
     * \return The section where this report component resists in.
     */
    css::uno::Reference< css::report::XSection> lcl_getSection(const css::uno::Reference< css::uno::XInterface>& _xReportComponent);

    /** throws an illegal argument exception. The message text is the resource RID_STR_ERROR_WRONG_ARGUMENT + the type as reference.
     *
     * \param _sTypeName The reference where to look for the correct values.
     * \param ExceptionContext_ The exception context.
     * \param ArgumentPosition_ The argument position.
     */
    void throwIllegallArgumentException(const OUString& _sTypeName
                                        ,const css::uno::Reference< css::uno::XInterface >& ExceptionContext_
                                        ,sal_Int16 ArgumentPosition_);

    /** clones the given object
    *
    * \param _xReportComponent the object to be cloned
    * \param _xFactory  the factory to create the clone
    * \param _sServiceName the service of the to be cloned object
    * \return the clone
    */
    css::uno::Reference< css::util::XCloneable > cloneObject(
                                        const css::uno::Reference< css::report::XReportComponent>& _xReportComponent
                                        ,const css::uno::Reference< css::lang::XMultiServiceFactory>& _xFactory
                                        ,const OUString& _sServiceName);

    class OShapeHelper
    {
    public:
        template<typename T> static void setSize(const css::awt::Size& aSize,T* _pShape)
        {
            OSL_ENSURE(aSize.Width >= 0 && aSize.Height >= 0,"Illegal width or height!");

            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                css::awt::Size aOldSize = _pShape->m_aProps.aComponent.m_xShape->getSize();
                if ( aOldSize.Height != aSize.Height || aOldSize.Width != aSize.Width )
                {
                    _pShape->m_aProps.aComponent.m_nWidth = aOldSize.Width;
                    _pShape->m_aProps.aComponent.m_nHeight = aOldSize.Height;
                    _pShape->m_aProps.aComponent.m_xShape->setSize(aSize);
                }
            }
            _pShape->set(PROPERTY_WIDTH,aSize.Width,_pShape->m_aProps.aComponent.m_nWidth);
            _pShape->set(PROPERTY_HEIGHT,aSize.Height,_pShape->m_aProps.aComponent.m_nHeight);
        }
        template<typename T> static css::awt::Size getSize( T* _pShape )
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                css::awt::Size aSize = _pShape->m_aProps.aComponent.m_xShape->getSize();
                OSL_ENSURE(aSize.Width >= 0 && aSize.Height >= 0,"Illegal width or height!");
                return aSize;
            }
            return css::awt::Size(_pShape->m_aProps.aComponent.m_nWidth,_pShape->m_aProps.aComponent.m_nHeight);
        }

        template<typename T> static void setPosition( const css::awt::Point& _aPosition ,T* _pShape)
        {
            // we know it is not allowed that the position in smaller 0, but in NbcMove() it will handled right.
            // only at 'Undo' it is possible to short set the position smaller 0
            // OSL_ENSURE(_aPosition.X >= 0 && _aPosition.Y >= 0,"set to Illegal position!");
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            css::awt::Point aOldPos;
            aOldPos.X = _pShape->m_aProps.aComponent.m_nPosX;
            aOldPos.Y = _pShape->m_aProps.aComponent.m_nPosY;

            css::awt::Point aPosition(_aPosition);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                aOldPos = _pShape->m_aProps.aComponent.m_xShape->getPosition();
                if ( aOldPos.X != aPosition.X || aOldPos.Y != aPosition.Y )
                {
                    _pShape->m_aProps.aComponent.m_nPosX = aOldPos.X;
                    _pShape->m_aProps.aComponent.m_nPosY = aOldPos.Y;
                    _pShape->m_aProps.aComponent.m_xShape->setPosition(aPosition);
                }
            }
            _pShape->set(PROPERTY_POSITIONX,aPosition.X,aOldPos.X);
            _pShape->set(PROPERTY_POSITIONY,aPosition.Y,aOldPos.Y);
        }
        template<typename T> static css::awt::Point getPosition(T* _pShape)
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                css::awt::Point aPosition = _pShape->m_aProps.aComponent.m_xShape->getPosition();
                return aPosition;
            }
            return css::awt::Point(_pShape->m_aProps.aComponent.m_nPosX,_pShape->m_aProps.aComponent.m_nPosY);
        }
        template<typename T> static void setParent( const css::uno::Reference< css::uno::XInterface >& Parent, T* _pShape)
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            _pShape->m_aProps.aComponent.m_xParent = css::uno::Reference< css::container::XChild >(Parent,css::uno::UNO_QUERY);
            css::uno::Reference< css::container::XChild > xChild;
            comphelper::query_aggregation(_pShape->m_aProps.aComponent.m_xProxy,xChild);
            if ( xChild.is() )
                xChild->setParent(Parent);
        }
        template<typename T> static css::uno::Reference< css::uno::XInterface > getParent( T* _pShape )
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            css::uno::Reference< css::container::XChild > xChild;
            comphelper::query_aggregation(_pShape->m_aProps.aComponent.m_xProxy,xChild);
            if ( xChild.is() )
                    return xChild->getParent();
            return _pShape->m_aProps.aComponent.m_xParent;
        }
    };

} // namespace reportdesign

#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
