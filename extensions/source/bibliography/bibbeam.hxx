/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibbeam.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:12:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
