/*************************************************************************
 *
 *  $RCSfile: AccessibleEditableTextPara.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:24 $
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

#ifndef _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX
#define _SVX_ACCESSIBLE_EDITABLE_TEXT_PARA_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleContext.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEDITABLETEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEditableText.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
#endif

#ifndef _SVX_ACCESSIBLE_PARA_MANAGER_HXX
#include "AccessibleParaManager.hxx"
#endif

#ifndef _SVX_ACCESSIBLE_IMAGE_BULLET_HXX
#include "AccessibleImageBullet.hxx"
#endif

#ifndef _SVX_UNOEDPRX_HXX
#include "unoedprx.hxx"
#endif


namespace accessibility
{
    typedef ::cppu::WeakImplHelper6< ::drafts::com::sun::star::accessibility::XAccessible,
                                     ::drafts::com::sun::star::accessibility::XAccessibleContext,
                                     ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                                     ::drafts::com::sun::star::accessibility::XAccessibleEditableText,
                                     ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                                     ::com::sun::star::lang::XServiceInfo >  AccessibleTextParaInterfaceBase;

    /** This class implements the actual text paragraphs for the EditEngine/Outliner UAA
     */
    class AccessibleEditableTextPara : public AccessibleTextParaInterfaceBase, public ::comphelper::OCommonAccessibleText
    {

    protected:
        // override OCommonAccessibleText methods
        virtual ::rtl::OUString                 implGetText();
        virtual ::com::sun::star::lang::Locale  implGetLocale();
        virtual void                            implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );
        virtual void                            implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );
        virtual void                            implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex );

    public:
        /// Create accessible object for given parent
        AccessibleEditableTextPara ( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& rParent );

        virtual ~AccessibleEditableTextPara ();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface (const ::com::sun::star::uno::Type & rType) throw (::com::sun::star::uno::RuntimeException);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole() throw (::com::sun::star::uno::RuntimeException);
        /// Maximal length of text returned by getAccessibleDescription()
        enum { MaxDescriptionLen = 40 };
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale() throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleComponent
        virtual sal_Bool SAL_CALL contains( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAt( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleText (this comes implicitely inherited by XAccessibleEditableText)
        virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        /// Does not support AccessibleTextType::SENTENCE (missing feature in EditEngine)
        virtual ::rtl::OUString SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleEditableText
        virtual sal_Bool SAL_CALL cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL pasteText( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aAttributeSet ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setText( const ::rtl::OUString& sText ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName (void) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService (const ::rtl::OUString& sServiceName) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void) throw (::com::sun::star::uno::RuntimeException);

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName (void) throw (::com::sun::star::uno::RuntimeException);

        /** Set the current index in the accessibility parent

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetIndexInParent( sal_Int32 nIndex );

        /** Get the current index in the accessibility parent

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Int32 GetIndexInParent() const;

        /** Set the current paragraph number

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetParagraphIndex( sal_Int32 nIndex );

        /** Query the current paragraph number (0 - nParas-1)

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Int32 GetParagraphIndex() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Set the edit engine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEEOffset( const Point& rOffset );

        /** Set the EditEngine offset

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        void SetEditSource( SvxEditSourceAdapter* pEditSource );

        /** Dispose this object

            Notifies and deregisters the listeners, drops all references.
         */
        void Dispose();

        /// Calls all Listener objects to tell them the change. Don't hold locks when calling this!
        virtual void FireEvent(const sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue = ::com::sun::star::uno::Any(), const ::com::sun::star::uno::Any& rOldValue = ::com::sun::star::uno::Any() ) const;

        /// Queries the given state on the internal state set
        bool HasState( const sal_Int16 nStateId );
        /// Sets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void SetState( const sal_Int16 nStateId );
        /// Unsets the given state on the internal state set and fires STATE_CHANGE event. Don't hold locks when calling this!
        void UnSetState( const sal_Int16 nStateId );

        static Rectangle LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder );

        SvxEditSourceAdapter& GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the SvxTextForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextAdapter&   GetTextForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the SvxViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxViewForwarder&   GetViewForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query whether a GetEditViewForwarder( sal_False ) will return a forwarder

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        sal_Bool    HaveEditView() const;

        /** Query the SvxEditViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxAccessibleTextEditViewAdapter& GetEditViewForwarder( sal_Bool bCreate = sal_False ) const SAL_THROW((::com::sun::star::uno::RuntimeException));

    private:

        // declared, but not defined
        AccessibleEditableTextPara( const AccessibleEditableTextPara& );
        AccessibleEditableTextPara& operator= ( const AccessibleEditableTextPara& );

        /** Calculate character range of similar attributes

            @param nStartIndex
            Therein, the start of the character range with the same attributes is returned

            @param nEndIndex
            Therein, the end (exclusively) of the character range with the same attributes is returned

            @param nIndex
            The character index at where to look for similar character attributes

            @return sal_False, if the method was not able to determine the range
         */
        sal_Bool GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, sal_Int32 nIndex );

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const ::com::sun::star::uno::Any& rNewValue, const sal_Int16 nEventId ) const;
        void LostPropertyEvent( const ::com::sun::star::uno::Any& rOldValue, const sal_Int16 nEventId ) const;

        /** Query the visibility state

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.

            @return the visibility state. Per definition, a defunc object is no longer visible
         */
        sal_Bool IsVisible() const;

        int getNotifierClientId() const;

        // retrieve text interface for given paragraph index
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleText > GetParaInterface( sal_Int32 nIndex );

        /// Do we have children? This is the case for image bullets
        sal_Bool HaveChildren();

        /// Is the underlying object in edit mode
        sal_Bool IsActive() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        const Point& GetEEOffset() const;

        // Get text from forwarder
        String GetText( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        String GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        USHORT GetTextLen() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Get the current selection of this paragraph

            @return sal_False, if nothing in this paragraph is selected
         */
        sal_Bool GetSelection( USHORT& nStartPos, USHORT& nEndPos ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** create selection from Accessible selection.

        */
        ESelection  MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex );
        ESelection  MakeSelection( sal_Int32 nEEIndex );
        ESelection  MakeCursor( sal_Int32 nEEIndex );

        // check whether index value is within permitted range

        /// Check whether 0<=nIndex<=n-1
        void CheckIndex( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));
        /// Check whether 0<=nIndex<=n
        void CheckPosition( sal_Int32 nIndex ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));
        /// Check whether 0<=nStart<=n and 0<=nEnd<=n
        void CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) SAL_THROW((::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException));

        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSourceAdapter* mpEditSource;

        // the possible child (for image bullets, guarded by solar mutex)
        typedef WeakCppRef < ::drafts::com::sun::star::accessibility::XAccessible, AccessibleImageBullet > WeakBullet;
        WeakBullet maImageBullet;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

        /// The shape we're the accessible for (unguarded)
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;
    };

} // end of namespace accessibility

#endif

