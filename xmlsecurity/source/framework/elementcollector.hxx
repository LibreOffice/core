/*************************************************************************
 *
 *  $RCSfile: elementcollector.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
     * PRI_AFTERMODIFY  - this ElementCollector will notify after all
     *                    internal modifications have finished.
     * PRI_BEFOREMODIFY - this ElementCollector must notify before any
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

