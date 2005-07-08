/*************************************************************************
 *
 *  $RCSfile: fmtextcontrolshell.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:34:20 $
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
#ifndef SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX
#define SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef SVX_SOURCE_INC_FMSLOTINVALIDATOR_HXX
#include "fmslotinvalidator.hxx"
#endif

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
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >         m_xActiveControl;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >   m_xActiveTextComponent;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > m_xActiveController;
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
        ~FmTextControlShell();

        // clean up any resources associated with this instance
        void    dispose();

        void    ExecuteTextAttribute( SfxRequest& _rReq );
        void    GetTextAttributeState( SfxItemSet& _rSet );
        bool    IsActiveControl( bool _bCountRichTextOnly = false ) const;
        void    ForgetActiveControl();
        void    SetControlActivationHandler( const Link& _rHdl ) { m_aControlActivationHandler = _rHdl; }

        /** to be called when a form in our document has been activated
        */
        void    formActivated( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController );
        /** to be called when a form in our document has been deactivated
        */
        void    formDeactivated( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController );

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
        void    startControllerListening( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController );
        /** stops listening at the active controller
        @precond
            we have an active controller currently
        */
        void    stopControllerListening( );

        DECL_LINK( OnInvalidateClipboard, void* );
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX

