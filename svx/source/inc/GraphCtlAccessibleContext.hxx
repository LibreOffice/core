/*************************************************************************
 *
 *  $RCSfile: GraphCtlAccessibleContext.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:58:44 $
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


#ifndef _SVXGRAPHACCESSIBLECONTEXT_HXX
#define _SVXGRAPHACCESSIBLECONTEXT_HXX

#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_IllegalAccessibleComponentStateException_HPP_
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#include <set>
#include <map>

#include <unotools/servicehelper.hxx>

#ifndef _SVX_RECTENUM_HXX //autogen
#include <rectenum.hxx>
#endif

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TREE_INFO_HXX
#include <AccessibleShapeTreeInfo.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_IVIEW_FORWARDER_HXX
#include <IAccessibleViewForwarder.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <AccessibleShape.hxx>
#endif

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } };

class Rectangle;
class GraphCtrl;
class Window;
class SdrObject;
class SdrModel;
class SdrPage;
class SdrView;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef ::cppu::WeakAggComponentImplHelper7<
                ::com::sun::star::accessibility::XAccessible,
                ::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::accessibility::XAccessibleContext,
                ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::com::sun::star::accessibility::XAccessibleSelection,
                ::com::sun::star::lang::XServiceInfo,
                ::com::sun::star::lang::XServiceName >
                SvxGraphCtrlAccessibleContext_Base;

class SvxGraphCtrlAccessibleContext : public SvxGraphCtrlAccessibleContext_Base, ::comphelper::OBaseMutex, SfxListener, ::accessibility::IAccessibleViewForwarder
{
public:
    friend class GraphCtrl;

    //=====  internal  ========================================================
    SvxGraphCtrlAccessibleContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent,
        GraphCtrl&              rRepresentation,
        const ::rtl::OUString*  pName = NULL,
        const ::rtl::OUString*  pDescription = NULL );

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint );

protected:
    virtual ~SvxGraphCtrlAccessibleContext();
public:
    //=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )    throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding() throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XAccessibleContext  ==============================================

    virtual long SAL_CALL getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleChild (long nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleParent (void) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent (void) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet (void) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale (void) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::accessibility::IllegalAccessibleComponentStateException);
    virtual void SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) {}

    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener>& xListener) throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(  const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener>& xListener) throw (com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    virtual ::rtl::OUString SAL_CALL getImplementationName (void) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService (const ::rtl::OUString& sServiceName) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceName  ====================================================

    virtual ::rtl::OUString SAL_CALL getServiceName (void) throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleSelection =============================================

    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL clearAccessibleSelection() throw (::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL selectAllAccessible() throw (::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    //=====  IAccessibleViewforwarder  ========================================

    virtual BOOL IsValid (void) const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel (const Point& rPoint) const;
    virtual Size LogicToPixel (const Size& rSize) const;
    virtual Point PixelToLogic (const Point& rPoint) const;
    virtual Size PixelToLogic (const Size& rSize) const;

protected:
    //=====  internals ========================================================

    void checkChildIndex( long nIndexOfChild ) throw (::com::sun::star::lang::IndexOutOfBoundsException );
    void checkChildIndexOnSelection( long nIndexOfChild ) throw (::com::sun::star::lang::IndexOutOfBoundsException );

public:

    /// Sets the name
    void setName( const ::rtl::OUString& rName );

    /// Sets the description
    void setDescription( const ::rtl::OUString& rDescr );
private:
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUniqueId( void );
protected:

    /** Return the object's current bounding box relative to the desktop,
        i.e in absolute pixel coordinates.
        @return
            The returned rectangle is a bounding box of the object given in
            absolute screen coordinates.
        @raise DisposedException
            When the object is already disposed then a
            <type>DisposedException</type> is thrown.
    */
     virtual Rectangle GetBoundingBoxOnScreen(void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing();

private:
    SdrObject* getSdrObject( sal_Int32 nIndex )
        throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );

    void CommitChange (sal_Int16 aEventId, const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue);
    void FireEvent (const ::com::sun::star::accessibility::AccessibleEventObject& aEvent);

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessible( const SdrObject* pObj );

    accessibility::AccessibleShapeTreeInfo maTreeInfo;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::com::sun::star::accessibility::XAccessible> mxParent;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    ::rtl::OUString msDescription;

    /** Name of this object.
    */
    ::rtl::OUString msName;

    /// map of accessible shapes
    struct SdrObjectCompareLess
    {
        bool operator()(const SdrObject* p1, const SdrObject* p2) const
        {
            return p1 < p2;
        }
    };
    typedef ::std::map< const SdrObject*, ::accessibility::AccessibleShape*, SdrObjectCompareLess > ShapesMapType;
    ShapesMapType mxShapes;

    GraphCtrl*  mpControl;

    SdrModel* mpModel;
    SdrPage* mpPage;
    SdrView* mpView;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    sal_Bool mbDisposed;
};

#endif

