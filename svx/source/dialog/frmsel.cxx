/*************************************************************************
 *
 *  $RCSfile: frmsel.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:53 $
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

// include ---------------------------------------------------------------

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#pragma hdrstop

#define _SVX_FRMSEL_CXX
#define ITEMID_BOX          SID_ATTR_BORDER_OUTER
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include "frmsel.hxx"
#include <frmsel.hrc>
#ifndef _SVX_DIALOGS_HRC
#include <dialogs.hrc>
#endif
#include "dialmgr.hxx"
#include "linelink.hxx"

#ifndef _SVX_BOXITEM_HXX //autogen
#include <boxitem.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEY_HPP_
#include <com/sun/star/awt/Key.hpp>
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
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HDL_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HDL_
#include <drafts/com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HDL_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FOCUSCHANGEREASON_HPP_
#include <com/sun/star/awt/FocusChangeReason.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef MNEMONIC_CHAR
#define MNEMONIC_CHAR               ((sal_Unicode)'~')
#endif
#include <com/sun/star/awt/XFocusListener.hpp>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

// struct SvxFrameSelector_Impl ------------------------------------------------
struct SvxFrameSelector_Impl
{
    SvxFrameSelectorType                eSel;           // Selektor-Typ (Tabelle oder Absatz)

    Color                               aCurLineCol;    // aktuelle Linienfarbe
    Color                               aPaintLineCol;
    SvxLineStruct                       aCurLineStyle;  // aktueller LineStyle
    Bitmap                              aBackBmp;       // Hintergrund-Bitmap
    Rectangle                           aRectFrame;     // der Rahmen (Mitte der Linien)
    Rectangle                           aBoundingRect;  // alle Linien umschliessender Rahmen
    SvxFrameLine                        aLeftLine;      // seine Linien
    SvxFrameLine                        aRightLine;
    SvxFrameLine                        aTopLine;
    SvxFrameLine                        aBottomLine;
    SvxFrameLine                        aHorLine;
    SvxFrameLine                        aVerLine;
    Rectangle                           aSpotLeft;      // Click-HotSpots auf der Bitmap
    Rectangle                           aSpotRight;
    Rectangle                           aSpotTop;
    Rectangle                           aSpotBottom;
    Rectangle                           aSpotHor;
    Rectangle                           aSpotVer;
    BOOL                                bIsDontCare;
    BOOL                                bHC;
    SvxFrameSelectorAccessible_Impl*    pAccess;
    uno::Reference< XAccessible >       xAccess;
    SvxFrameSelectorAccessible_Impl*    pChildren[6];
    uno::Reference< XAccessible >       xChildren[6];

                                        SvxFrameSelector_Impl( const SvxFrameSelector* pThis );
                                        ~SvxFrameSelector_Impl();

    uno::Reference< XAccessible >       GetChildAccessible( SvxFrameSelector& rFrameSel, SvxFrameSelectorLine eWhich );
    const Rectangle&                    GetLineSpot( SvxFrameSelectorLine eWhich ) const;

    void                                SetHC( const SvxFrameSelector* pContrl = NULL );    // pContrl == NULL -> no HC
    inline const Color&                 GetPaintLineCol( void ) const;
};

inline const Color& SvxFrameSelector_Impl::GetPaintLineCol( void ) const
{
    return bHC? aPaintLineCol : aCurLineCol;
}

// class SvxFrameSelectorAccessible_Impl ------------------------------------------------

class SvxFrameSelectorAccessible_Impl :
    public ::cppu::WeakImplHelper4<
                ::drafts::com::sun::star::accessibility::XAccessible,
                ::drafts::com::sun::star::accessibility::XAccessibleContext,
                ::drafts::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::lang::XServiceInfo
                >,
    public Resource
{
    SvxFrameSelector*   pFrameSel;
    ::osl::Mutex aFocusMutex;
    ::osl::Mutex aPropertyMutex;

    SvxFrameSelectorLine    eWhichChild;

    ::cppu::OInterfaceContainerHelper aFocusListeners;
    ::cppu::OInterfaceContainerHelper aPropertyListeners;

    ResStringArray      aNameArray;
    ResStringArray      aDescriptionArray;

    void IsValid() throw (::com::sun::star::uno::RuntimeException);
public:
    SvxFrameSelectorAccessible_Impl(SvxFrameSelector& rSelector, SvxFrameSelectorLine eWhichChild);
    ~SvxFrameSelectorAccessible_Impl();

    //XAccessible
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    //XAccessibleComponent
    virtual sal_Bool SAL_CALL contains( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAt( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isShowing(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isVisible(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isFocusTraversable(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    void    Invalidate();
    void    NotifyFocusListeners(sal_Bool bGetFocus);
};

/*-- 04.02.2002 14:11:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxFrameSelectorAccessible_Impl::SvxFrameSelectorAccessible_Impl(
        SvxFrameSelector& rSelector, SvxFrameSelectorLine eChild) :
    Resource(SVX_RES( RID_SVXSTR_BORDER_CONTROL)),
    aNameArray(         ResId(ARR_TEXTS       )),
    aDescriptionArray(  ResId(ARR_DESCRIPTIONS)),
    pFrameSel(&rSelector),
    aFocusListeners(aFocusMutex),
    aPropertyListeners(aPropertyMutex),
    eWhichChild(eChild)
{
    FreeResource();
}
/*-- 04.02.2002 14:11:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxFrameSelectorAccessible_Impl::~SvxFrameSelectorAccessible_Impl()
{
}
/*-- 04.02.2002 14:11:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessibleContext > SvxFrameSelectorAccessible_Impl::getAccessibleContext(  )
    throw (uno::RuntimeException)
{
    return this;
}
/*-- 04.02.2002 14:11:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SvxFrameSelectorAccessible_Impl::getAccessibleChildCount(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return eWhichChild != SVX_FRMSELLINE_NONE ?
                0 :
                    pFrameSel->pImpl->eSel == SVX_FRMSELTYPE_TABLE ? 6 : 4;
}
/*-- 04.02.2002 14:11:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessible > SvxFrameSelectorAccessible_Impl::getAccessibleChild( sal_Int32 i )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    uno::Reference< XAccessible > xRet;
    switch(i)
    {
        case 0: xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_LEFT); break;
        case 1: xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_RIGHT); break;
        case 2: xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_TOP); break;
        case 3: xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_BOTTOM); break;
        case 4: if(pFrameSel->pImpl->eSel == SVX_FRMSELTYPE_TABLE)
                    xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_HOR);
        break;
        case 5: if(pFrameSel->pImpl->eSel == SVX_FRMSELTYPE_TABLE)
            xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, SVX_FRMSELLINE_VER);
        break;
    }
    if(!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}
/*-- 04.02.2002 14:11:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessible > SvxFrameSelectorAccessible_Impl::getAccessibleParent(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    uno::Reference< XAccessible > xRet;
    if(eWhichChild == SVX_FRMSELLINE_NONE)
        xRet = pFrameSel->GetParent()->GetAccessible( sal_True );
    else
        xRet = pFrameSel->CreateAccessible();
    return xRet;
}
/*-- 04.02.2002 14:11:56---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SvxFrameSelectorAccessible_Impl::getAccessibleIndexInParent(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    USHORT nIdx = 0;
    if(eWhichChild == SVX_FRMSELLINE_NONE)
    {
        Window* pTabPage = pFrameSel->GetParent();
        USHORT nChildren = pTabPage->GetChildCount();
        for(nIdx = 0; nIdx < nChildren; nIdx++)
            if(pTabPage->GetChild( nIdx ) == pFrameSel)
                break;
    }
    else
        nIdx = eWhichChild - 1;
    return nIdx;
}
/*-- 04.02.2002 14:11:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SvxFrameSelectorAccessible_Impl::getAccessibleRole(  ) throw (uno::RuntimeException)
{
    return AccessibleRole::OPTIONPANE;
}
/*-- 04.02.2002 14:11:57---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString SvxFrameSelectorAccessible_Impl::getAccessibleDescription(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return aDescriptionArray.GetString(eWhichChild);
}
/*-- 04.02.2002 14:11:57---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString SvxFrameSelectorAccessible_Impl::getAccessibleName(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return aNameArray.GetString(eWhichChild);
}
/*-- 04.02.2002 14:11:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessibleRelationSet > SvxFrameSelectorAccessible_Impl::getAccessibleRelationSet(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    uno::Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(eWhichChild == SVX_FRMSELLINE_NONE)
    {
        //add the label relation
        Window* pPrev = pFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            AccessibleRelation aLabelRelation;
            aLabelRelation.RelationType = AccessibleRelationType::LABELED_BY;
            aLabelRelation.TargetSet.realloc(1);
            aLabelRelation.TargetSet.getArray()[0]  = pPrev->GetAccessible();
            pHelper->AddRelation(aLabelRelation);
        }
    }
    return xRet;
}
/*-- 04.02.2002 14:11:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessibleStateSet > SvxFrameSelectorAccessible_Impl::getAccessibleStateSet(  )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< XAccessibleStateSet > xRet = pStateSetHelper;

    if(!pFrameSel)
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);
    else
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::EDITABLE,
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::MULTISELECTABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0};
        sal_Int16 nState = 0;
        while(aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if(pFrameSel->IsEnabled())
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);

        sal_Bool bIsParent = eWhichChild == SVX_FRMSELLINE_NONE;
        if(pFrameSel->HasFocus() &&
            (bIsParent || pFrameSel->GetLine(eWhichChild).IsSelected()))
        {
            pStateSetHelper->AddState(AccessibleStateType::ACTIVE);
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
            pStateSetHelper->AddState(AccessibleStateType::SELECTED);
        }
    }
    return xRet;
}
/*-- 04.02.2002 14:11:58---------------------------------------------------

  -----------------------------------------------------------------------*/
