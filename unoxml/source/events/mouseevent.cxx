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

#include "mouseevent.hxx"

namespace DOM { namespace events
{

    sal_Int32 SAL_CALL CMouseEvent::getScreenX() throw (RuntimeException)
    {
        return m_screenX;
    }
    sal_Int32 SAL_CALL CMouseEvent::getScreenY() throw (RuntimeException)
    {
        return m_screenY;
    }
    sal_Int32 SAL_CALL CMouseEvent::getClientX() throw (RuntimeException)
    {
        return m_clientX;
    }
    sal_Int32 SAL_CALL CMouseEvent::getClientY() throw (RuntimeException)
    {
        return m_clientY;
    }
    sal_Bool SAL_CALL CMouseEvent::getCtrlKey() throw (RuntimeException)
    {
        return m_ctrlKey;
    }
    sal_Bool SAL_CALL CMouseEvent::getShiftKey() throw (RuntimeException)
    {
        return m_shiftKey;
    }
    sal_Bool SAL_CALL CMouseEvent::getAltKey() throw (RuntimeException)
    {
        return m_altKey;
    }
    sal_Bool SAL_CALL CMouseEvent::getMetaKey() throw (RuntimeException)
    {
        return m_metaKey;
    }
    sal_Int16 SAL_CALL CMouseEvent::getButton() throw (RuntimeException)
    {
        return m_button;
    }
    Reference< XEventTarget > SAL_CALL CMouseEvent::getRelatedTarget()  throw(RuntimeException)
    {
        return m_relatedTarget;
    }

    void SAL_CALL CMouseEvent::initMouseEvent(
                        const OUString& typeArg,
                        sal_Bool canBubbleArg,
                        sal_Bool cancelableArg,
                        const Reference< XAbstractView >& viewArg,
                        sal_Int32 detailArg,
                        sal_Int32 screenXArg,
                        sal_Int32 screenYArg,
                        sal_Int32 clientXArg,
                        sal_Int32 clientYArg,
                        sal_Bool ctrlKeyArg,
                        sal_Bool altKeyArg,
                        sal_Bool shiftKeyArg,
                        sal_Bool metaKeyArg,
                        sal_Int16 buttonArg,
                        const Reference< XEventTarget >& /*relatedTargetArg*/)
        throw(RuntimeException)
    {
        CUIEvent::initUIEvent(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg);
        m_screenX = screenXArg;
        m_screenY = screenYArg;
        m_clientX = clientXArg;
        m_clientY = clientYArg;
        m_ctrlKey = ctrlKeyArg;
        m_altKey = altKeyArg;
        m_shiftKey = shiftKeyArg;
        m_metaKey = metaKeyArg;
        m_button = buttonArg;
    }

    // delegate to CUIEvent, since we are inheriting from CUIEvent and XUIEvent
    Reference< XAbstractView > SAL_CALL CMouseEvent::getView() throw(RuntimeException)
    {
        return CUIEvent::getView();
    }

    sal_Int32 SAL_CALL CMouseEvent::getDetail() throw(RuntimeException)
    {
        return CUIEvent::getDetail();
    }

    void SAL_CALL CMouseEvent::initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const Reference< XAbstractView >& viewArg,
                     sal_Int32 detailArg) throw(RuntimeException)
    {
        CUIEvent::initUIEvent(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg);
    }

    OUString SAL_CALL CMouseEvent::getType() throw (RuntimeException)
    {
        return CUIEvent::getType();
    }

    Reference< XEventTarget > SAL_CALL CMouseEvent::getTarget() throw (RuntimeException)
    {
        return CUIEvent::getTarget();
    }

    Reference< XEventTarget > SAL_CALL CMouseEvent::getCurrentTarget() throw (RuntimeException)
    {
        return CUIEvent::getCurrentTarget();
    }

    PhaseType SAL_CALL CMouseEvent::getEventPhase() throw (RuntimeException)
    {
        return CUIEvent::getEventPhase();
    }

    sal_Bool SAL_CALL CMouseEvent::getBubbles() throw (RuntimeException)
    {
        return CEvent::getBubbles();
    }

    sal_Bool SAL_CALL CMouseEvent::getCancelable() throw (RuntimeException)
    {
        return CUIEvent::getCancelable();
    }

    com::sun::star::util::Time SAL_CALL CMouseEvent::getTimeStamp() throw (RuntimeException)
    {
        return CUIEvent::getTimeStamp();
    }

    void SAL_CALL CMouseEvent::stopPropagation() throw (RuntimeException)
    {
        CUIEvent::stopPropagation();
    }

    void SAL_CALL CMouseEvent::preventDefault() throw (RuntimeException)
    {
        CUIEvent::preventDefault();
    }

    void SAL_CALL CMouseEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        // base initializer
        CUIEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}}

