/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX
#define SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
/** === end UNO includes === **/
#include <comphelper/implementationreference.hxx>
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
class Window;
class SfxApplication;

//........................................................................
namespace svx
{
//........................................................................

    class FmFocusListenerAdapter;
    class FmTextControlFeature;
    class FmMouseListenerAdapter;

    //====================================================================
    //= IFocusObserver
    //====================================================================
    class IFocusObserver
    {
    public:
        virtual void    focusGained( const ::com::sun::star::awt::FocusEvent& _rEvent ) = 0;
        virtual void    focusLost( const ::com::sun::star::awt::FocusEvent& _rEvent ) = 0;
    };

    //====================================================================
    //= IFocusObserver
    //====================================================================
    class IContextRequestObserver
    {
    public:
        virtual void    contextMenuRequested( const ::com::sun::star::awt::MouseEvent& _rEvent ) = 0;
    };

    //====================================================================
    //= FmTextControlShell
    //====================================================================
    class FmTextControlShell :public IFocusObserver
                             ,public ISlotInvalidator
                             ,public IContextRequestObserver
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >             m_xURLTransformer;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >                     m_xActiveControl;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >               m_xActiveTextComponent;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xActiveController;
#ifndef DONT_REMEMBER_LAST_CONTROL
        // without this define, m_xActiveControl remembers the *last* active control, even
        // if it, in the meantime, already lost the focus
        bool                                                                        m_bActiveControl;
            // so we need an additional boolean flag telling whether the active cotrol
            // is really focused
#endif
        bool                                                                        m_bActiveControlIsReadOnly;
        bool                                                                        m_bActiveControlIsRichText;

        // listening at all controls of the active controller for focus changes
        typedef ::comphelper::ImplementationReference< FmFocusListenerAdapter, ::com::sun::star::awt::XFocusListener >
                                                                                    FocusListenerAdapter;
        typedef ::std::vector< FocusListenerAdapter >                               FocusListenerAdapters;
        FocusListenerAdapters                                                       m_aControlObservers;

        typedef ::comphelper::ImplementationReference< FmMouseListenerAdapter, ::com::sun::star::awt::XMouseListener >
                                                                                    MouseListenerAdapter;
        MouseListenerAdapter                                                        m_aContextMenuObserver;

        // translating between "slots" of the framework and "features" of the active control
        typedef ::comphelper::ImplementationReference< FmTextControlFeature, ::com::sun::star::frame::XStatusListener >
                                                                                    ControlFeature;
        typedef ::std::map< SfxSlotId, ControlFeature, ::std::less< SfxSlotId > >   ControlFeatures;
        ControlFeatures                                                             m_aControlFeatures;

        SfxViewFrame*                                                               m_pViewFrame;
        // invalidating slots
        SfxBindings&                                                                m_rBindings;
        Link                                                                        m_aControlActivationHandler;
        AutoTimer                                                                   m_aClipboardInvalidation;
        bool                                                                        m_bNeedClipboardInvalidation;

    public:
        FmTextControlShell( SfxViewFrame* _pFrame );
        virtual ~FmTextControlShell();

        // clean up any resources associated with this instance
        void    dispose();

        void    ExecuteTextAttribute( SfxRequest& _rReq );
        void    GetTextAttributeState( SfxItemSet& _rSet );
        bool    IsActiveControl( bool _bCountRichTextOnly = false ) const;
        void    ForgetActiveControl();
        void    SetControlActivationHandler( const Link& _rHdl ) { m_aControlActivationHandler = _rHdl; }

        /** to be called when a form in our document has been activated
        */
        void    formActivated( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController );
        /** to be called when a form in our document has been deactivated
        */
        void    formDeactivated( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController );

        /** notifies the instance that the design mode has changed
        */
        void    designModeChanged( bool _bNewDesignMode );

    protected:
        // IFocusObserver
        virtual void    focusGained( const ::com::sun::star::awt::FocusEvent& _rEvent );
        virtual void    focusLost( const ::com::sun::star::awt::FocusEvent& _rEvent );

        // IContextRequestObserver
        virtual void    contextMenuRequested( const ::com::sun::star::awt::MouseEvent& _rEvent );

        // ISlotInvalidator
        virtual void    Invalidate( SfxSlotId _nSlot );

    protected:
        enum AttributeSet { eCharAttribs, eParaAttribs };
        void    executeAttributeDialog( AttributeSet _eSet, SfxRequest& _rReq );
        bool    executeSelectAll( );
        bool    executeClipboardSlot( SfxSlotId _nSlot );

    private:
        inline  bool    isControllerListening() const { return !m_aControlObservers.empty(); }

        FmTextControlFeature*
                        implGetFeatureDispatcher(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& _rxProvider,
                            SfxApplication* _pApplication,
                            SfxSlotId _nSlot
                        );

        // fills the given structure with dispatchers for the given slots, for the given control
        void            fillFeatureDispatchers(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _rxControl,
                            SfxSlotId* _pZeroTerminatedSlots,
                            ControlFeatures& _rDispatchers
                        );

        /// creates SfxPoolItes for all features in the given set, and puts them into the given SfxAllItemSet
        void            transferFeatureStatesToItemSet(
                            ControlFeatures& _rDispatchers,
                            SfxAllItemSet& _rSet,
                            bool _bTranslateLatin = false
                        );

        /// to be called when a control has been activated
        void    controlActivated( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
        /// to be called when the currently active control has been deactivated
        void    controlDeactivated( );

        void    implClearActiveControlRef();

        /** starts listening at all controls of the given controller for focus events
        @precond
            we don't have an active controller currently
        */
        void    startControllerListening( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _rxController );
        /** stops listening at the active controller
        @precond
            we have an active controller currently
        */
        void    stopControllerListening( );

        /** parses the given URL's Complete member, by calling XURLTransformer::parseString
        */
        void    impl_parseURL_nothrow( ::com::sun::star::util::URL& _rURL );

        DECL_LINK( OnInvalidateClipboard, void* );
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
