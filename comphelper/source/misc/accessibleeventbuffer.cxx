/*************************************************************************
 *
 *  $RCSfile: accessibleeventbuffer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:27:12 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "comphelper/accessibleeventbuffer.hxx"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/accessibility/AccessibleEventObject.hpp"
#include "com/sun/star/accessibility/XAccessibleEventListener.hpp"
#include "osl/diagnose.h"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace css = ::com::sun::star;

using comphelper::AccessibleEventBuffer;

struct AccessibleEventBuffer::Entry
{
    inline Entry(::css::accessibility::AccessibleEventObject const & rEvent,
                 ::css::uno::Sequence< ::css::uno::Reference<
                 ::css::uno::XInterface > > const & rListeners):
        m_aEvent(rEvent), m_aListeners(rListeners) {}

    ::css::accessibility::AccessibleEventObject m_aEvent;

    ::css::uno::Sequence<
        ::css::uno::Reference< ::css::uno::XInterface > > m_aListeners;
};

AccessibleEventBuffer::AccessibleEventBuffer()
{}

AccessibleEventBuffer::AccessibleEventBuffer(
    AccessibleEventBuffer const & rOther):
    m_aEntries(rOther.m_aEntries)
{}

AccessibleEventBuffer::~AccessibleEventBuffer()
{}

AccessibleEventBuffer
AccessibleEventBuffer::operator =(AccessibleEventBuffer const & rOther)
{
    m_aEntries = rOther.m_aEntries;
    return *this;
}

void AccessibleEventBuffer::addEvent(
    ::css::accessibility::AccessibleEventObject const & rEvent,
    ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >
    const & rListeners)
{
    m_aEntries.push_back(Entry(rEvent, rListeners));
}

void AccessibleEventBuffer::sendEvents() const
{
    for (Entries::const_iterator aIt(m_aEntries.begin());
         aIt != m_aEntries.end(); ++aIt)
        for (::sal_Int32 i = 0; i < aIt->m_aListeners.getLength(); ++i)
        {
            ::css::uno::Reference<
                  ::css::accessibility::XAccessibleEventListener > xListener(
                      aIt->m_aListeners[i], ::css::uno::UNO_QUERY);
            if (xListener.is())
                try
                {
                    xListener->notifyEvent(aIt->m_aEvent);
                }
                catch (::css::uno::RuntimeException & rEx)
                {
                    OSL_TRACE(
                        "comphelper::AccessibleEventBuffer::sendEvents:"
                        " caught %s\n",
                        ::rtl::OUStringToOString(
                            rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
                }
        }
}
