/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