lang::Locale SvxFrameSelectorAccessible_Impl::getLocale(  )
    throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    lang::Locale aRet;
    SvxLanguageToLocale( aRet, Application::GetSettings().GetUILanguage() );
    return aRet;
}
/*-- 04.02.2002 14:11:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::addPropertyChangeListener(
    const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw (uno::RuntimeException)
{
    aPropertyListeners.addInterface( xListener );
}
/*-- 04.02.2002 14:11:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::removePropertyChangeListener( const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    aPropertyListeners.removeInterface( xListener );
}
/*-- 04.02.2002 14:11:59---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvxFrameSelectorAccessible_Impl::contains( const awt::Point& aPt )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPoint(aPt.X, aPt.Y);
    //aPoint is relative to the frame selector
    return pFrameSel->pImpl->aSpotLeft.IsInside( aPoint ) ||
            pFrameSel->pImpl->aSpotRight.IsInside( aPoint ) ||
            pFrameSel->pImpl->aSpotTop.IsInside( aPoint ) ||
            pFrameSel->pImpl->aSpotBottom.IsInside( aPoint ) ||
            (pFrameSel->pImpl->eSel == SVX_FRMSELTYPE_TABLE &&
                (pFrameSel->pImpl->aSpotHor.IsInside( aPoint ) ||
                    pFrameSel->pImpl->aSpotVer.IsInside( aPoint ) ));
}
/*-- 04.02.2002 14:12:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XAccessible > SvxFrameSelectorAccessible_Impl::getAccessibleAt(
    const awt::Point& aPt )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPoint(aPt.X, aPt.Y);
    uno::Reference< XAccessible > xRet;
    SvxFrameSelectorLine eEnd = pFrameSel->pImpl->eSel == SVX_FRMSELTYPE_TABLE ?
        SVX_FRMSELLINE_VER : SVX_FRMSELLINE_BOTTOM;
    for(sal_Int16 nWhich = SVX_FRMSELLINE_LEFT; nWhich <= eEnd; nWhich++)
    {
        const Rectangle aSpot = pFrameSel->pImpl->GetLineSpot((SvxFrameSelectorLine) nWhich);
        if(aSpot.IsInside( aPoint ))
        {
            xRet = pFrameSel->pImpl->GetChildAccessible(*pFrameSel, (SvxFrameSelectorLine) nWhich);
            break;
        }
    }
    return xRet;
}
/*-- 04.02.2002 14:12:00---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Rectangle SvxFrameSelectorAccessible_Impl::getBounds(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Size aSz;
    Point aPos;
    switch(eWhichChild)
    {
        case SVX_FRMSELLINE_NONE:
            aSz = pFrameSel->GetSizePixel();
            aPos = pFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = pFrameSel->pImpl->GetLineSpot(eWhichChild);
            aPos = aSpot.TopLeft();
            aSz = aSpot.GetSize();
    }
    awt::Rectangle aRet;
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}
/*-- 04.02.2002 14:12:00---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SvxFrameSelectorAccessible_Impl::getLocation(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPos;
    switch(eWhichChild)
    {
        case SVX_FRMSELLINE_NONE:
            aPos = pFrameSel->GetPosPixel();
        break;
        default:
            const SvxFrameLine& rLine = pFrameSel->GetLine(eWhichChild);
            aPos = rLine.GetStartPos();
    }
    awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}
/*-- 04.02.2002 14:12:01---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SvxFrameSelectorAccessible_Impl::getLocationOnScreen(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPos;
    switch(eWhichChild)
    {
        case SVX_FRMSELLINE_NONE:
            aPos = pFrameSel->GetPosPixel();
        break;
        default:
            const SvxFrameLine& rLine = pFrameSel->GetLine(eWhichChild);
            aPos = rLine.GetStartPos();
    }
    aPos = pFrameSel->OutputToAbsoluteScreenPixel( aPos );
    awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}
/*-- 04.02.2002 14:12:01---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Size SvxFrameSelectorAccessible_Impl::getSize(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Size aSz;
    switch(eWhichChild)
    {
        case SVX_FRMSELLINE_NONE:
            aSz = pFrameSel->GetSizePixel();
        break;
        default:
            const SvxFrameLine& rLine = pFrameSel->GetLine(eWhichChild);
            Point aPos = rLine.GetStartPos();
            Point aEndPos = rLine.GetEndPos();
            aSz.Width() = aPos.X() - aEndPos.X();
            aSz.Height() = aPos.Y() - aEndPos.Y();
    }
    awt::Size aRet(aSz.Width(), aSz.Height());
    return awt::Size();
}
/*-- 04.02.2002 14:12:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvxFrameSelectorAccessible_Impl::isShowing(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}
/*-- 04.02.2002 14:12:02---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvxFrameSelectorAccessible_Impl::isVisible(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}
/*-- 04.02.2002 14:12:02---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvxFrameSelectorAccessible_Impl::isFocusTraversable(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}
/*-- 04.02.2002 14:12:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
{
    aFocusListeners.addInterface( xListener );
}
/*-- 04.02.2002 14:12:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
{
    aFocusListeners.removeInterface( xListener );
}
/*-- 04.02.2002 14:12:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::grabFocus(  ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    pFrameSel->GrabFocus();
}
/*-- 04.02.2002 14:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SvxFrameSelectorAccessible_Impl::getAccessibleKeyBinding(  ) throw (uno::RuntimeException)
{
    uno::Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    uno::Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(eWhichChild == SVX_FRMSELLINE_NONE)
    {
        Window* pPrev = pFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            String sText = pPrev->GetText();
            xub_StrLen nFound = sText.Search( MNEMONIC_CHAR );
            if(STRING_NOTFOUND != nFound && ++nFound < sText.Len())
            {
                sText.ToUpperAscii();
                sal_Unicode cChar = sText.GetChar(nFound);
                awt::KeyEvent aEvent;

                aEvent.KeyCode = 0;
                aEvent.KeyChar = cChar;
                aEvent.KeyFunc = 0;
                if(cChar >= 'A' && cChar <= 'Z')
                {
                     aEvent.KeyCode = awt::Key::A + cChar - 'A';
                }
                aEvent.Modifiers = awt::KeyModifier::MOD2;
                aRet <<= aEvent;
            }
        }
    }
    return aRet;
}
/*-- 04.02.2002 14:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SvxFrameSelectorAccessible_Impl::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    uno::Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return pFrameSel->GetControlForeground().GetColor();
}
/*-- 04.02.2002 14:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SvxFrameSelectorAccessible_Impl::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    uno::Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return pFrameSel->GetControlBackground().GetColor();
}
/*-- 04.02.2002 14:12:04---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString SvxFrameSelectorAccessible_Impl::getImplementationName(  ) throw (uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("SvxFrameSelectorAccessible_Impl");
}
/*-- 04.02.2002 14:12:05---------------------------------------------------

  -----------------------------------------------------------------------*/
const sal_Char sAccessible[]          = "Accessible";
const sal_Char sAccessibleContext[]   = "AccessibleContext";
const sal_Char sAccessibleComponent[] = "AccessibleComponent";

sal_Bool SvxFrameSelectorAccessible_Impl::supportsService( const ::rtl::OUString& rServiceName )
    throw (uno::RuntimeException)
{
    return  rServiceName.equalsAsciiL( sAccessible         , sizeof(sAccessible         )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleContext  , sizeof(sAccessibleContext  )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleComponent, sizeof(sAccessibleComponent)-1 );
}
/*-- 04.02.2002 14:12:05---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< ::rtl::OUString > SvxFrameSelectorAccessible_Impl::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessible         ) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleContext  ) );
    pArray[2] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleComponent) );
    return aRet;
}
/* -----------------------------04.02.2002 14:31------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelectorAccessible_Impl::IsValid() throw (uno::RuntimeException)
{
    if(!pFrameSel)
        throw uno::RuntimeException();
}
/* -----------------------------04.02.2002 14:31------------------------------

 ---------------------------------------------------------------------------*/
void    SvxFrameSelectorAccessible_Impl::NotifyFocusListeners(sal_Bool bGetFocus)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    awt::FocusEvent aEvent;
    aEvent.FocusFlags = 0;
    if(bGetFocus)
    {
        USHORT nFocusFlags = pFrameSel->GetGetFocusFlags();
        if(nFocusFlags&GETFOCUS_TAB)
            aEvent.FocusFlags |= awt::FocusChangeReason::TAB;
        if(nFocusFlags&GETFOCUS_CURSOR)
            aEvent.FocusFlags |= awt::FocusChangeReason::CURSOR;
        if(nFocusFlags&GETFOCUS_MNEMONIC)
            aEvent.FocusFlags |= awt::FocusChangeReason::MNEMONIC;
        if(nFocusFlags&GETFOCUS_FORWARD)
            aEvent.FocusFlags |= awt::FocusChangeReason::FORWARD;
        if(nFocusFlags&GETFOCUS_BACKWARD)
            aEvent.FocusFlags |= awt::FocusChangeReason::BACKWARD;
        if(nFocusFlags&GETFOCUS_AROUND)
            aEvent.FocusFlags |= awt::FocusChangeReason::AROUND;
        if(nFocusFlags&GETFOCUS_UNIQUEMNEMONIC)
            aEvent.FocusFlags |= awt::FocusChangeReason::UNIQUEMNEMONIC;
    //        if(nFocusFlags&GETFOCUS_INIT)
    //            aEvent.FocusFlags |= FocusChangeReason::
    }
