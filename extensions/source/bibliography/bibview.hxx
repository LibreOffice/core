/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibview.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:15:49 $
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
#ifndef _BIB_VIEW_HXX
#define _BIB_VIEW_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _SV_SECTION_HXX
#include <svtools/section.hxx>
#endif

#ifndef _SV_SECTCTR_HXX
#include <svtools/sectctr.hxx>
#endif

#ifndef EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX
#include "formcontrolcontainer.hxx"
#endif
#ifndef _BIBSHORTCUTHANDLER_HXX
#include "bibshortcuthandler.hxx"
#endif

class   BibGeneralPage;
class   BibDataManager;

namespace com{ namespace sun{ namespace star{ namespace awt{ class XFocusListener;}}}}

//.........................................................................
namespace bib
{
//.........................................................................

    // -----------------------------------------------------------------------
    class BibView : public BibWindow, public FormControlContainer
    {
    private:
        BibDataManager*                                                             m_pDatMan;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable>        m_xDatMan;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener>    m_xGeneralPage;
        BibGeneralPage*                                                             m_pGeneralPage;

    private:
        DECL_STATIC_LINK(BibView, CallMappingHdl, BibView*);

    protected:
        // Window overridables
            virtual void    Resize();

        // FormControlContainer
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                            getControlContainer();

        // XLoadListener equivalents
        virtual void        _loaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void        _reloaded( const ::com::sun::star::lang::EventObject& _rEvent );

    public:
                            BibView( Window* _pParent, BibDataManager* _pDatMan, WinBits nStyle = WB_3DLOOK );
                            ~BibView();

        void                UpdatePages();

        virtual void        GetFocus();

        virtual BOOL        HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled
    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif


