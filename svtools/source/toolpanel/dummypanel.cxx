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



#include "precompiled_svtools.hxx"

#include "dummypanel.hxx"

//........................................................................
namespace svt
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;

    //====================================================================
    //= DummyPanel
    //====================================================================
    //--------------------------------------------------------------------
    DummyPanel::DummyPanel()
    {
    }

    //--------------------------------------------------------------------
    DummyPanel::~DummyPanel()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_IREFERENCE( DummyPanel )

    //--------------------------------------------------------------------
    void DummyPanel::Activate( Window& )
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::Deactivate()
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::SetSizePixel( const Size& )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DummyPanel::GetDisplayName() const
    {
        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    Image DummyPanel::GetImage() const
    {
        return Image();
    }

    //--------------------------------------------------------------------
    rtl::OString DummyPanel::GetHelpID() const
    {
        return rtl::OString();
    }

    //--------------------------------------------------------------------
    void DummyPanel::GrabFocus()
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::Dispose()
    {
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > DummyPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        (void)i_rParentAccessible;
        return NULL;
    }

//........................................................................
} // namespace svt
//........................................................................