//    else
    //how can I find the current focus window?
//        aEvent.NextFocus = ;
    aEvent.Temporary = sal_False;

    uno::Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;

    ::cppu::OInterfaceIteratorHelper aIter( aFocusListeners );
    while( aIter.hasMoreElements() )
    {
        uno::Reference < awt::XFocusListener > xListener( aIter.next(), uno::UNO_QUERY );
        if(bGetFocus)
            xListener->focusGained( aEvent );
        else
            xListener->focusLost( aEvent );
    }
}
/* -----------------------------04.02.2002 16:16------------------------------

 ---------------------------------------------------------------------------*/
void    SvxFrameSelectorAccessible_Impl::Invalidate()
{
    pFrameSel = 0;
    lang::EventObject aEvent;
    uno::Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;
    aFocusListeners.disposeAndClear( aEvent );
    aPropertyListeners.disposeAndClear( aEvent );
}
// class SvxFrameLine ----------------------------------------------------

struct SvxLineStruct SvxFrameLine::NO_LINE           = { 0,0,0 };
struct SvxLineStruct SvxFrameLine::THIN_LINE         = { 1,0,0 };
struct SvxLineStruct SvxFrameLine::THICK_LINE        = { 3,0,0 };
struct SvxLineStruct SvxFrameLine::THIN_DOUBLE_LINE  = { 1,1,1 };
struct SvxLineStruct SvxFrameLine::THICK_DOUBLE_LINE = { 2,1,2 };

//------------------------------------------------------------------------

SvxFrameLine::SvxFrameLine()
    :   aStartPos    ( 0, 0 ),
        aMidPos      ( 0, 0 ),
        aEndPos      ( 0, 0 ),
        aColor       ( COL_BLACK ),
        aState     ( SVX_FRMLINESTATE_HIDE ),
#ifndef MIPS
        aStyle     ( NO_LINE ),
        aCoreStyle ( NO_LINE ),
#endif
        bIsSelected  ( FALSE )
{
#if defined MIPS
    aStyle.nLeft=0;
    aStyle.nMiddle=0;
    aStyle.nRight=0;
    aCoreStyle.nLeft=0;
    aCoreStyle.nMiddle=0;
    aCoreStyle.nRight=0;
#endif
}

//------------------------------------------------------------------------

#define WIDTH_THICK     (DEF_LINE_WIDTH_2 *100)

void SvxFrameLine::SetStyle( const SvxLineStruct& rStyle )
{
    aCoreStyle = rStyle;
    /*
     * Mapping: (in TWIPS, 1pt = 100 * 20 TWIPS = duenn)
     * alles was <=0                    ist -> NO_LINE
     * alles was <=DEF_SLINE_WIDTH_2    ist -> THIN_LINE
     * alles andere                         -> THICK_LINE
     */
    if ( ( rStyle.nLeft <= 0 ) && ( rStyle.nRight <= 0 ) )
    {
        aStyle = NO_LINE;
        aState = SVX_FRMLINESTATE_HIDE;
    }
    else if ( ( rStyle.nRight == 0 ) &&             // einzelne Linie
              ( rStyle.nLeft  >  0 ) &&             // sichtbar
              ( rStyle.nLeft  < WIDTH_THICK ) )     // duenn
    {
        aStyle = THIN_LINE;
        aState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( rStyle.nRight == 0 ) &&             // einzelne Linie
              ( rStyle.nLeft   > 0 ) &&             // sichtbar
              ( rStyle.nLeft  >= WIDTH_THICK ) )    // dick
    {
        aStyle = THICK_LINE;
        aState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( rStyle.nRight > 0 ) &&              // doppelte Linie
              ( rStyle.nLeft  > 0 ) &&              // sichtbar
              ( rStyle.nLeft  < WIDTH_THICK ) )     // duenn
    {
        aStyle = THIN_DOUBLE_LINE;
        aState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( rStyle.nRight > 0 ) &&              // doppelte Linie
              ( rStyle.nLeft  > 0 ) &&              // sichtbar
              ( rStyle.nLeft  >= WIDTH_THICK ) )    // dick
    {
        aStyle = THICK_DOUBLE_LINE;
        aState = SVX_FRMLINESTATE_SHOW;
    }
}

#undef WIDTH_THICK

//------------------------------------------------------------------------

void SvxFrameLine::SetState( SvxFrameLineState eState )
{
    aState = eState;

    if ( SVX_FRMLINESTATE_DONT_CARE == eState )
    {
        aStyle     = THICK_LINE;
        aCoreStyle = NO_LINE;
    }
}
// class SvxFrameSelector ------------------------------------------------

inline const Color& SvxFrameSelector::GetPaintColor( const Color& rNormalColor ) const
{
    return pImpl->bHC? pImpl->aPaintLineCol : rNormalColor;
}

inline const Color& SvxFrameSelector::GetDisplayBackgroundColor( const StyleSettings& rSettings ) const
{
    return rSettings.GetFieldColor();
}

const Color& SvxFrameSelector::GetDisplayBackgroundColor( void ) const
{
    return GetDisplayBackgroundColor( GetSettings().GetStyleSettings() );
}

SvxFrameSelector::SvxFrameSelector( Window* pParent,
                                    const ResId& rResId )
    :   Control         ( pParent, rResId ),
        eShadow         ( SVX_FRMSHADOW_NONE ),
        aShadowCol      ( COL_BLACK ),
        bIsClicked      ( FALSE ),
        pImpl           ( new SvxFrameSelector_Impl( this ) )
{
    EnableRTL( false ); // #107808# don't mirror the mouse handling
}
/* -----------------------------01.02.2002 16:47------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameSelector::~SvxFrameSelector()
{
    delete pImpl;
}
/* -----------------------------01.02.2002 14:37------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::Initialize(  SvxFrameSelectorType eType, BOOL bDontCare )
{
    pImpl->eSel = eType;
    pImpl->bIsDontCare = bDontCare;
    InitBitmap_Impl();
    Show();
}
// -----------------------------------------------------------------------

void SvxFrameSelector::InitBitmap_Impl()
{
    Size aSzParent( GetSizePixel() );
    aSzParent.Width() -= 4;
    aSzParent.Height() -= 4;
    VirtualDevice   aVirDev;
    const long      nX  = aSzParent.Width();
    const long      nY  = aSzParent.Height();
    long            nXMid = nX / 2;
    long            nYMid = nY / 2;

    /*
     * Berechnen des Frame-Rects und der Linie-Mittelpunkte
     */
    pImpl->aBoundingRect = pImpl->aRectFrame = Rectangle( 21, 21, nX - 21, nY - 21 );

    pImpl->aLeftLine.aStartPos   = Point( 21, 21 );
    pImpl->aLeftLine.aEndPos     = Point( 21, nY - 21 );
    pImpl->aLeftLine.aMidPos     = Point( 21, nYMid );

    pImpl->aRightLine.aStartPos  = Point( nX - 21, 21 );
    pImpl->aRightLine.aEndPos    = Point( nX - 21, nY - 21 );
    pImpl->aRightLine.aMidPos    = Point( nX - 21, nYMid );

    pImpl->aVerLine.aStartPos    = Point( nXMid, 21 );
    pImpl->aVerLine.aEndPos      = Point( nXMid, nY - 21 );
    pImpl->aVerLine.aMidPos      = Point( nXMid, nYMid );

    pImpl->aTopLine.aStartPos    = pImpl->aLeftLine.aStartPos;
    pImpl->aTopLine.aEndPos      = pImpl->aRightLine.aStartPos;
    pImpl->aTopLine.aMidPos      = pImpl->aVerLine.aStartPos;

    pImpl->aBottomLine.aStartPos = pImpl->aLeftLine.aEndPos;
    pImpl->aBottomLine.aEndPos   = pImpl->aRightLine.aEndPos;
    pImpl->aBottomLine.aMidPos   = pImpl->aVerLine.aEndPos;

    pImpl->aHorLine.aStartPos    = pImpl->aLeftLine.aMidPos;
    pImpl->aHorLine.aEndPos      = pImpl->aRightLine.aMidPos;
    pImpl->aHorLine.aMidPos      = pImpl->aVerLine.aMidPos;

    // HotSpot-Rectangles:
    pImpl->aSpotLeft   = Rectangle( Point( 0, 0 ),
                             Size( pImpl->aLeftLine.aStartPos.X() + 3,
                                   nY ) );
    pImpl->aSpotRight  = Rectangle( Point( pImpl->aRightLine.aStartPos.X() - 3, 0 ),
                             pImpl->aSpotLeft.GetSize() );
    pImpl->aSpotTop        = Rectangle( Point( pImpl->aTopLine.aStartPos.X(), 0 ),
                             Size( pImpl->aRectFrame.GetWidth(),
                                   pImpl->aTopLine.aStartPos.Y() + 3 ) );
    pImpl->aSpotBottom = Rectangle( pImpl->aBottomLine.aStartPos - Point( 0, 3 ),
                             pImpl->aSpotTop.GetSize() );
    pImpl->aSpotHor        = ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
                  ? Rectangle( pImpl->aHorLine.aStartPos + Point( 0, -2 ),
                               Size( pImpl->aRectFrame.GetWidth(), 5 ) )
                  : Rectangle( Point( -1, -1 ), Size(- 1, -1 ) );
    pImpl->aSpotVer        = ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
                  ? Rectangle( pImpl->aVerLine.aStartPos + Point( -2, 0 ),
                               Size( 5, pImpl->aRectFrame.GetHeight() ) )
                  : Rectangle( Point( -1, -1 ), Size( -1, -1 ) );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    /*
     * Initialisieren der Bitmap:
     */
    aVirDev.SetOutputSizePixel( aSzParent );
    aVirDev.SetLineColor();

    aVirDev.SetFillColor( GetDisplayBackgroundColor( rStyleSettings ) );
    aVirDev.DrawRect( Rectangle( Point( 0, 0 ), aSzParent ) );

    DrawContents_Impl( aVirDev );

    /*
     * Malen der vier Eck-Winkel:
     */
    aVirDev.SetLineColor( rStyleSettings.GetFieldTextColor() );

    // links/rechts oben:
    aVirDev.DrawLine( Point( 10, 15 ), Point( 15, 15 ) );
    aVirDev.DrawLine( Point( 15, 15 ), Point( 15, 10 ) );
    aVirDev.DrawLine( Point( nX - 10, 15 ), Point( nX - 15, 15 ) );
    aVirDev.DrawLine( Point( nX - 15, 15 ), Point( nX - 15, 10 ) );

    // links/rechts unten:
    aVirDev.DrawLine( Point( 10, nY - 15 ), Point( 15, nY - 15 ) );
    aVirDev.DrawLine( Point( 15, nY - 15 ), Point( 15, nY - 10 ) );
    aVirDev.DrawLine( Point( nX -10, nY - 15 ), Point( nX - 15, nY - 15 ) );
    aVirDev.DrawLine( Point( nX -15, nY - 15 ), Point( nX - 15, nY - 10 ) );

    /*
     * Tabelle: Malen der vier Mitte-Winkel:
     */
    if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
    {
        // links/rechts
        aVirDev.DrawLine( Point( 10, nYMid ),       Point( 15, nYMid ) );
        aVirDev.DrawLine( Point( 15, nYMid-2 ),     Point( 15, nYMid+2 ) );
        aVirDev.DrawLine( Point( nX-10, nYMid ),    Point( nX-15, nYMid ) );
        aVirDev.DrawLine( Point( nX-15, nYMid-2 ),  Point( nX-15, nYMid+2 ) );

        // horizontal/vertikal
        aVirDev.DrawLine( Point( nXMid, 10 ),       Point( nXMid, 15 ) );
        aVirDev.DrawLine( Point( nXMid-2, 15 ),     Point( nXMid+2, 15 ) );
        aVirDev.DrawLine( Point( nXMid, nY-10 ),    Point( nXMid, nY-15 ) );
        aVirDev.DrawLine( Point( nXMid-2, nY-15 ),  Point( nXMid+2, nY-15 ) );
    }
    pImpl->aBackBmp = aVirDev.GetBitmap( Point( 0, 0 ), aSzParent );
}

// -----------------------------------------------------------------------

void SvxFrameSelector::DrawFrameLine_Impl( OutputDevice& rVirDev, Point from,
    Point to, BOOL bHor, SvxLineStruct& dline, SvxLineStruct& lt,
    SvxLineStruct& lLine, SvxLineStruct& lb, SvxLineStruct& rt,
    SvxLineStruct& rLine, SvxLineStruct& rb )
{
    short diffArr[4];       // Kreuzungspunkt-Differenzen
    short nHeight = 0;
    Point from2( from );
    Point to2( to );

    if ( bHor ) // horizontale Linien
    {
        // Berechnung der Differenzen der Linienlaenge
        // in den Kreuzungspunkten:
        LinkLine( dline, lt, lLine, lb, rt, rLine, rb, diffArr );
        nHeight = dline.nLeft + dline.nMiddle + dline.nRight;

        from.Y() -= nHeight/2;
        to.Y()   -= nHeight/2;
        to.Y()   += ( dline.nLeft - 1 );

        from.X() += diffArr[0];
        to.X()   += diffArr[2];

        rVirDev.DrawRect( Rectangle( from, to ) );

        // noch eine zweite Linie zu malen?
        if ( dline.nRight != 0 )
        {
            from2.Y() -= nHeight / 2;
            from2.Y() += dline.nLeft + dline.nMiddle;
            to2.Y()   -= nHeight / 2;
            to2.Y()   += dline.nMiddle + dline.nLeft;
            to2.Y()   += ( dline.nRight - 1 );
            from2.X() += diffArr[1];
            to2.X()   += diffArr[3];
            rVirDev.DrawRect( Rectangle( from2, to2 ) );
        }
    }
    else // vertikale Linien
    {
        // Berechnung der Differenzen der Linienlaenge
        // in den Kreuzungspunkten:
        LinkLine( dline, lt, lLine, lb, rt, rLine, rb, diffArr );
        nHeight = dline.nLeft + dline.nMiddle + dline.nRight;

        from.X() -= nHeight / 2;
        to.X()   -= nHeight / 2;
        to.X()   += ( dline.nLeft - 1 );
        from.Y() -= diffArr[2];
        to.Y()   -= diffArr[0];
        rVirDev.DrawRect( Rectangle( from, to ) );

        // noch eine zweite Linie zu malen?
        if ( dline.nRight != 0 )
        {
            from2.X() -= nHeight / 2;
            from2.X() += dline.nLeft + dline.nMiddle;
            to2.X()   -= nHeight / 2;
            to2.X()   += dline.nMiddle + dline.nLeft;
            to2.X()   += ( dline.nRight - 1 );
            from2.Y() -= diffArr[3];
            to2.Y()   -= diffArr[1];
            rVirDev.DrawRect( Rectangle( from2, to2 ) );
        }
    }

    /*
     * Anpassen des umgebenden Rectangles:
     */
    long x1 = Min( from.X(), from2.X() );
    long x2 = Max( to.X(),   to2.X()   );
    long y1 = Min( from.Y(), from2.Y() );
    long y2 = Max( to.Y(),   to2.Y()   );

    if ( x1 < pImpl->aBoundingRect.Left() )
        pImpl->aBoundingRect.Left() = x1;
    else if ( x1 > pImpl->aBoundingRect.Right() )
        pImpl->aBoundingRect.Right() = x1;

    if ( x2 < pImpl->aBoundingRect.Left() )
        pImpl->aBoundingRect.Left() = x2;
    else if ( x2 > pImpl->aBoundingRect.Right() )
        pImpl->aBoundingRect.Right() = x2;

    if ( y1 < pImpl->aBoundingRect.Top() )
        pImpl->aBoundingRect.Top() = y1;
    else if ( y1 > pImpl->aBoundingRect.Bottom() )
        pImpl->aBoundingRect.Bottom() = y1;

    if ( y2 < pImpl->aBoundingRect.Top() )
        pImpl->aBoundingRect.Top() = y2;
    else if ( y2 > pImpl->aBoundingRect.Bottom() )
        pImpl->aBoundingRect.Bottom() = y2;
}

// class SvxFrameSelector ------------------------------------------------

void SvxFrameSelector::HideLines()
{
    pImpl->aLeftLine.aState    =
    pImpl->aRightLine.aState   =
    pImpl->aTopLine.aState     =
    pImpl->aBottomLine.aState  =
    pImpl->aHorLine.aState     =
    pImpl->aVerLine.aState     = SVX_FRMLINESTATE_HIDE;
    pImpl->aLeftLine.aStyle    =
    pImpl->aRightLine.aStyle   =
    pImpl->aTopLine.aStyle     =
    pImpl->aBottomLine.aStyle  =
    pImpl->aHorLine.aStyle     =
    pImpl->aHorLine.aStyle     =
    pImpl->aVerLine.aStyle     = SvxFrameLine::NO_LINE;

    ShowLines();
}

// -----------------------------------------------------------------------

