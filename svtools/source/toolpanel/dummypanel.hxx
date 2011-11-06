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



#ifndef DUMMYPANEL_HXX
#define DUMMYPANEL_HXX

#include "svtools/toolpanel/toolpanel.hxx"
#include "svtools/toolpanel/refbase.hxx"

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= DummyPanel
    //====================================================================
    /// is a dummy implementation of the IToolPanel interface
    class DummyPanel    :public RefBase
                        ,public IToolPanel
    {
    public:
        DummyPanel();
        ~DummyPanel();

        // IToolPanel
        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual rtl::OString GetHelpID() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    );

        DECLARE_IREFERENCE()
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // DUMMYPANEL_HXX
