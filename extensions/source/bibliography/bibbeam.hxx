/*************************************************************************
 *
 *  $RCSfile: bibbeam.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 16:12:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ADRBEAM_HXX
#define ADRBEAM_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
// #100312# -----------------
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifndef _SV_SPLITWIN_HXX
#include <vcl/splitwin.hxx>
#endif

#ifndef _BIB_TOOLBAR_HXX
#include "toolbar.hxx"
#endif
#ifndef EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX
#include "formcontrolcontainer.hxx"
#endif
#ifndef _BIBSHORTCUTHANDLER_HXX
#include "bibshortcuthandler.hxx"
#endif

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
