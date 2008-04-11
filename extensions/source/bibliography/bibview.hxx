/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bibview.hxx,v $
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
#ifndef _BIB_VIEW_HXX
#define _BIB_VIEW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <svtools/section.hxx>
#include <svtools/sectctr.hxx>
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

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