void SvxFrameSelector::ShowLines()
{
    VirtualDevice   aVirDev;
    Color           aFillColor;
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color aFieldColor = rStyleSettings.GetFieldColor();
    Color aBackgroundColor = aFieldColor; //was: COL_WHITE

    aVirDev.SetOutputSizePixel( pImpl->aBackBmp.GetSizePixel() );
    aVirDev.DrawBitmap( Point( 0, 0 ), pImpl->aBackBmp );

    // gesamten Rahmen loeschen und Inhalt malen
    aVirDev.SetLineColor();
    aVirDev.SetFillColor( aBackgroundColor );
    aVirDev.DrawRect( pImpl->aBoundingRect );
    DrawContents_Impl( aVirDev );
    pImpl->aBoundingRect = pImpl->aRectFrame;

    /*
     * Zeichnen aller Linien:
     * ----------------------
     * Eine Linie wird in zwei Abschnitten gemalt, um die Kreuzungs-
     * punkte mit anderen Linien beruecksichtigen zu koennen.
     * Diese Aufgabe uebernimmt die Methode DrawFrameLine_Impl().
     */

    // innere Linien: ---------------------------------------------

    if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE &&
         ( pImpl->aVerLine.aState == SVX_FRMLINESTATE_SHOW ||
           pImpl->aVerLine.aState == SVX_FRMLINESTATE_DONT_CARE ) )
    {
        if ( pImpl->aVerLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aVerLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aVerLine.aStartPos,
            pImpl->aVerLine.aMidPos,
            FALSE,
            pImpl->aVerLine.aStyle,    // dline
            pImpl->aHorLine.aStyle,    // rb
            pImpl->aVerLine.aStyle,    // lLine
            pImpl->aHorLine.aStyle,    // rt
            pImpl->aTopLine.aStyle,    // lb
            SvxFrameLine::NO_LINE,  // rLine
            pImpl->aTopLine.aStyle     // lt
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aVerLine.aMidPos,
            pImpl->aVerLine.aEndPos,
            FALSE,
            pImpl->aVerLine.aStyle,    // dline
            pImpl->aBottomLine.aStyle, // rb
            SvxFrameLine::NO_LINE,  // lLine
            pImpl->aBottomLine.aStyle, // rb
            pImpl->aHorLine.aStyle,    // lb
            pImpl->aVerLine.aStyle,    // rLine
            pImpl->aHorLine.aStyle     // lt
        );
    }

    if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE &&
         ( pImpl->aHorLine.aState == SVX_FRMLINESTATE_SHOW ||
           pImpl->aHorLine.aState == SVX_FRMLINESTATE_DONT_CARE ) )
    {
        if ( pImpl->aHorLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aHorLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aHorLine.aStartPos,
            pImpl->aHorLine.aMidPos,
            TRUE,
            pImpl->aHorLine.aStyle,    // dline
            pImpl->aLeftLine.aStyle,   // lt
            SvxFrameLine::NO_LINE,  // lLine
            pImpl->aLeftLine.aStyle,   // lb
            pImpl->aVerLine.aStyle,    // rt
            pImpl->aHorLine.aStyle,    // rLine
            pImpl->aVerLine.aStyle     // rb
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aHorLine.aMidPos,
            pImpl->aHorLine.aEndPos,
            TRUE,
            pImpl->aHorLine.aStyle,    // dline
            pImpl->aVerLine.aStyle,    // lt
            pImpl->aHorLine.aStyle,    // lLine
            pImpl->aVerLine.aStyle,    // lb
            pImpl->aRightLine.aStyle,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            pImpl->aRightLine.aStyle   // rb
        );
    }

    // aeussere Linien: -------------------------------------------

    if ( pImpl->aLeftLine.aState == SVX_FRMLINESTATE_SHOW ||
         pImpl->aLeftLine.aState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( pImpl->aLeftLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aLeftLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aLeftLine.aStartPos,
            pImpl->aLeftLine.aMidPos,
            FALSE,
            pImpl->aLeftLine.aStyle,   // dline
            SvxFrameLine::NO_LINE,  // lb
            pImpl->aLeftLine.aStyle,   // lLine
            pImpl->aHorLine.aStyle,    // lt
            SvxFrameLine::NO_LINE,  // rb
            SvxFrameLine::NO_LINE,  // rLine
            pImpl->aTopLine.aStyle     // rt
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aLeftLine.aMidPos,
            pImpl->aLeftLine.aEndPos,
            FALSE,
            pImpl->aLeftLine.aStyle,   // dline
            SvxFrameLine::NO_LINE,  // lb
            SvxFrameLine::NO_LINE,  // lLine
            pImpl->aBottomLine.aStyle, // lt
            SvxFrameLine::NO_LINE,  // rb
            pImpl->aLeftLine.aStyle,   // rLine
            pImpl->aHorLine.aStyle     // rt
        );
    }

    if ( pImpl->aRightLine.aState == SVX_FRMLINESTATE_SHOW ||
         pImpl->aRightLine.aState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( pImpl->aRightLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aRightLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aRightLine.aStartPos,
            pImpl->aRightLine.aMidPos,
            FALSE,
            pImpl->aRightLine.aStyle,  // dline
            pImpl->aHorLine.aStyle,    // rb
            pImpl->aRightLine.aStyle,  // lLine
            SvxFrameLine::NO_LINE,  // rt
            pImpl->aTopLine.aStyle,    // lb
            SvxFrameLine::NO_LINE,  // eLine
            SvxFrameLine::NO_LINE   // lt
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aRightLine.aMidPos,
            pImpl->aRightLine.aEndPos,
            FALSE,
            pImpl->aRightLine.aStyle,  // dline
            pImpl->aBottomLine.aStyle, // rb
            SvxFrameLine::NO_LINE,  // lLine
            SvxFrameLine::NO_LINE,  // lt
            pImpl->aHorLine.aStyle,    // lb
            pImpl->aRightLine.aStyle,  // lLine
            SvxFrameLine::NO_LINE   // rt
        );
    }

    if ( pImpl->aTopLine.aState == SVX_FRMLINESTATE_SHOW ||
         pImpl->aTopLine.aState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( pImpl->aTopLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aTopLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aTopLine.aStartPos,
            pImpl->aTopLine.aMidPos,
            TRUE,
            pImpl->aTopLine.aStyle,    // dline
            SvxFrameLine::NO_LINE,  // lt
            SvxFrameLine::NO_LINE,  // lLine
            pImpl->aLeftLine.aStyle,   // lb
            SvxFrameLine::NO_LINE,  // rt
            pImpl->aTopLine.aStyle,    // rLine
            pImpl->aVerLine.aStyle     // rb
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aTopLine.aMidPos,
            pImpl->aTopLine.aEndPos,
            TRUE,
            pImpl->aTopLine.aStyle,    // dline
            SvxFrameLine::NO_LINE,  // lt
            pImpl->aTopLine.aStyle,    // lLine
            pImpl->aVerLine.aStyle,    // lb
            SvxFrameLine::NO_LINE,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            pImpl->aRightLine.aStyle   // rb
        );
    }

    if ( pImpl->aBottomLine.aState == SVX_FRMLINESTATE_SHOW ||
         pImpl->aBottomLine.aState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( pImpl->aBottomLine.aState == SVX_FRMLINESTATE_SHOW )
            aFillColor = GetPaintColor( pImpl->aBottomLine.aColor );
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        if(aFieldColor == aFillColor)
            aFillColor.Invert();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aBottomLine.aStartPos,
            pImpl->aBottomLine.aMidPos,
            TRUE,
            pImpl->aBottomLine.aStyle, // dline
            pImpl->aLeftLine.aStyle,   // lt
            SvxFrameLine::NO_LINE,  // lLine
            SvxFrameLine::NO_LINE,  // lb
            pImpl->aVerLine.aStyle,    // rt
            pImpl->aBottomLine.aStyle, // rLine
            SvxFrameLine::NO_LINE   // rb
        );

        DrawFrameLine_Impl( aVirDev,
            pImpl->aBottomLine.aMidPos,
            pImpl->aBottomLine.aEndPos,
            TRUE,
            pImpl->aBottomLine.aStyle, // dline
            pImpl->aVerLine.aStyle,    // lt
            pImpl->aBottomLine.aStyle, // lLine
            SvxFrameLine::NO_LINE,  // lb
            pImpl->aRightLine.aStyle,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            SvxFrameLine::NO_LINE   // rb
        );
    }

    pImpl->aBackBmp = aVirDev.GetBitmap( Point(0,0), pImpl->aBackBmp.GetSizePixel() );

//    if ( eShadow != SVX_FRMSHADOW_NONE )
//        ShowShadow();
//    else
        Invalidate( INVALIDATE_NOERASE );
}

// -----------------------------------------------------------------------

void SvxFrameSelector::SetCurLineStyle( const SvxLineStruct& rStyle )
{
    pImpl->aCurLineStyle = rStyle;

    if ( pImpl->aLeftLine.bIsSelected )      pImpl->aLeftLine.SetStyle( rStyle );
    if ( pImpl->aRightLine.bIsSelected )     pImpl->aRightLine.SetStyle( rStyle );
    if ( pImpl->aTopLine.bIsSelected )       pImpl->aTopLine.SetStyle( rStyle );
    if ( pImpl->aBottomLine.bIsSelected )    pImpl->aBottomLine.SetStyle( rStyle );
    if ( pImpl->aHorLine.bIsSelected )       pImpl->aHorLine.SetStyle( rStyle );
    if ( pImpl->aVerLine.bIsSelected )       pImpl->aVerLine.SetStyle( rStyle );
}

//------------------------------------------------------------------------

void SvxFrameSelector::SetCurLineColor( const Color& aColor )
{
    pImpl->aCurLineCol = aColor;

    if ( pImpl->aLeftLine.bIsSelected )      pImpl->aLeftLine.aColor   = aColor;
    if ( pImpl->aRightLine.bIsSelected )     pImpl->aRightLine.aColor  = aColor;
    if ( pImpl->aTopLine.bIsSelected )       pImpl->aTopLine.aColor    = aColor;
    if ( pImpl->aBottomLine.bIsSelected )    pImpl->aBottomLine.aColor = aColor;
    if ( pImpl->aHorLine.bIsSelected )       pImpl->aHorLine.aColor    = aColor;
    if ( pImpl->aVerLine.bIsSelected )       pImpl->aVerLine.aColor    = aColor;
}

//------------------------------------------------------------------------

