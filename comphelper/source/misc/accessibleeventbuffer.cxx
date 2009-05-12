/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibleeventbuffer.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

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
