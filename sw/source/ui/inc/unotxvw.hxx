/*************************************************************************
 *
 *  $RCSfile: unotxvw.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:43 $
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
#ifndef _UNOTXVW_HXX
#define _UNOTXVW_HXX

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfx2/sfxbasecontroller.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSOR_HPP_
#include <com/sun/star/text/XTextViewCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSORSUPPLIER_HPP_
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XCONTROLACCESS_HPP_
#include <com/sun/star/view/XControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSCREENCURSOR_HPP_
#include <com/sun/star/view/XScreenCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XLINECURSOR_HPP_
#include <com/sun/star/view/XLineCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWCURSOR_HPP_
#include <com/sun/star/view/XViewCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPAGECURSOR_HPP_
#include <com/sun/star/text/XPageCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx> // helper for implementations
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif


class SwView;
typedef ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > * XSelectionChangeListenerPtr;
SV_DECL_PTRARR_DEL( SelectionChangeListenerArr, XSelectionChangeListenerPtr, 4, 4 );

/******************************************************************************
 *
 ******************************************************************************/
class SwXTextView :
    public ::com::sun::star::view::XSelectionSupplier,
    public ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::view::XControlAccess,
    public ::com::sun::star::text::XTextViewCursorSupplier,
    public ::com::sun::star::view::XViewSettingsSupplier,
    public SfxBaseController
{
    SelectionChangeListenerArr aSelChangedListeners;

    SwView*                 pView;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *         pxViewSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor > *   pxTextViewCursor;

public:
    SwXTextView(SwView* pSwView);
    virtual ~SwXTextView();

    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XSelectionSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& aInterface) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener) throw( ::com::sun::star::uno::RuntimeException );

    //XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getControl(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XTextViewCursorSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextViewCursor >  SAL_CALL getViewCursor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getViewSettings(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void                    NotifySelChanged();

    SwView*                 GetView() {return pView;}
    void                    Invalidate(){pView = 0;}
};

/* -----------------17.09.98 12:52-------------------
 *
 * --------------------------------------------------*/
class SwXTextViewCursor : public cppu::WeakImplHelper8
<
::com::sun::star::text::XTextViewCursor,
::com::sun::star::lang::XServiceInfo,
::com::sun::star::text::XPageCursor,
::com::sun::star::view::XScreenCursor,
::com::sun::star::view::XViewCursor,
::com::sun::star::view::XLineCursor,
::com::sun::star::beans::XPropertySet,
::com::sun::star::beans::XPropertyState
>
{
    SwView*             pView;
    SfxItemPropertySet  aPropSet;
public:
    SwXTextViewCursor(SwView* pVw);
    virtual ~SwXTextViewCursor();

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setVisible(sal_Bool bVisible) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextCursor - neu
    virtual void SAL_CALL collapseToStart(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collapseToEnd(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL isCollapsed(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goLeft( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual BOOL SAL_CALL goRight( sal_Int16 nCount, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoStart( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, BOOL bExpand ) throw(::com::sun::star::uno::RuntimeException);

    //XPageCursor
    virtual sal_Bool SAL_CALL jumpToFirstPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToLastPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToPage(sal_Int16 nPage) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToNextPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToPreviousPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToEndOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL jumpToStartOfPage(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL getPage(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL screenUp(void) throw( ::com::sun::star::uno::RuntimeException );

    //XViewCursor
    virtual sal_Bool SAL_CALL goDown(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL goUp(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
//    virtual sal_Bool goLeft(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
//    virtual sal_Bool goRight(sal_Int16 nCount, sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );

    //XLineCursor
    virtual sal_Bool SAL_CALL isAtStartOfLine(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isAtEndOfLine(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL gotoEndOfLine(sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL gotoStartOfLine(sal_Bool bExpand) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void    Invalidate(){pView = 0;}
};
#endif


