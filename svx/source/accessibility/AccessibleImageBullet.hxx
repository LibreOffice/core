/*************************************************************************
 *
 *  $RCSfile: AccessibleImageBullet.hxx,v $
 *
 *  $Revision: 1.8 $
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

#ifndef _SVX_ACCESSIBLE_IMAGE_BULLET_HXX
#define _SVX_ACCESSIBLE_IMAGE_BULLET_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
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

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif

#ifndef _SVX_ACCESSIBLE_PARA_MANAGER_HXX
#include "AccessibleParaManager.hxx"
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include "unoedsrc.hxx"
#endif

namespace accessibility
{
    typedef ::cppu::WeakImplHelper5< ::drafts::com::sun::star::accessibility::XAccessible,
                                     ::drafts::com::sun::star::accessibility::XAccessibleContext,
                                     ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                                     ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                                     ::com::sun::star::lang::XServiceInfo >  AccessibleImageBulletInterfaceBase;

    /** This class implements the image bullets for the EditEngine/Outliner UAA
     */
    class AccessibleImageBullet : public AccessibleImageBulletInterfaceBase
    {

    public:
        /// Create accessible object for given parent
        AccessibleImageBullet ( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& rParent );

        virtual ~AccessibleImageBullet  ();

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
        void SetEditSource( SvxEditSource* pEditSource );

        /** Dispose this object

            Notifies and deregisters the listeners, drops all references.
         */
        void Dispose();

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

        /// Calls all Listener objects to tell them the change. Don't hold locks when calling this!
        virtual void FireEvent(const sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue = ::com::sun::star::uno::Any(), const ::com::sun::star::uno::Any& rOldValue = ::com::sun::star::uno::Any() ) const;

    private:

        // declared, but not defined
        AccessibleImageBullet( const AccessibleImageBullet& );
        AccessibleImageBullet& operator= ( const AccessibleImageBullet& );

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const ::com::sun::star::uno::Any& rNewValue, const sal_Int16 nEventId ) const;
        void LostPropertyEvent( const ::com::sun::star::uno::Any& rOldValue, const sal_Int16 nEventId ) const;

        // maintain state set and send STATE_CHANGE events
        void SetState( const sal_Int16 nStateId );
        void UnSetState( const sal_Int16 nStateId );

        SvxEditSource& GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        int getNotifierClientId() const;

        /** Query the SvxTextForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxTextForwarder&   GetTextForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** Query the SvxViewForwarder for EditEngine access.

            @attention This method does not lock the SolarMutex,
            leaving that to the calling code. This is because only
            there potential deadlock situations can be resolved. Thus,
            make sure SolarMutex is locked when calling this.
         */
        SvxViewForwarder&   GetViewForwarder() const SAL_THROW((::com::sun::star::uno::RuntimeException));

        const Point& GetEEOffset() const;

        // the paragraph index in the edit engine (guarded by solar mutex)
        sal_Int32   mnParagraphIndex;

        // our current index in the parent (guarded by solar mutex)
        sal_Int32   mnIndexInParent;

        // the current edit source (guarded by solar mutex)
        SvxEditSource* mpEditSource;

        // the offset of the underlying EditEngine from the shape/cell (guarded by solar mutex)
        Point maEEOffset;

        // the current state set (updated from SetState/UnSetState and guarded by solar mutex)
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

        mutable osl::Mutex  maMutex;

        /// The shape we're the accessible for (unguarded)
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > mxParent;

        /// Our listeners (guarded by maMutex)
        int mnNotifierClientId;
    };

} // end of namespace accessibility

#endif

