/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBBEAM_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBBEAM_HXX

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <vcl/splitwin.hxx>
#include "toolbar.hxx"
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class BibDataManager;


namespace bib
{


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
            VclPtr<BibToolBar>      pToolBar;
            VclPtr<BibGridwin>      pGridWin;

            DECL_LINK_TYPED( RecalcLayout_Impl, void*, void );

        protected:

            void                    createToolBar();
            void                    createGridWin();

            // FormControlContainer ----------
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    getControlContainer() SAL_OVERRIDE;
        public:
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception >
                    getDispatchProviderInterception();

            BibBeamer(vcl::Window* pParent,BibDataManager* pDatMan, WinBits nStyle = WB_3DLOOK );
            virtual ~BibBeamer();
            virtual void dispose() SAL_OVERRIDE;

            void    SetXController(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &);

            virtual void            GetFocus() SAL_OVERRIDE;
    };


}   // namespace bib


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
