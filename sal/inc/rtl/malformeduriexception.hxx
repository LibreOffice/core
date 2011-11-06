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



#ifndef INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
#define INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX

#include "rtl/ustring.hxx"

namespace rtl {

/** An exception indicating a malformed URI.

    <P>Used when parsing (part of) a URI fails for syntactical reasons.</P>
 */
class SAL_EXCEPTION_DLLPUBLIC_EXPORT MalformedUriException
{
public:
    /** Create a MalformedUriException.

        @param rMessage
        A message containing any details about the exception.
     */
    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        rtl::OUString const & rMessage): m_aMessage(rMessage) {}

    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        MalformedUriException const & other): m_aMessage(other.m_aMessage) {}

    inline SAL_EXCEPTION_DLLPRIVATE ~MalformedUriException() {}

    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException operator =(
        MalformedUriException const & rOther)
    { m_aMessage = rOther.m_aMessage; return *this; }

    /** Get the message.

        @return
        A reference to the message.  The reference is valid for the lifetime of
        this MalformedUriException.
     */
    inline SAL_EXCEPTION_DLLPRIVATE rtl::OUString const & getMessage() const
    { return m_aMessage; }

private:
    rtl::OUString m_aMessage;
};

}

#endif // INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
