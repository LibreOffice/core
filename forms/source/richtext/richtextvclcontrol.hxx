/*************************************************************************
 *
 *  $RCSfile: richtextvclcontrol.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:20:56 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTVCLCONTROL_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTVCLCONTROL_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#include "rtattributes.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX
#include "textattributelistener.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

class EditView;
class EditEngine;
class SfxItemSet;
//........................................................................
namespace frm
{
//........................................................................

    class RichTextControlImpl;
    class RichTextEngine;
    //====================================================================
    //= RichTextControl
    //====================================================================
    class RichTextControl : public Control, public IMultiAttributeDispatcher
    {
    private:
        RichTextControlImpl*    m_pImpl;

    public:
        RichTextControl(
            RichTextEngine* _pEngine,
            Window* _pParent,
            WinBits _nStyle,
            ITextAttributeListener* _pTextAttribListener,
            ITextSelectionListener* _pSelectionListener
        );

        ~RichTextControl( );

        /* enables the change notifications for a particular attribute

           If you want to be notified of any changes in the state of an attribute, you need to call enableAttributeNotification.

           If you provide a dedicated listener for this attribute, this listener is called for every change in the state of
           the attribute.

           No matter whether you provide such a dedicated listener, the "global" listener which you specified
           in the constructor of the control is also called for all changes in the attribute state.

           If you previously already enabled the notification for this attribute, and specified a different listener,
           then the previous listener will be replaced with the new listener, provided the latter is not <NULL/>.
        */
        void        enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener = NULL );

        /** disables the change notifications for a particular attribute

            If there was a listener dedicated to this attribute, it will not be referenced and used anymore
            after this method had been called
        */
        void        disableAttributeNotification( AttributeId _nAttributeId );

        /** determines whether a given slot can be mapped to an aspect of an attribute of the EditEngine

            E.g. SID_ATTR_PARA_ADJUST_LEFT can, though it's not part of the EditEngine pool, be mapped
            to the SID_ATTR_PARA_ADJUST slot, which in fact *is* usable with the EditEngine.
        */
        static bool isMappableSlot( SfxSlotId _nSlotId );
        /// converts an EditEngine-compatible font information to an UNO-compatible font
        static void convert( const SfxPoolItem& _rFontItem, ::com::sun::star::awt::FontDescriptor& _rUnoFont );
        /** converts an UNO-compatible font to an EditEngine-compatible font.

            The caller is responsible for deleting the returned item.
        */
        static void convert( const ::com::sun::star::awt::FontDescriptor& _rUnoFont, SfxPoolItem*& _rpFontItem );
        /// converts an EditEngine-compatible font height to an UNO-compatible information
        static sal_uInt32 convertFontHeight( const SfxPoolItem& _rItem );
        /** converts an UNO-compatible font height to an EditEngine-compatible information

            The caller is responsible for deleting the returned item.
        */
        static SfxPoolItem* convertFontHeight( sal_uInt32 _nUnoFontHeight );

        // IMultiAttributeDispatcher
        virtual AttributeState  getState( AttributeId _nAttributeId ) const;
        virtual void            executeAttribute( AttributeId _nAttributeId, const SfxPoolItem* _pArgument );

        void    SetBackgroundColor( );
        void    SetBackgroundColor( const Color& _rColor );

        void    SetReadOnly( bool _bReadOnly );
        bool    IsReadOnly() const;

        void    SetHideInactiveSelection( bool _bHide );
        bool    GetHideInactiveSelection() const;

        const EditView& getView() const;
              EditView& getView();

    protected:
        // Window overridables
        virtual void        Resize();
        virtual void        GetFocus();
        virtual void        StateChanged( StateChangedType nStateChange );
        virtual long        PreNotify( NotifyEvent& _rNEvt );
        virtual void        Draw( OutputDevice* _pDev, const Point& _rPos, const Size& _rSize, ULONG _nFlags );
        virtual long        Notify( NotifyEvent& _rNEvt );

    private:
                void    applyAttributes( const SfxItemSet& _rAttributesToApply );
                void    implInit( RichTextEngine* _pEngine, ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener );
        static  WinBits implInitStyle( WinBits nStyle );

    private:
        EditEngine&  getEngine() const;
        Window&      getViewport() const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTVCLCONTROL_HXX

