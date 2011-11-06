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



#ifndef _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/window.hxx>
#include <tools/resid.hxx>
// #95343# --------------------
#include <com/sun/star/awt/Size.hpp>

//............................................................................
namespace pcr
{
//............................................................................

    class OPropertyEditor;

    //========================================================================
    //=
    //========================================================================
    class OPropertyBrowserView : public Window
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

        OPropertyEditor*        m_pPropBox;
        sal_uInt16              m_nActivePage;
        Link                    m_aPageActivationHandler;

    protected:
        virtual void Resize();
        virtual void GetFocus();
        virtual long Notify( NotifyEvent& _rNEvt );

    public:
        OPropertyBrowserView(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xORB,
                        Window* pParent, WinBits nBits = 0);

        virtual ~OPropertyBrowserView();

        OPropertyEditor&    getPropertyBox() { return *m_pPropBox; }

        // page handling
        sal_uInt16  getActivaPage() const { return m_nActivePage; }
        void        activatePage(sal_uInt16 _nPage);

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

        // #95343# ------------------
        ::com::sun::star::awt::Size getMinimumSize();

    protected:
        DECL_LINK(OnPageActivation, void*);
    };


//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_

