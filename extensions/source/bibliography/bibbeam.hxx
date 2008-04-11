/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bibbeam.hxx,v $
 * $Revision: 1.9 $
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
