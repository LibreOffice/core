/*************************************************************************
 *
 *  $RCSfile: acccontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2002-02-05 15:52:06 $
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
#ifndef _ACCBASE_HXX
#define _ACCBASE_HXX

#ifndef _ACCFRAME_HXX
#include <accframe.hxx>
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
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ILLEGALACCESSIBLECOMPONENTSTATEEXCEPTION_HDL_
#include <drafts/com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

namespace utl { class AccessibleStateSetHelper; };

class SwAccessibleContext :
    public ::cppu::WeakImplHelper4<
                ::drafts::com::sun::star::accessibility::XAccessible,
                ::drafts::com::sun::star::accessibility::XAccessibleContext,
                ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::lang::XServiceInfo
                >,
    public SwAccessibleFrame
{
    ::rtl::OUString sName;
    ::osl::Mutex aMutex;
    ::cppu::OInterfaceContainerHelper aPropChangedListeners;
    ::cppu::OInterfaceContainerHelper aFocusListeners;

    sal_Int16 nRole;

protected:
    void SetName( const ::rtl::OUString& rName ) { sName = rName; }

    // A child has been added while setting the vis area
    virtual void LowerAdded( const SwFrm *pFrm );

    // A child has been removed while setting the vis area
    virtual void LowerRemoved( const SwFrm *pFrm );

    // A child has been moved while setting the vis area
    virtual void LowerMoved( const SwFrm *pFrm );

    // The object is not visible an longer and should be destroyed
    void Dispose();

    void PropertyChanged( ::com::sun::star::beans::PropertyChangeEvent& rEvent );

    ::rtl::OUString GetResource( sal_uInt16 nResId,
                                 const ::rtl::OUString *pArg1 = 0,
                                 const ::rtl::OUString *pArg2 = 0 ) const;

    // Set states for getAccessibleStateSet.
    // This base class sets DEFUNC(0/1), EDITABLE(0/1), ENABLED(1),
    // SHOWING(0/1), OPAQUE(0/1) and VISIBLE(1).
    virtual void SetStates( ::utl::AccessibleStateSetHelper& rStateSet );

public:

    SwAccessibleContext( sal_Int16 nRole, const Rectangle& rVisArea,
                         const SwFrm *pFrm );
    SwAccessibleContext( const ::rtl::OUString& rName, sal_Int16 nRole,
                         const Rectangle& rVisArea, const SwFrm *pFrm );
    virtual ~SwAccessibleContext();

    //=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual long SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale (void)
        throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    /** Add listener that is informed of future changes of name and
          description properties.
    */
    virtual void SAL_CALL
        addPropertyChangeListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //  Remove an existing property change listener.
    virtual void SAL_CALL
        removePropertyChangeListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ==============================================
    virtual sal_Bool SAL_CALL contains(
            const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAt(
                const ::com::sun::star::awt::Point& aPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isShowing()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVisible()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isFocusTraversable()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addFocusListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeFocusListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding()
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
             throw (::com::sun::star::uno::RuntimeException);
#ifdef DEBUG
#define DBG_MSG( a ) DbgMsg( a, 0 )
#define DBG_MSG2( a, b ) DbgMsg( a, b )
    void DbgMsg( const char *pName, const SwAccessibleContext *pAcc=0 );
#else
#define DBG_MSG( a )
#define DBG_MSG2( a, b )
#endif
};

// some heaviliy used exception support
const sal_Char sDefunc[] = "object is defunctional";
const sal_Char sMissingWindow[] = "window is missing";

#define THROW_RUNTIME_EXCEPTION( ifc, msg )                                 \
    Reference < ifc > xThis( this );                                        \
    RuntimeException aExcept( OUString( RTL_CONSTASCII_USTRINGPARAM(msg) ), \
                              xThis );                                      \
    throw aExcept;

#define CHECK_FOR_DEFUNC( ifc )                                             \
    if( !GetFrm() )                                                         \
    {                                                                       \
        THROW_RUNTIME_EXCEPTION( ifc, sDefunc );                            \
    }

#define CHECK_FOR_WINDOW( i, w )                                            \
    if( !(w) )                                                              \
    {                                                                       \
        THROW_RUNTIME_EXCEPTION( i, sMissingWindow );                       \
    }


#endif

