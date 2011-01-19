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

#ifndef EVENT_MOUSEEVENT_HXX
#define EVENT_MOUSEEVENT_HXX

#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XMouseEvent.hpp>

#include <cppuhelper/implbase1.hxx>

#include "uievent.hxx"


using ::rtl::OUString;

namespace DOM { namespace events {

class CMouseEvent : public cppu::ImplInheritanceHelper1< CUIEvent, XMouseEvent >
{
    friend class CEventDispatcher;
protected:
    sal_Int32 m_screenX;
    sal_Int32 m_screenY;
    sal_Int32 m_clientX;
    sal_Int32 m_clientY;
    sal_Bool m_ctrlKey;
    sal_Bool m_shiftKey;
    sal_Bool m_altKey;
    sal_Bool m_metaKey;
    sal_Int16 m_button;
    Reference< XEventTarget > m_relatedTarget;

public:

    virtual sal_Int32 SAL_CALL getScreenX() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getScreenY() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getClientX() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getClientY() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getCtrlKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getShiftKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getAltKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getMetaKey() throw (RuntimeException);
    virtual sal_Int16 SAL_CALL getButton() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getRelatedTarget()  throw(RuntimeException);

    virtual void SAL_CALL initMouseEvent(
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
                        const Reference< XEventTarget >& relatedTargetArg)
        throw(RuntimeException);

    // delegate to CUIevent
    virtual Reference< XAbstractView > SAL_CALL getView() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getDetail() throw (RuntimeException);
    virtual void SAL_CALL initUIEvent(const OUString& typeArg,
                     sal_Bool canBubbleArg,
                     sal_Bool cancelableArg,
                     const Reference< XAbstractView >& viewArg,
                     sal_Int32 detailArg) throw (RuntimeException);
    virtual OUString SAL_CALL getType() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getTarget() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getCurrentTarget() throw (RuntimeException);
    virtual PhaseType SAL_CALL getEventPhase() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException);
    virtual com::sun::star::util::Time SAL_CALL getTimeStamp() throw (RuntimeException);
    virtual void SAL_CALL stopPropagation() throw (RuntimeException);
    virtual void SAL_CALL preventDefault() throw (RuntimeException);
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg,
        sal_Bool canBubbleArg,
        sal_Bool cancelableArg)
        throw (RuntimeException);
};
}}
#endif
