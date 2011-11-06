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



#ifndef SD_TOOLPANELUIELEMENT_HXX
#define SD_TOOLPANELUIELEMENT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
/** === end UNO includes === **/

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class TreeNode;

    //==================================================================================================================
    //= ToolPanelUIElement
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::ui::XUIElement
                                                >   ToolPanelUIElement_Base;
    class ToolPanelUIElement    :public ::cppu::BaseMutex
                                ,public ToolPanelUIElement_Base
    {
    public:
        ToolPanelUIElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
            const ::rtl::OUString& i_rResourceURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >& i_rToolPanel
        );

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException);

        void checkDisposed();
        ::osl::Mutex& getMutex() { return m_aMutex; }

    protected:
        virtual ~ToolPanelUIElement();

        // OComponentHelper
        virtual void SAL_CALL disposing();

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >   m_xFrame;
        const ::rtl::OUString                                                       m_sResourceURL;
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >  m_xToolPanel;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANELUIELEMENT_HXX
