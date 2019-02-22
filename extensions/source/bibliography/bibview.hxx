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
#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBVIEW_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBVIEW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include "formcontrolcontainer.hxx"
#include "bibshortcuthandler.hxx"

class   BibGeneralPage;
class   BibDataManager;

namespace com{ namespace sun{ namespace star{ namespace awt{ class XFocusListener;}}}}


namespace bib
{


    class BibView;
    class BibViewFormControlContainer : public FormControlContainer
    {
    private:
        VclPtr<BibView> mpBibView;
    protected:
        // FormControlContainer
        virtual css::uno::Reference< css::awt::XControlContainer >
                            getControlContainer() override;
        // XLoadListener equivalents
        virtual void        _loaded( const css::lang::EventObject& _rEvent ) override;
        virtual void        _reloaded( const css::lang::EventObject& _rEvent ) override;
    public:
        using FormControlContainer::connectForm;
        using FormControlContainer::disconnectForm;
        using FormControlContainer::isFormConnected;
        explicit BibViewFormControlContainer(BibView *pBibView);
    };

    class BibView : public BibWindow
    {
    private:
        BibDataManager*                                   m_pDatMan;
        css::uno::Reference< css::form::XLoadable>        m_xDatMan;
        VclPtr<BibGeneralPage>                            m_pGeneralPage;
        BibViewFormControlContainer                       m_aFormControlContainer;

    private:
        DECL_LINK(CallMappingHdl, void*, void);

    public:
        // Window overridables
        virtual void    Resize() override;

    public:
                            BibView( vcl::Window* _pParent, BibDataManager* _pDatMan, WinBits nStyle );
                            virtual ~BibView() override;
        virtual void        dispose() override;

        void                UpdatePages();
        css::uno::Reference< css::awt::XControlContainer > getControlContainer();

        virtual void        GetFocus() override;

        virtual bool        HandleShortCutKey( const KeyEvent& rKeyEvent ) override; // returns true, if key was handled
    };


}   // namespace bib


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
