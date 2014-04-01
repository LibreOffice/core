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
#ifndef _BIB_VIEW_HXX
#define _BIB_VIEW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class   BibGeneralPage;
class   BibDataManager;

namespace com{ namespace sun{ namespace star{ namespace awt{ class XFocusListener;}}}}


namespace bib
{



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
            virtual void    Resize() SAL_OVERRIDE;

        // FormControlContainer
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                            getControlContainer() SAL_OVERRIDE;

        // XLoadListener equivalents
        virtual void        _loaded( const ::com::sun::star::lang::EventObject& _rEvent ) SAL_OVERRIDE;
        virtual void        _reloaded( const ::com::sun::star::lang::EventObject& _rEvent ) SAL_OVERRIDE;

    public:
                            BibView( Window* _pParent, BibDataManager* _pDatMan, WinBits nStyle = WB_3DLOOK );
                            virtual ~BibView();

        void                UpdatePages();

        virtual void        GetFocus() SAL_OVERRIDE;

        virtual sal_Bool        HandleShortCutKey( const KeyEvent& rKeyEvent ) SAL_OVERRIDE; // returns true, if key was handled
    };


}   // namespace bib


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
