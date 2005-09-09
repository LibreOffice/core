/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementcollector.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:15:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _ELEMENTCOLLECTOR_HXX
#define _ELEMENTCOLLECTOR_HXX

#include "elementmark.hxx"

#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_XREFERENCERESOLVEDLISTENER_HPP_
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_ELEMENTMARKPRIORITY_HPP_
#include <com/sun/star/xml/crypto/sax/ElementMarkPriority.hpp>
#endif

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
 *   HISTORY
 *  05.01.2004 -    implemented
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

