/*************************************************************************
 *
 *  $RCSfile: richtextimplcontrol.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:19:54 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTIMPLCONTOL_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTIMPLCONTOL_HXX

#ifndef FORMS_SOURCE_COMPONENT_RTATTRIBUTEHANDLER_HXX
#include "rtattributehandler.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX
#include "richtextviewport.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX
#include "richtextengine.hxx"
#endif

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef _MyEDITDATA_HXX
#include <svx/editdata.hxx>
#endif

#include <map>

class EditView;
class EditStatus;
class Window;
class SvxScriptSetItem;
//........................................................................
namespace frm
{
//........................................................................

    class ITextAttributeListener;
    class ITextSelectionListener;
    class RichTextViewPort;
    //====================================================================
    //= RichTextControlImpl
    //====================================================================
    class RichTextControlImpl : public IEngineStatusListener
    {
        typedef ::std::map< AttributeId, AttributeState >                           StateCache;
        typedef ::std::map< AttributeId, ::rtl::Reference< IAttributeHandler > >    AttributeHandlerPool;
        typedef ::std::map< AttributeId, ITextAttributeListener* >                  AttributeListenerPool;

        StateCache              m_aLastKnownStates;
        AttributeHandlerPool    m_aAttributeHandlers;
        AttributeListenerPool   m_aAttributeListeners;

        ESelection              m_aLastKnownSelection;

        Control*                m_pAntiImpl;
        RichTextViewPort*       m_pViewport;
        ScrollBar*              m_pHScroll;
        ScrollBar*              m_pVScroll;
        ScrollBarBox*           m_pScrollCorner;
        RichTextEngine*         m_pEngine;
        EditView*               m_pView;
        ITextAttributeListener* m_pTextAttrListener;
        ITextSelectionListener* m_pSelectionListener;
        bool                    m_bLockedPaperSize;
        bool                    m_bHasEverBeenShown;

    public:
        struct GrantAccess { friend class RichTextControl; private: GrantAccess() { } };
        inline EditView*        getView( const GrantAccess& ) const     { return m_pView; }
        inline RichTextEngine*  getEngine( const GrantAccess& ) const   { return m_pEngine; }
        inline Window*          getViewport( const GrantAccess& ) const { return m_pViewport; }

    public:
        RichTextControlImpl( Control* _pAntiImpl, RichTextEngine* _pEngine,
            ITextAttributeListener* _pTextAttrListener, ITextSelectionListener* _pSelectionListener );
        ~RichTextControlImpl();

        /** updates the cache with the state of all attribute values from the given set, notifies
            the listener if the state changed
        */
        void    updateAllAttributes( );

        /** updates the cache with the state of the attribute given by which id, notifies
            the listener if the state changed
        */
        void    updateAttribute( AttributeId _nAttribute );

        /// enables the callback for a particular attribute
        void    enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener = NULL );

        /// disables the change notifications for a particular attribute
        void    disableAttributeNotification( AttributeId _nAttributeId );

        /// executes a toggle of the given attribute
        bool    executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, AttributeId _nAttribute, const SfxPoolItem* _pArgument, ScriptType _nForScriptType );

        /// retrieves the state of the given attribute from the cache
        AttributeState  getAttributeState( AttributeId _nAttributeId ) const;

        /** normalizes the given item so that the state of script dependent attributes
            is correct considering the current script type

            There are some attributes which are script dependent, e.g. the CharPosture. This means
            that in real, there are 3 attributes for this, one for every possible script type (latin,
            asian, complex). However, to the out world, we behave as if there is only one attribute:
            E.g., if the outter world asks for the state of the "CharPosture" attribute, we return
            the state of either CharPostureLatin, CharPostureAsian, or CharPostureComplex, depending
            on the script type of the current selection. (In real, it may be more complex since
            the current selection may contain more than one script type.)

            This method normalizes a script dependent attribute, so that it's state takes into account
            the currently selected script type.
        */
        void        normalizeScriptDependentAttribute( SvxScriptSetItem& _rScriptSetItem );

        // gets the script type of the selection in our edit view (with fallback)
        ScriptType  getSelectedScriptType() const;

        /** re-arranges the view and the scrollbars
        */
        void    layoutWindow();

        /** to be called when the style of our window changed
        */
        void    notifyStyleChanged();

        /** to be called when the zoom of our window changed
        */
        void    notifyZoomChanged();

        /** to be called when the STATE_CHANGE_INITSHOW event arrives
        */
        void    notifyInitShow();

        // VCL "overrides"
        void    SetBackgroundColor( );
        void    SetBackgroundColor( const Color& _rColor );

        void    SetReadOnly( bool _bReadOnly );
        bool    IsReadOnly() const;

        void    SetHideInactiveSelection( bool _bHide );
        bool    GetHideInactiveSelection() const;

        /// draws the control onto a given output device
        void    Draw( OutputDevice* _pDev, const Point& _rPos, const Size& _rSize, ULONG _nFlags );

        /// handles command events arrived at the anti-impl control
        long    HandleCommand( const CommandEvent& _rEvent );

    private:
        // updates the cache with the state provided by the given attribut handler
        void    implUpdateAttribute( AttributeHandlerPool::const_iterator _pHandler );

        // updates the cache with the given state, and calls listeners (if necessary)
        void    implCheckUpdateCache( AttributeId _nAttribute, const AttributeState& _rState );

        // updates range and position of our scrollbars
        void    updateScrollbars();

        // determines whether automatic (soft) line breaks are ON
        bool    windowHasAutomaticLineBreak();

        /// hides or shows our scrollbars, according to the current WinBits of the window
        void    ensureScrollbars();

        /// ensures that our "automatic line break" setting matches the current WinBits of the window
        void    ensureLineBreakSetting();

        inline  bool    hasVScrollBar( ) const { return m_pVScroll != NULL; }
        inline  bool    hasHScrollBar( ) const { return m_pHScroll != NULL; }

        // IEngineStatusListener overridables
        virtual void EditEngineStatusChanged( const EditStatus& _rStatus );

    private:
        DECL_LINK( OnInvalidateAllAttributes, void* );
        DECL_LINK( OnHScroll, ScrollBar* );
        DECL_LINK( OnVScroll, ScrollBar* );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTIMPLCONTOL_HXX

