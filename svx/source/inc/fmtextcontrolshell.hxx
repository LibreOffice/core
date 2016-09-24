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
#ifndef INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX
#define INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include "fmslotinvalidator.hxx"

#include <vector>
#include <map>

class SfxRequest;
class SfxItemSet;
class SfxAllItemSet;
class SfxBindings;
class SfxViewFrame;
class SfxApplication;


namespace svx
{


    class FmFocusListenerAdapter;
    class FmTextControlFeature;
    class FmMouseListenerAdapter;

    class IFocusObserver
    {
    public:
        virtual void    focusGained( const css::awt::FocusEvent& _rEvent ) = 0;
        virtual void    focusLost( const css::awt::FocusEvent& _rEvent ) = 0;

    protected:
        ~IFocusObserver() {}
    };


    //= IFocusObserver

    class IContextRequestObserver
    {
    public:
        virtual void    contextMenuRequested( const css::awt::MouseEvent& _rEvent ) = 0;

    protected:
        ~IContextRequestObserver() {}
    };

    class FmTextControlShell :public IFocusObserver
                             ,public ISlotInvalidator
                             ,public IContextRequestObserver
    {
    private:
        css::uno::Reference< css::util::XURLTransformer >             m_xURLTransformer;
        css::uno::Reference< css::awt::XControl >                     m_xActiveControl;
        css::uno::Reference< css::awt::XTextComponent >               m_xActiveTextComponent;
        css::uno::Reference< css::form::runtime::XFormController >    m_xActiveController;
#ifndef DONT_REMEMBER_LAST_CONTROL
        // without this define, m_xActiveControl remembers the *last* active control, even
        // if it, in the meantime, already lost the focus
        bool                                                          m_bActiveControl;
            // so we need an additional boolean flag telling whether the active control
            // is really focused
#endif
        bool                                                          m_bActiveControlIsReadOnly;
        bool                                                          m_bActiveControlIsRichText;

        // listening at all controls of the active controller for focus changes
        typedef rtl::Reference<FmFocusListenerAdapter> FocusListenerAdapter;
        typedef ::std::vector< FocusListenerAdapter >  FocusListenerAdapters;
        FocusListenerAdapters                                         m_aControlObservers;

        typedef rtl::Reference<FmMouseListenerAdapter> MouseListenerAdapter;
        MouseListenerAdapter                                         m_aContextMenuObserver;

        // translating between "slots" of the framework and "features" of the active control
        typedef rtl::Reference<FmTextControlFeature> ControlFeature;
        typedef ::std::map< SfxSlotId, ControlFeature, ::std::less< SfxSlotId > >   ControlFeatures;
        ControlFeatures                                             m_aControlFeatures;

        SfxViewFrame*                                               m_pViewFrame;
        // invalidating slots
        SfxBindings&                                                m_rBindings;
        Link<LinkParamNone*,void>                                   m_aControlActivationHandler;
        AutoTimer                                                   m_aClipboardInvalidation;
        bool                                                        m_bNeedClipboardInvalidation;

    public:
        FmTextControlShell( SfxViewFrame* _pFrame );
        virtual ~FmTextControlShell();

        // clean up any resources associated with this instance
        void    dispose();

        void    ExecuteTextAttribute( SfxRequest& _rReq );
        void    GetTextAttributeState( SfxItemSet& _rSet );
        bool    IsActiveControl( bool _bCountRichTextOnly = false ) const;
        void    ForgetActiveControl();
        void    SetControlActivationHandler( const Link<LinkParamNone*,void>& _rHdl ) { m_aControlActivationHandler = _rHdl; }

        /** to be called when a form in our document has been activated
        */
        void    formActivated( const css::uno::Reference< css::form::runtime::XFormController >& _rxController );
        /** to be called when a form in our document has been deactivated
        */
        void    formDeactivated( const css::uno::Reference< css::form::runtime::XFormController >& _rxController );

        /** notifies the instance that the design mode has changed
        */
        void    designModeChanged( bool _bNewDesignMode );

    protected:
        // IFocusObserver
        virtual void    focusGained( const css::awt::FocusEvent& _rEvent ) override;
        virtual void    focusLost( const css::awt::FocusEvent& _rEvent ) override;

        // IContextRequestObserver
        virtual void    contextMenuRequested( const css::awt::MouseEvent& _rEvent ) override;

        // ISlotInvalidator
        virtual void    Invalidate( SfxSlotId _nSlot ) override;

    protected:
        enum AttributeSet { eCharAttribs, eParaAttribs };
        void    executeAttributeDialog( AttributeSet _eSet, SfxRequest& _rReq );
        void    executeSelectAll( );
        void    executeClipboardSlot( SfxSlotId _nSlot );

    private:
        inline  bool    isControllerListening() const { return !m_aControlObservers.empty(); }

        FmTextControlFeature*
                        implGetFeatureDispatcher(
                            const css::uno::Reference< css::frame::XDispatchProvider >& _rxProvider,
                            SfxApplication* _pApplication,
                            SfxSlotId _nSlot
                        );

        // fills the given structure with dispatchers for the given slots, for the given control
        void            fillFeatureDispatchers(
                            const css::uno::Reference< css::awt::XControl >& _rxControl,
                            SfxSlotId* _pZeroTerminatedSlots,
                            ControlFeatures& _rDispatchers
                        );

        /// creates SfxPoolItes for all features in the given set, and puts them into the given SfxAllItemSet
        static void     transferFeatureStatesToItemSet(
                            ControlFeatures& _rDispatchers,
                            SfxAllItemSet& _rSet,
                            bool _bTranslateLatin
                        );

        /// to be called when a control has been activated
        void    controlActivated( const css::uno::Reference< css::awt::XControl >& _rxControl );
        /// to be called when the currently active control has been deactivated
        void    controlDeactivated( );

        void    implClearActiveControlRef();

        /** starts listening at all controls of the given controller for focus events
        @precond
            we don't have an active controller currently
        */
        void    startControllerListening( const css::uno::Reference< css::form::runtime::XFormController >& _rxController );
        /** stops listening at the active controller
        @precond
            we have an active controller currently
        */
        void    stopControllerListening( );

        /** parses the given URL's Complete member, by calling XURLTransformer::parseString
        */
        void    impl_parseURL_nothrow( css::util::URL& _rURL );

        DECL_LINK_TYPED( OnInvalidateClipboard, Timer*, void );
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