void SvxFrameSelector::DrawContents_Impl( OutputDevice& rVirDev )
{
    /*
     * Malen des Inhaltes:
     */
    if( pImpl->bHC )
        return;

    if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
    {
        Color aPrevLineColor = rVirDev.GetLineColor();
        rVirDev.SetLineColor( Color( COL_LIGHTGRAY ) );
        Color aPrevFillColor = rVirDev.GetFillColor();
        rVirDev.SetFillColor( Color( COL_LIGHTGRAY ) );

        Size aContentsSize =
            Size( pImpl->aVerLine.aStartPos.X() - pImpl->aLeftLine.aStartPos.X() - 8,
                  pImpl->aHorLine.aStartPos.Y() - pImpl->aTopLine.aStartPos.Y() - 8 );

        // links-oben
        rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // rechts-oben
        rVirDev.DrawRect( Rectangle( pImpl->aVerLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // links-unten
        rVirDev.DrawRect( Rectangle( pImpl->aHorLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // rechts-unten
        rVirDev.DrawRect( Rectangle( pImpl->aHorLine.aMidPos + Point( 4, 4 ),
                                     aContentsSize ) );

        rVirDev.SetLineColor( aPrevLineColor );
        rVirDev.SetFillColor( aPrevFillColor);
    }
    else
    {
        Color aPrevLineColor = rVirDev.GetLineColor();
        rVirDev.SetLineColor( Color( COL_LIGHTGRAY ) );
        Color aPrevFillColor = rVirDev.GetFillColor();
        rVirDev.SetFillColor( Color( COL_LIGHTGRAY ) );
        Size aContentsSize =
            Size( pImpl->aRightLine.aStartPos.X() - pImpl->aLeftLine.aStartPos.X() - 8 ,
                  pImpl->aBottomLine.aStartPos.Y() - pImpl->aLeftLine.aStartPos.Y() - 8 );

        rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );

    /* symbolisierter Paragraph ist nicht gewuenscht - vielleicht spaeter mal wieder...
        Size aContentsSize =
            Size( pImpl->aRightLine.aStartPos.X() - pImpl->aLeftLine.aStartPos.X() - 8,
                  pImpl->aBottomLine.aStartPos.Y() - pImpl->aLeftLine.aStartPos.Y() - 13 );

        rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aEndPos + Point( 4, -13 ),
                                     Size( 20, 8 ) ) );
    */
        /* -------------------------------------------------------------------
            kann StarWriter noch nicht - vielleicht spaeter mal:
            Size    aContentsSize =
                    Size(  pImpl->aRightLine.aStartPos.X()
                         - pImpl->aLeftLine.aStartPos.X()
                         - 8,
                           pImpl->aHorLine.aStartPos.Y()
                         - pImpl->aLeftLine.aStartPos.Y()
                         - 13 );

            // oben
            rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aStartPos + Point(4,4),
                                         aContentsSize ) );
            rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aMidPos + Point(4,-13),
                                         Size(20,8) ) );
            // unten
            rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aMidPos + Point(4,4),
                                         aContentsSize ) );
            rVirDev.DrawRect( Rectangle( pImpl->aLeftLine.aEndPos + Point(4,-13),
                                         Size(20,8) ) );
        --------------------------------------------------------------------*/
    }
}

//------------------------------------------------------------------------

void SvxFrameSelector::SelectLine( SvxFrameSelectorLine eNewLine, BOOL bSet )
{
    VirtualDevice aVirDev;
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Color aDrawColor(
        ( bSet && ( eNewLine != SVX_FRMSELLINE_NONE ) ) ?
            rStyleSettings.GetFieldTextColor() : rStyleSettings.GetFieldColor() );

    long nX    = pImpl->aBackBmp.GetSizePixel().Width();
    long nY    = pImpl->aBackBmp.GetSizePixel().Height();
    long nXMid = pImpl->aVerLine.aStartPos.X();
    long nYMid = pImpl->aHorLine.aStartPos.Y();

    aVirDev.SetOutputSizePixel( pImpl->aBackBmp.GetSizePixel() );
    aVirDev.DrawBitmap( Point( 0, 0 ), pImpl->aBackBmp );

    switch ( eNewLine )
    {
        case SVX_FRMSELLINE_LEFT:
        {
            DrawSelArrow_Impl( aVirDev, Point( 13, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( 13, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            pImpl->aLeftLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_RIGHT:
        {
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            pImpl->aRightLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_TOP:
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            pImpl->aTopLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_BOTTOM:
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            pImpl->aBottomLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_HOR:
        if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, nYMid - 2 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nYMid - 2 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            pImpl->aHorLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_VER:
        if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
        {
            DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN);
            DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            pImpl->aVerLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_NONE:
        {
            // links
            DrawSelArrow_Impl( aVirDev, Point( 13, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( 13, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            // rechts
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            // oben
            DrawSelArrow_Impl( aVirDev, Point( 3, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            // unten
            DrawSelArrow_Impl( aVirDev, Point( 3, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );

            if ( pImpl->eSel == SVX_FRMSELTYPE_TABLE )
            {
                // horizontal
                DrawSelArrow_Impl( aVirDev, Point( 3, nYMid-2 ), aDrawColor,
                              SVX_SELARROWTYPE_RIGHT );
                DrawSelArrow_Impl( aVirDev, Point( nX - 8, nYMid - 2 ), aDrawColor,
                              SVX_SELARROWTYPE_LEFT );

                // vertikal
                DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, 3 ),aDrawColor,
                              SVX_SELARROWTYPE_DOWN );
                DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, nY - 8 ), aDrawColor,
                              SVX_SELARROWTYPE_UP );
            }

            pImpl->aLeftLine.bIsSelected     =
            pImpl->aRightLine.bIsSelected    =
            pImpl->aTopLine.bIsSelected      =
            pImpl->aBottomLine.bIsSelected   =
            pImpl->aHorLine.bIsSelected      =
            pImpl->aVerLine.bIsSelected      = FALSE;
        }
    }
    pImpl->aBackBmp = aVirDev.GetBitmap( Point( 0, 0 ), pImpl->aBackBmp.GetSizePixel() );
    Invalidate( INVALIDATE_NOERASE );
    if(aSelectLink.IsSet())
        aSelectLink.Call(0);
}

//------------------------------------------------------------------------

void SvxFrameSelector::DrawSelArrow_Impl(

    OutputDevice& rDev, Point aPos, Color aCol, SvxSelArrowType eType )
{
    Point   aPtArray[5];
    Color   aPrevLineColor = rDev.GetLineColor();
    Color   aPrevFillColor = rDev.GetFillColor();
    rDev.SetLineColor( aCol );
    rDev.SetFillColor( aCol );

    switch ( eType )
    {
        case SVX_SELARROWTYPE_LEFT:
            aPtArray[0] = aPos + Point( 3, 0 );
            aPtArray[1] = aPos + Point( 4, 4 );
            aPtArray[2] = aPos + Point( 1, 1 );
            aPtArray[3] = aPos + Point( 2, 3 );
            aPtArray[4] = aPos + Point( 0, 2 );
            break;
        case SVX_SELARROWTYPE_RIGHT:
            aPtArray[0] = aPos;
            aPtArray[1] = aPos + Point( 1, 4 );
            aPtArray[2] = aPos + Point( 2, 1 );
            aPtArray[3] = aPos + Point( 3, 3 );
            aPtArray[4] = aPos + Point( 4, 2 );
            break;
        case SVX_SELARROWTYPE_UP:
            aPtArray[0] = aPos + Point( 0, 3 );
            aPtArray[1] = aPos + Point( 4, 4 );
            aPtArray[2] = aPos + Point( 1, 1 );
            aPtArray[3] = aPos + Point( 3, 2 );
            aPtArray[4] = aPos + Point( 2, 0 );
            break;
        case SVX_SELARROWTYPE_DOWN:
            aPtArray[0] = aPos;
            aPtArray[1] = aPos + Point( 4, 1 );
            aPtArray[2] = aPos + Point( 1, 2 );
            aPtArray[3] = aPos + Point( 3, 3 );
            aPtArray[4] = aPos + Point( 2, 4 );
            break;
    }
    rDev.DrawRect( Rectangle( aPtArray[0], aPtArray[1] ) );
    rDev.DrawRect( Rectangle( aPtArray[2], aPtArray[3] ) );
    rDev.DrawLine( aPtArray[4], aPtArray[4] );
    rDev.SetLineColor( aPrevLineColor );
    rDev.SetFillColor( aPrevFillColor );
}


// -----------------------------------------------------------------------
void lcl_InvertTracking(Window& rWin, const SvxFrameLine& rLine, sal_Bool bHori)
{
    Rectangle aCompleteRect;
    aCompleteRect.Left() = bHori ? rLine.GetStartPos().X() : rLine.GetStartPos().X() - 2;
    aCompleteRect.Right() = bHori ? rLine.GetEndPos().X() : rLine.GetEndPos().X() + 2;
    aCompleteRect.Top() = bHori ? rLine.GetStartPos().Y() - 2 : rLine.GetStartPos().Y();
    aCompleteRect.Bottom() = bHori ? rLine.GetStartPos().Y() + 2 : rLine.GetEndPos().Y();
    rWin.InvertTracking(aCompleteRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);
}

void SvxFrameSelector::Paint( const Rectangle& )
{
    DrawBitmap( Point( 0, 0 ), pImpl->aBackBmp );
    if(HasFocus())
    {
        if(pImpl->aLeftLine.bIsSelected || pImpl->aRightLine.bIsSelected ||
            pImpl->aTopLine.bIsSelected || pImpl->aBottomLine.bIsSelected ||
                pImpl->aHorLine.bIsSelected || pImpl->aVerLine.bIsSelected )
        {
            if(pImpl->aLeftLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aLeftLine, sal_False);
            if(pImpl->aRightLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aRightLine, sal_False);
            if(pImpl->aTopLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aTopLine, sal_True);
            if(pImpl->aBottomLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aBottomLine, sal_True);
            if(pImpl->aHorLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aHorLine, sal_True);
            if(pImpl->aVerLine.bIsSelected)
                lcl_InvertTracking(*this, pImpl->aVerLine, sal_False);
        }
        else
        {
            Size aSize = GetSizePixel();
            aSize.Width() -= 3;
            aSize.Height() -= 3;
            Rectangle aCompleteRect(Point(0, 0), aSize);
            InvertTracking(aCompleteRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);
        }
    }
}

// -----------------------------------------------------------------------

void SvxFrameSelector::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
    {
        CaptureMouse();
    }
}

// -----------------------------------------------------------------------

void SvxFrameSelector::MouseButtonUp( const MouseEvent& rMEvt )
{
    /* Verhalten bei Mausclicks:
     * -------------------------
     * o nur Einfachclicks
     * o Shift-Einfachclick -> Mehrfachselektion
     * o einzelne Linien:
     *   - ist die Linie bereits selektiert -> an/aus-Toggle
     *   - sonst -> an
     * o Kreuzungspunkt vertikal/horizontal (Mitte)
     *   - ist eine oder keine Linie selektiert -> an
     *   - sonst -> an/aus-Toggle beide
     * o Click auf die gleiche Linie -> Toggle SHOW/HIDE/DONT_CARE
     */

    BOOL bWasSelected = IsAnyLineSelected_Impl();
    GrabFocus();
    if ( !bWasSelected )
        pImpl->aTopLine.bIsSelected = FALSE;


    if ( rMEvt.IsLeft() )
    {
        Point aBtnUpPos( rMEvt.GetPosPixel() );

        if ( !bIsClicked && !pImpl->bIsDontCare )
        {
            bIsClicked = TRUE;

            // wenn Linien auf DontCare sind, muessen diese auf HIDE
            // gesetzt werden (ausser der aktuellen Linie)
            typedef ::std::pair<SvxFrameLine*,Rectangle*> TOuterPair;
            TOuterPair eTypes[] =   {
                                        TOuterPair(&pImpl->aLeftLine,&pImpl->aSpotLeft),
                                        TOuterPair(&pImpl->aRightLine,&pImpl->aSpotRight),
                                        TOuterPair(&pImpl->aTopLine,&pImpl->aSpotTop),
                                        TOuterPair(&pImpl->aBottomLine,&pImpl->aSpotBottom),
                                        TOuterPair(&pImpl->aVerLine,&pImpl->aSpotVer),
                                        TOuterPair(&pImpl->aHorLine,&pImpl->aSpotHor)
                                    };

            for (sal_Int32 i=0; i < sizeof(eTypes)/sizeof(TOuterPair); ++i)
            {

                if ( eTypes[i].first->aState == SVX_FRMLINESTATE_DONT_CARE &&
                     !eTypes[i].second->IsInside( aBtnUpPos ) )
                {
                    eTypes[i].first->SetStyle( SvxFrameLine::NO_LINE );
                }
            }
        }

        if ( pImpl->aSpotLeft.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aLeftLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_LEFT );
        }
        else if ( pImpl->aSpotRight.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aRightLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_RIGHT );
        }
        else if ( pImpl->aSpotTop.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aTopLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_TOP );
        }
        else if ( pImpl->aSpotBottom.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aBottomLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_BOTTOM );
        }
        else if ( pImpl->aSpotVer.IsInside( aBtnUpPos ) &&
                  pImpl->aSpotHor.IsInside( aBtnUpPos ) )
        {
            if ( !pImpl->aHorLine.bIsSelected || !pImpl->aVerLine.bIsSelected )
            {
                pImpl->aVerLine.aColor   =
                pImpl->aHorLine.aColor   = pImpl->aCurLineCol;
                pImpl->aVerLine.SetStyle( pImpl->aCurLineStyle );
                pImpl->aHorLine.SetStyle( pImpl->aCurLineStyle );

                if ( !rMEvt.IsShift() )
                    SelectLine( SVX_FRMSELLINE_NONE );

                SelectLine( SVX_FRMSELLINE_VER );
                SelectLine( SVX_FRMSELLINE_HOR );
            }
            else if ( pImpl->aHorLine.bIsSelected && pImpl->aVerLine.bIsSelected )
            {
                if ( !rMEvt.IsShift() )
                    SelectLine( SVX_FRMSELLINE_NONE );
                SelectLine( SVX_FRMSELLINE_VER );
                SelectLine( SVX_FRMSELLINE_HOR );
                LineClicked_Impl( pImpl->aVerLine, TRUE, ( rMEvt.GetClicks() != 1 ) );
                LineClicked_Impl( pImpl->aHorLine, TRUE, ( rMEvt.GetClicks() != 1 ) );
            }
        }
        else if ( pImpl->aSpotVer.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aVerLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_VER );
        }
        else if ( pImpl->aSpotHor.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( pImpl->aHorLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_HOR );
        }
        ShowLines();
        ReleaseMouse();
    }
}

// -----------------------------------------------------------------------

void SvxFrameSelector::LineClicked_Impl( SvxFrameLine& aLine,
                                 BOOL bShiftPressed,
                                 BOOL bDoubleClick )
{
    if ( aLine.bIsSelected )
    {
        int nMod = pImpl->bIsDontCare ? 3 : 2;
        aLine.aState = SvxFrameLineState(
            ( ( (int)aLine.aState ) + ( ( !bDoubleClick ) ? 1 : 2 ) ) % nMod );

        switch ( aLine.aState )
        {
            case SVX_FRMLINESTATE_SHOW:
                aLine.SetStyle( pImpl->aCurLineStyle );
                aLine.aColor = pImpl->aCurLineCol;
                break;
            case SVX_FRMLINESTATE_HIDE:
                aLine.SetStyle( SvxFrameLine::NO_LINE );
                break;
            case SVX_FRMLINESTATE_DONT_CARE:
                aLine.aStyle = SvxFrameLine::THICK_LINE;
                break;
        }
    }
    else
    {
        aLine.aState = SVX_FRMLINESTATE_SHOW;
        aLine.aColor    = pImpl->aCurLineCol;
        aLine.SetStyle( pImpl->aCurLineStyle );
    }

    if ( !bShiftPressed )
        SelectLine( SVX_FRMSELLINE_NONE );
}
// -----------------------------------------------------------------------
BOOL SvxFrameSelector::IsAnyLineSelected_Impl() const
{
    return ( pImpl->aLeftLine.bIsSelected ||
        pImpl->aRightLine.bIsSelected ||
        pImpl->aTopLine.bIsSelected ||
        pImpl->aBottomLine.bIsSelected ||
        pImpl->aHorLine.bIsSelected ||
        pImpl->aVerLine.bIsSelected );
}
// -----------------------------------------------------------------------
SvxFrameSelectorLine SvxFrameSelector::GetFirstSelLineForKey_Impl()const
{
    SvxFrameSelectorLine eRet = SVX_FRMSELLINE_NONE;
    if( pImpl->aTopLine.bIsSelected )
        eRet = SVX_FRMSELLINE_TOP;
    else if( pImpl->aLeftLine.bIsSelected )
        eRet = SVX_FRMSELLINE_LEFT;
    else if( SVX_FRMSELTYPE_TABLE == pImpl->eSel)
    {
        if( pImpl->aVerLine.bIsSelected )
            eRet = SVX_FRMSELLINE_VER;
        else if( pImpl->aHorLine.bIsSelected )
            eRet = SVX_FRMSELLINE_HOR;
    }
    if( SVX_FRMSELLINE_NONE == eRet )
    {
        if(pImpl->aRightLine.bIsSelected )
            eRet = SVX_FRMSELLINE_RIGHT;
        else if( pImpl->aBottomLine.bIsSelected )
            eRet = SVX_FRMSELLINE_BOTTOM;
    }
    return eRet;
}

// -----------------------------------------------------------------------

BOOL    SvxFrameSelector::IsAnyLineSet() const
{
    if( pImpl->aLeftLine.aState    == SVX_FRMLINESTATE_SHOW ||
        pImpl->aRightLine.aState   == SVX_FRMLINESTATE_SHOW ||
        pImpl->aTopLine.aState     == SVX_FRMLINESTATE_SHOW ||
        pImpl->aBottomLine.aState  == SVX_FRMLINESTATE_SHOW ||
        pImpl->aHorLine.aState     == SVX_FRMLINESTATE_SHOW ||
        pImpl->aVerLine.aState     == SVX_FRMLINESTATE_SHOW )
        return TRUE;
    else
        return FALSE;
}
/* -----------------------------25.01.2002 10:25------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        pImpl->SetHC( this );
        InitBitmap_Impl();
        ShowLines();
    }
}
/* -----------------------------20.03.2002 14:31------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameSelectorLine lcl_GetNextStep(
    const SvxFrameSelectorLine eCurLine, USHORT nKeyCode, BOOL bIsTable)
{
    //the first entry in the array contains the last reachable line
    //if the current line is found at the beginning of the
    static const SvxFrameSelectorLine aLeftArr[]  =
        {SVX_FRMSELLINE_LEFT, SVX_FRMSELLINE_VER, SVX_FRMSELLINE_BOTTOM,
        SVX_FRMSELLINE_HOR, SVX_FRMSELLINE_TOP, SVX_FRMSELLINE_RIGHT};
    static const SvxFrameSelectorLine aRightArr[]  =
        {SVX_FRMSELLINE_RIGHT, SVX_FRMSELLINE_TOP, SVX_FRMSELLINE_HOR,
        SVX_FRMSELLINE_BOTTOM, SVX_FRMSELLINE_VER, SVX_FRMSELLINE_LEFT};
    static const SvxFrameSelectorLine aUpArr[]  =
        {SVX_FRMSELLINE_TOP, SVX_FRMSELLINE_VER, SVX_FRMSELLINE_RIGHT,
        SVX_FRMSELLINE_HOR, SVX_FRMSELLINE_LEFT, SVX_FRMSELLINE_BOTTOM};
    static const SvxFrameSelectorLine aDownArr[]  =
        {SVX_FRMSELLINE_BOTTOM, SVX_FRMSELLINE_LEFT, SVX_FRMSELLINE_HOR,
        SVX_FRMSELLINE_RIGHT, SVX_FRMSELLINE_VER, SVX_FRMSELLINE_TOP};
    const SvxFrameSelectorLine* pMoveArray = 0;
    BOOL bHLines = TRUE;
    switch(nKeyCode)
    {
        case KEY_UP : pMoveArray = aUpArr;break;
        case KEY_DOWN :pMoveArray = aDownArr;break;
        case KEY_LEFT :pMoveArray = aLeftArr; bHLines = FALSE;   break;
        case KEY_RIGHT :pMoveArray = aRightArr; bHLines = FALSE; break;
    }
    SvxFrameSelectorLine eRetLine = eCurLine;
    if(pMoveArray)
    {
        for(USHORT i = 0; i < SVX_FRMSELLINE_VER; i++)
        {
            if(eCurLine == pMoveArray[i])
            {
                if(i)
                {
                    eRetLine = pMoveArray[--i];
                    //make sure that no non-existing lines can be reached
                    //and vertical travel selects horizontal lines and vice versa
                    while((!bIsTable &&
                        (SVX_FRMSELLINE_VER == eRetLine || SVX_FRMSELLINE_HOR == eRetLine))||
                        (bHLines && (SVX_FRMSELLINE_VER  == eRetLine||  SVX_FRMSELLINE_LEFT == eRetLine || SVX_FRMSELLINE_RIGHT == eRetLine))||
                        (!bHLines && (SVX_FRMSELLINE_TOP  == eRetLine||  SVX_FRMSELLINE_BOTTOM == eRetLine || SVX_FRMSELLINE_HOR == eRetLine)))
                        eRetLine = pMoveArray[--i];
                }
                break;
            }
        }
    }
   return eRetLine;
}
/* -----------------------------17.07.2002 15:33-by LLA-----------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::ToggleOneLine(SvxFrameLine &aCurLine)
{
    // PRE: an initialised SvxFrameLine as reference, we will change it!

    if (aCurLine.GetState() == SVX_FRMLINESTATE_HIDE)
    {
        aCurLine.SetState(SVX_FRMLINESTATE_SHOW);
        //  aCurLine.aStyle = SvxFrameLine::THIN_LINE;
        aCurLine.SetStyle( pImpl->aCurLineStyle );
        aCurLine.aColor = pImpl->aCurLineCol;
    }
    else if (aCurLine.GetState() == SVX_FRMLINESTATE_SHOW)
    {
        aCurLine.SetState(SVX_FRMLINESTATE_HIDE);
        aCurLine.SetStyle( SvxFrameLine::NO_LINE );
    }
    // No need to handle SVX_FRMLINESTATE_DONT_CARE
    // else
    // {
    // }
}

void SvxFrameSelector::ToggleAllSelectedLines()
{

    if( pImpl->aTopLine.bIsSelected )    ToggleOneLine(pImpl->aTopLine);
    if( pImpl->aLeftLine.bIsSelected )   ToggleOneLine(pImpl->aLeftLine);
    if( pImpl->aRightLine.bIsSelected )  ToggleOneLine(pImpl->aRightLine);
    if( pImpl->aBottomLine.bIsSelected ) ToggleOneLine(pImpl->aBottomLine);

    // LLA: special extended?
    if( SVX_FRMSELTYPE_TABLE == pImpl->eSel)
    {
        if( pImpl->aVerLine.bIsSelected ) ToggleOneLine(pImpl->aVerLine);
        if( pImpl->aHorLine.bIsSelected ) ToggleOneLine(pImpl->aHorLine);
    }
}

/* -----------------------------01.02.2002 13:50------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bHandled = sal_False;
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    if ( !aKeyCode.GetModifier())
    {
        USHORT nCode = aKeyCode.GetCode();
        switch(nCode)
        {
            case KEY_SPACE:
                //toggle current line type
                // LineClicked_Impl(GetLine(eCurLine), FALSE, FALSE);
                // SelectLine(eCurLine, TRUE);
                ToggleAllSelectedLines();
                ShowLines();
                bHandled = sal_True;
            break;
            case KEY_UP :
            case KEY_DOWN :
            case KEY_LEFT :
            case KEY_RIGHT :
            {
                SvxFrameSelectorLine eCurLine = GetFirstSelLineForKey_Impl();

                BOOL bIsTable = SVX_FRMSELTYPE_TABLE == pImpl->eSel;
                SvxFrameSelectorLine eFoundLine = lcl_GetNextStep(eCurLine, nCode, bIsTable);
                if(eFoundLine != eCurLine)
                {
                    SelectLine( SVX_FRMSELLINE_NONE );
                    SelectLine(eFoundLine, TRUE);
                }
            }
            break;
        }
    }
    if(!bHandled)
        Window::KeyInput(rKEvt);
}
/* -----------------------------01.02.2002 15:34------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::GetFocus()
{
    if(!IsAnyLineSelected_Impl())
        SelectLine( SVX_FRMSELLINE_TOP, true );
    Invalidate();
    if(pImpl->xAccess.is())
        pImpl->pAccess->NotifyFocusListeners(sal_True);
    Control::GetFocus();
}
/* -----------------------------01.02.2002 15:34------------------------------

 ---------------------------------------------------------------------------*/
void SvxFrameSelector::LoseFocus()
{
    Invalidate();
    if(pImpl->xAccess.is())
        pImpl->pAccess->NotifyFocusListeners(sal_True);
    Control::LoseFocus();
}
/* -----------------------------01.02.2002 16:54------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameLine&       SvxFrameSelector::GetLine(SvxFrameSelectorLine eWhich)
{
    SvxFrameLine* pRet = 0;
    switch(eWhich)
    {
        case SVX_FRMSELLINE_TOP:        pRet = &pImpl->aTopLine;    break;
        case SVX_FRMSELLINE_BOTTOM:     pRet = &pImpl->aBottomLine; break;
        case SVX_FRMSELLINE_LEFT:       pRet = &pImpl->aLeftLine;   break;
        case SVX_FRMSELLINE_RIGHT:      pRet = &pImpl->aRightLine;  break;
        case SVX_FRMSELLINE_HOR:        pRet = &pImpl->aHorLine;    break;
        case SVX_FRMSELLINE_NONE:
                DBG_ERROR("wrong line selected!")
        case SVX_FRMSELLINE_VER:        pRet = &pImpl->aVerLine;    break;
    }
    return *pRet;
}
/* -----------------------------04.02.2002 14:14------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< XAccessible > SvxFrameSelector::CreateAccessible()
{
    if(!pImpl->xAccess.is())
        pImpl->xAccess = pImpl->pAccess =
            new SvxFrameSelectorAccessible_Impl(*this, SVX_FRMSELLINE_NONE);
    return pImpl->xAccess;
}
/* -----------------------------05.02.2002 15:46------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameSelector_Impl::SvxFrameSelector_Impl( const SvxFrameSelector* pThis ) :
        eSel            ( SVX_FRMSELTYPE_PARAGRAPH ),
        aCurLineStyle ( SvxFrameLine::NO_LINE ),
        aCurLineCol   ( COL_BLACK ),
        pAccess(0),
        bIsDontCare     ( sal_False )
{
    for(sal_Int16 i = 0; i < 6; i++)
        pChildren[i] = 0;

    SetHC( pThis );
}
/* -----------------------------05.02.2002 15:47------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameSelector_Impl::~SvxFrameSelector_Impl()
{
    if(pAccess)
        pAccess->Invalidate();
    for(sal_Int16 i = 0; i < 6; i++)
        if(pChildren[i])
            pChildren[i]->Invalidate();
}
/* -----------------------------05.02.2002 15:47------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< XAccessible >
    SvxFrameSelector_Impl::GetChildAccessible(
        SvxFrameSelector& rFrmSel, SvxFrameSelectorLine eWhich)
{
    DBG_ASSERT(eWhich >= SVX_FRMSELLINE_LEFT && eWhich <= SVX_FRMSELLINE_VER, "wrong line type")
    if(!pChildren[eWhich -1])
        xChildren[eWhich -1] = pChildren[eWhich -1] =
            new SvxFrameSelectorAccessible_Impl(rFrmSel, eWhich);
    return pChildren[eWhich -1];
}
/* -----------------------------13.02.2002 13:28------------------------------

 ---------------------------------------------------------------------------*/
const Rectangle& SvxFrameSelector_Impl::GetLineSpot(SvxFrameSelectorLine eWhich) const
{
    const Rectangle* pRet = 0;
    switch(eWhich)
    {
        case SVX_FRMSELLINE_LEFT    : pRet = &aSpotLeft;      break;
        case SVX_FRMSELLINE_RIGHT   : pRet = &aSpotRight;     break;
        case SVX_FRMSELLINE_TOP     : pRet = &aSpotTop;       break;
        case SVX_FRMSELLINE_BOTTOM  : pRet = &aSpotBottom;    break;
        case SVX_FRMSELLINE_HOR     : pRet = &aSpotHor;       break;
        case SVX_FRMSELLINE_VER     : pRet = &aSpotVer;       break;
    };
    return *pRet;
}


void SvxFrameSelector_Impl::SetHC( const SvxFrameSelector* pCntrl )
{
    if( pCntrl && pCntrl->GetDisplayBackgroundColor().IsDark() )
    {
        bHC = TRUE;
        aPaintLineCol = pCntrl->GetSettings().GetStyleSettings().GetLabelTextColor();
    }
    else
        bHC = FALSE;
}
