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



#ifndef ADRBEAM_HXX
#define ADRBEAM_HXX

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/form/XForm.hpp>
// #100312# -----------------
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <vcl/splitwin.hxx>
#include "toolbar.hxx"
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class BibDataManager;

//.........................................................................
namespace bib
{
//.........................................................................

    class BibGridwin;
    class BibBeamer
            :public BibSplitWindow
            ,public FormControlContainer
    {
        private:

            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            m_xController;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xToolBarRef;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xGridRef;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  m_xGridWin;

            BibDataManager*         pDatMan;
            BibToolBar*             pToolBar;
            BibGridwin*             pGridWin;

            DECL_LINK( RecalcLayout_Impl, void* );

        protected:

            void                    createToolBar();
            void                    createGridWin();

            // FormControlContainer ----------
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    getControlContainer();
        public:
            // #100312# -------------------
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception >
                    getDispatchProviderInterception();

            BibBeamer(Window* pParent,BibDataManager* pDatMan, WinBits nStyle = WB_3DLOOK );
            ~BibBeamer();

            void    SetXController(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &);

            virtual void            GetFocus();
    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif
