/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef _SVXPIXELACCESSIBLECONTEXT_HXX
#define _SVXPIXELACCESSIBLECONTEXT_HXX

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
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ILLEGALACCESSIBLECOMPONENTSTATEEXCEPTION_HPP_
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
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
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#include <comphelper/servicehelper.hxx>

//#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
//#include <com/sun/star/accessibility/XAccessibleTable.hpp>
//#endif

#ifndef _SVX_RECTENUM_HXX //autogen
#include <rectenum.hxx>
#endif


#include <tools/gen.hxx>


namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } };
class SvxPixelCtl;


typedef ::cppu::WeakAggComponentImplHelper5<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::lang::XServiceInfo >
            SvxPixelCtlAccessibleChild_BASE;

class SvxPixelCtlAccessibleChild :
    public ::comphelper::OBaseMutex,
    public SvxPixelCtlAccessibleChild_BASE
{
    SvxPixelCtl* mrParentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxParent;
       void IsValid() throw (::com::sun::star::uno::RuntimeException);
    sal_Bool m_bPixelColorOrBG;//Pixel Color Or BackGround Color
    Point maPoint;
    Rectangle*  mpBoundingBox;
    /// index of child in parent
    long                                mnIndexInParent;
    /// Mutex guarding this object.
    ::osl::Mutex                        m_aMutex;
public:
    SvxPixelCtlAccessibleChild(
                SvxPixelCtl* rWindow,
                sal_Bool bPixelColorOrBG,
                const Point& aPoint,
                const Rectangle& rBounds,
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xParent,
                long nIndexInParent );
    ~SvxPixelCtlAccessibleChild();

    //XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );
    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );

    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const ::com::sun::star::uno::Any& rOld, const ::com::sun::star::uno::Any& rNew);
    virtual void SAL_CALL disposing();

    virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::com::sun::star::uno::RuntimeException );

    virtual Rectangle GetBoundingBox( void ) throw( ::com::sun::star::uno::RuntimeException );

    /// @returns true if it's disposed or in disposing
    inline sal_Bool IsAlive( void ) const;
    /// @returns true if it's not disposed and no in disposing
    inline sal_Bool IsNotAlive( void ) const;
    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );


    void CheckChild();
    void SelectChild( sal_Bool bSelect);
    void ChangePixelColorOrBG(sal_Bool bPixelColorOrBG){ m_bPixelColorOrBG = bPixelColorOrBG ;}
    ::rtl::OUString GetName();
};


typedef ::cppu::WeakAggComponentImplHelper6<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::accessibility::XAccessibleSelection,
//          ::com::sun::star::accessibility::XAccessibleTable,
            ::com::sun::star::lang::XServiceInfo >
            SvxPixelCtlAccessible_BASE;

class SvxPixelCtlAccessible :
    public ::comphelper::OBaseMutex,
    public SvxPixelCtlAccessible_BASE
{
    SvxPixelCtl* pPixelCtl;
    void IsValid() throw (::com::sun::star::uno::RuntimeException);

public:
    SvxPixelCtlAccessible(SvxPixelCtl& rWindow);
    ~SvxPixelCtlAccessible();

    void ensureIsAlive() const throw ( ::com::sun::star::lang::DisposedException );

    //XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);


    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

/*
    // XAccessibleTable -------------------------------------------------------
    /-** @return  The number of used rows in the table (0 = empty table). *-/
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  The number of used columns in the table (0 = empty table). *-/
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  The row extent of the specified cell (always 1). *-/
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    /-** @return  The column extent of the specified cell (always 1). *-/
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    /-** @return  The caption cell of the table (not supported). *-/
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleCaption() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  The summary object of the table (not supported). *-/
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleSummary() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  The child index of the specified cell. *-/
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  The row index of the specified child cell. *-/
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  The column index of the specified child cell. *-/
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  The description text of the specified row. *-/
    virtual ::rtl::OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  The description text of the specified column. *-/
    virtual ::rtl::OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  The XAccessibleTable interface of the row header bar. *-/
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
        getAccessibleRowHeaders() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  The XAccessibleTable interface of the column header bar. *-/
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
        getAccessibleColumnHeaders() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  An index list of completely selected rows. *-/
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleRows() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  An index list of completely selected columns. *-/
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleColumns() throw ( ::com::sun::star::uno::RuntimeException );
    /-** @return  <TRUE/>, if the specified row is completely selected. *-/
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return  <TRUE/>, if the specified column is completely selected. *-/
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );
    /-** @return The XAccessible interface of the cell object at the specified cell position. *-/
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    /-** @return  <TRUE/>, if the specified cell is selected. *-/
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );

*/
    //Methods for XAccessibleEventBroadcaster
    sal_uInt32 mnClientId;

    virtual void SAL_CALL
        addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
        removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );
    void CommitChange( const com::sun::star::accessibility::AccessibleEventObject& rEvent );
    //Solution: Add the event handling method
    void FireAccessibleEvent (short nEventId, const ::com::sun::star::uno::Any& rOld, const ::com::sun::star::uno::Any& rNew);
    virtual void SAL_CALL disposing();

    /// select child
    void SelectChild( Point pos );
    void SelectChild( long nIndex);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
         CreateChild (long nIndex, Point mPoint);


    void LoseFocus();

    void Invalidate();
    /// @returns true if it's disposed or in disposing
    inline sal_Bool IsAlive( void ) const;
    /// @returns true if it's not disposed and no in disposing
    inline sal_Bool IsNotAlive( void ) const;
    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );

protected:
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified row index is invalid. */
    void ensureIsValidRow( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified column index is invalid. */
    void ensureIsValidColumn( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified cell address is invalid. */
    void ensureIsValidAddress( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified child index is invalid. */
    void ensureIsValidIndex( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );


    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> m_xCurChild;
public:
    void NotifyChild(long nIndex,sal_Bool bSelect ,sal_Bool bCheck);
};

inline sal_Bool SvxPixelCtlAccessible::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline sal_Bool SvxPixelCtlAccessible::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}

inline sal_Bool SvxPixelCtlAccessibleChild::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline sal_Bool SvxPixelCtlAccessibleChild::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}


#endif
