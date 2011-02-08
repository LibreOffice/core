/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _ELEMENTCOLLECTOR_HXX
#define _ELEMENTCOLLECTOR_HXX

#include "elementmark.hxx"
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>

class ElementCollector : public ElementMark
/****** elementcollector.hxx/CLASS ElementCollector ***************************
 *
 *   NAME
 *  ElementCollector -- Class to manipulate an element collector
 *
 *   FUNCTION
 *  This class is derived from the ElementMark class. Beyond the function
 *  of the ElementMark class, this class also maintains the priority, and
 *  manages the notify process
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /*
     * the notify priority, is one of following values:
     * AFTERMODIFY  - this ElementCollector will notify after all
     *                    internal modifications have finished.
     * BEFOREMODIFY - this ElementCollector must notify before any
     *                    internal modification happens.
     */
    com::sun::star::xml::crypto::sax::ElementMarkPriority m_nPriority;

    /*
     * the modify flag, representing whether which elementcollector will
     * modify its data.
     */
    bool m_bToModify;

    /* the notify enable flag, see notifyListener method */
    bool m_bAbleToNotify;

    /* whether the listener has been notified */
    bool m_bNotified;

    /* the listener to be notified */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XReferenceResolvedListener > m_xReferenceResolvedListener;

public:
    ElementCollector(
        sal_Int32 nSecurityId,
        sal_Int32 nBufferId,
        com::sun::star::xml::crypto::sax::ElementMarkPriority nPriority,
        bool bToModify,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XReferenceResolvedListener >&
            xReferenceResolvedListener);
    virtual ~ElementCollector() {};

    //bool isInternalNotificationSuppressed() const;
    com::sun::star::xml::crypto::sax::ElementMarkPriority getPriority() const;
    bool getModify() const;
    void notifyListener();
    bool isAbleToNotify() const;
    void setReferenceResolvedListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XReferenceResolvedListener >&
            referenceResolvedListener);
    void setSecurityId(sal_Int32 nSecurityId);
    void doNotify();
    ElementCollector* clone(
        sal_Int32 nId,
        com::sun::star::xml::crypto::sax::ElementMarkPriority nPriority ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
