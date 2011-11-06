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
#include "precompiled_svtools.hxx"

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/textwindowpeer.hxx>
#include <svtools/textview.hxx>
#include "svtaccessiblefactory.hxx"

namespace css = ::com::sun::star;

namespace svt
{
    TextWindowPeer::TextWindowPeer(::TextView & rView, bool bCompoundControlChild):
        m_rEngine(*rView.GetTextEngine()), m_rView(rView), m_bCompoundControlChild(bCompoundControlChild)
    {
        SetWindow(rView.GetWindow());
        m_pFactoryAccess.reset( new AccessibleFactoryAccess );
    }

    // virtual
    TextWindowPeer::~TextWindowPeer()
    {
    }

    ::css::uno::Reference< ::css::accessibility::XAccessibleContext > TextWindowPeer::CreateAccessibleContext()
    {
        return m_pFactoryAccess->getFactory().createAccessibleTextWindowContext(
            this, m_rEngine, m_rView, m_bCompoundControlChild
        );
    }
}
