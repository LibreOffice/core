/*************************************************************************
 *
 *  $RCSfile: unomod.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _UNOMOD_HXX
#include <unomod.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX //autogen
#include <svtools/itemprop.hxx>
#endif
#ifndef _PRTOPT_HXX //autogen
#include <prtopt.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

/******************************************************************
 * SwXModule
 ******************************************************************/
/* -----------------30.03.99 15:10-------------------
 *
 * --------------------------------------------------*/
Reference< uno::XInterface >  SAL_CALL SwXModule_CreateInstance(const Reference< lang::XMultiServiceFactory > & )
{
    static Reference< uno::XInterface >  xModule = (cppu::OWeakObject*)new SwXModule();;
    return xModule;
}
/*-- 17.12.98 12:19:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXModule::SwXModule() :
    pxViewSettings(0),
    pxPrintSettings(0)
{
}
/*-- 17.12.98 12:19:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXModule::~SwXModule()
{
    delete pxViewSettings;
    delete pxPrintSettings;
}
/*-- 17.12.98 12:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySet >  SwXModule::getViewSettings(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(!pxViewSettings)
    {
        ((SwXModule*)this)->pxViewSettings = new Reference< beans::XPropertySet > ;
        DBG_ERROR("Web oder Text?")
        *pxViewSettings = new SwXViewSettings(sal_False, 0);
    }
    return *pxViewSettings;
}
/*-- 17.12.98 12:19:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySet >  SwXModule::getPrintSettings(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if(!pxPrintSettings)
    {
        ((SwXModule*)this)->pxPrintSettings = new Reference< beans::XPropertySet > ;
        DBG_ERROR("Web oder Text?")
        *pxPrintSettings = new SwXPrintSettings(sal_False);
    }
    return *pxPrintSettings;
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXModule::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXModule");
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXModule::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.Module") == rServiceName;
}
/* -----------------------------06.04.00 10:59--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXModule::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Module");
    return aRet;
}

/******************************************************************
 * SwXPrintSettings
 ******************************************************************/

/*-- 17.12.98 12:54:04---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPrintSettings::SwXPrintSettings(sal_Bool bWebView) :
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PRINT_SETTINGS)),
    bWeb(bWebView)
{

}
/*-- 17.12.98 12:54:05---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPrintSettings::~SwXPrintSettings()
{

}
/*-- 17.12.98 12:54:06---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySetInfo >  SwXPrintSettings::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    static Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo(_pMap);
    return aRef;
}
/*-- 17.12.98 12:54:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPrintSettings::setPropertyValue(const OUString& rPropertyName,
                    const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bVal = sal_False;

    if(COMPARE_EQUAL != rPropertyName.compareToAscii(UNO_NAME_PRINT_ANNOTATION_MODE))
        bVal = *(sal_Bool*)aValue.getValue();

    SwPrintOptions* pPrtOpt = SW_MOD()->GetPrtOptions(bWeb);
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if(pCur)
        switch( pCur->nWID )
        {
            case WID_PRTSET_LEFT_PAGES     : pPrtOpt->SetPrintLeftPage(bVal);       break;
            case WID_PRTSET_RIGHT_PAGES    : pPrtOpt->SetPrintRightPage(bVal);  break;
            case WID_PRTSET_REVERSED       : pPrtOpt->SetPrintReverse(bVal);        break;
            case WID_PRTSET_PROSPECT       : pPrtOpt->SetPrintProspect(bVal);  break;
            case WID_PRTSET_GRAPHICS       : pPrtOpt->SetPrintGraphic(bVal);  break;
            case WID_PRTSET_TABLES         : pPrtOpt->SetPrintTable(bVal);  break;
            case WID_PRTSET_DRAWINGS       : pPrtOpt->SetPrintDraw(bVal);  break;
            case WID_PRTSET_CONTROLS       : pPrtOpt->SetPrintControl(bVal);  break;
            case WID_PRTSET_PAGE_BACKGROUND: pPrtOpt->SetPrintPageBackground(bVal);  break;
            case WID_PRTSET_BLACK_FONTS    : pPrtOpt->SetPrintBlackFont(bVal);  break;
            case WID_PRTSET_ANNOTATION_MODE:
            {
                sal_Int16 nVal;
                    aValue >>= nVal;
                if(nVal <= text::NotePrintMode_PAGE_END)
                    pPrtOpt->SetPrintPostIts(nVal);
                else
                    throw lang::IllegalArgumentException();
            }
            break;
            default: DBG_ERROR("Diese Id gibt's nicht!");
        }
    else
        throw beans::UnknownPropertyException();
}
/*-- 17.12.98 12:54:06---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXPrintSettings::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwPrintOptions* pPrtOpt = SW_MOD()->GetPrtOptions(bWeb);
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if(pCur)
    {
        sal_Bool bBool = TRUE;
        sal_Bool bBoolVal;
        switch( pCur->nWID )
        {
            case WID_PRTSET_LEFT_PAGES      : bBoolVal = pPrtOpt->IsPrintLeftPage();        break;
            case WID_PRTSET_RIGHT_PAGES     : bBoolVal = pPrtOpt->IsPrintRightPage();   break;
            case WID_PRTSET_REVERSED        : bBoolVal = pPrtOpt->IsPrintReverse();     break;
            case WID_PRTSET_PROSPECT       : bBoolVal = bBoolVal = pPrtOpt->IsPrintProspect();  break;
            case WID_PRTSET_GRAPHICS       : bBoolVal = pPrtOpt->IsPrintGraphic();  break;
            case WID_PRTSET_TABLES         : bBoolVal = pPrtOpt->IsPrintTable();  break;
            case WID_PRTSET_DRAWINGS       : bBoolVal = pPrtOpt->IsPrintDraw();  break;
            case WID_PRTSET_CONTROLS       : bBoolVal = pPrtOpt->IsPrintControl();  break;
            case WID_PRTSET_PAGE_BACKGROUND: bBoolVal = pPrtOpt->IsPrintPageBackground();  break;
            case WID_PRTSET_BLACK_FONTS    : bBoolVal = pPrtOpt->IsPrintBlackFont();  break;
            case WID_PRTSET_ANNOTATION_MODE: bBool = FALSE; aRet <<= (sal_Int16)pPrtOpt->GetPrintPostIts();  break;
            default: DBG_ERROR("Diese Id gibt's nicht!");
        }
        if(bBool)
            aRet.setValue(&bBoolVal, ::getBooleanCppuType());
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}
/*-- 17.12.98 12:54:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPrintSettings::addPropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 12:54:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPrintSettings::removePropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 12:54:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPrintSettings::addVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 12:54:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPrintSettings::removeVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXPrintSettings::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXPrintSettings");
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXPrintSettings::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.PrintSettings") == rServiceName;
}
/* -----------------------------06.04.00 11:02--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXPrintSettings::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.PrintSettings");
    return aRet;
}


/******************************************************************
 *
 ******************************************************************/
/*-- 18.12.98 11:01:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXViewSettings::SwXViewSettings(sal_Bool bWebView, SwView* pVw) :
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_VIEW_SETTINGS)),
    pView(pVw),
    bWeb(bWebView),
    bObjectValid(sal_True)
{

}
/*-- 18.12.98 11:01:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXViewSettings::~SwXViewSettings()
{

}
/*-- 18.12.98 11:01:11---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySetInfo >  SwXViewSettings::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo(_pMap);
    return aRef;
}
/*-- 18.12.98 11:01:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXViewSettings::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bApply = sal_True;
    sal_Bool bApplyZoom = sal_False;

    const SwViewOption* pVOpt = 0;
    if(pView)
    {
        if(!IsValid())
            return;
        pVOpt = pView->GetWrtShell().GetViewOptions();
    }
    else
    {
        pVOpt = SW_MOD()->GetViewOption(bWeb);
    }
    SwViewOption aVOpt(*pVOpt);
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(
                                            _pMap, rPropertyName);
    if(pCur)
    {
        sal_Bool bVal = WID_VIEWSET_ZOOM != pCur->nWID ?
            *(sal_Bool*)aValue.getValue() : sal_False;
        // the API flag should not be set to the application's view settings
        if(pView)
            aVOpt.SetStarOneSetting(sal_True);
        switch( pCur->nWID )
        {
            case  WID_VIEWSET_HRULER :                  aVOpt.SetViewTabwin(bVal);  break;
            case  WID_VIEWSET_VRULER :                  aVOpt.SetViewVLin(bVal);break;
            case  WID_VIEWSET_HSCROLL:                  aVOpt.SetViewHScrollBar(bVal);break;
            case  WID_VIEWSET_VSCROLL:                  aVOpt.SetViewVScrollBar(bVal);break;
            case  WID_VIEWSET_GRAPHICS              :   aVOpt.SetGraphic(bVal);break;
            case  WID_VIEWSET_TABLES                :   aVOpt.SetTable(bVal);   break;
            case  WID_VIEWSET_DRAWINGS              :   aVOpt.SetDraw(bVal);    break;
            case  WID_VIEWSET_FIELD_COMMANDS        :   aVOpt.SetFldName(bVal); break;
            case  WID_VIEWSET_ANNOTATIONS           :   aVOpt.SetPostIts(bVal); break;
            case  WID_VIEWSET_INDEX_MARK_BACKGROUND :   aVOpt.SetTox(bVal); break;
            case  WID_VIEWSET_FOOTNOTE_BACKGROUND   :   aVOpt.SetFootNote(bVal);    break;
            case  WID_VIEWSET_TEXT_FIELD_BACKGROUND :   aVOpt.SetField(bVal);   break;
            case  WID_VIEWSET_PARA_BREAKS           :   aVOpt.SetParagraph(bVal);   break;
            case  WID_VIEWSET_SOFT_HYPHENS          :   aVOpt.SetSoftHyph(bVal);    break;
            case  WID_VIEWSET_SPACES                :   aVOpt.SetBlank(bVal);   break;
            case  WID_VIEWSET_PROTECTED_SPACES      :   aVOpt.SetHardBlank(bVal);   break;
            case  WID_VIEWSET_TABSTOPS              :   aVOpt.SetTab(bVal); break;
            case  WID_VIEWSET_BREAKS                :   aVOpt.SetLineBreak(bVal); break;
            case  WID_VIEWSET_HIDDEN_TEXT           :   aVOpt.SetHidden(bVal);  break;
            case  WID_VIEWSET_HIDDEN_PARAGRAPHS     :   aVOpt.SetShowHiddenPara(bVal);  break;
            case  WID_VIEWSET_TABLE_BOUNDARIES      :   aVOpt.SetSubsTable(bVal);   break;
            case  WID_VIEWSET_TEXT_BOUNDARIES       :   aVOpt.SetSubsLines(bVal);   break;
            case  WID_VIEWSET_SMOOTH_SCROLLING      :   aVOpt.SetSmoothScroll(bVal);    break;
            case  WID_VIEWSET_SOLID_MARK_HANDLES    :   aVOpt.SetSolidMarkHdl(bVal);    break;
            case  WID_VIEWSET_ZOOM                  :
            {
                sal_Int16 nZoom = *(sal_Int16*)aValue.getValue();
                if(nZoom > 1000 || nZoom < 5)
                    throw lang::IllegalArgumentException();
                aVOpt.SetZoom((sal_uInt16)nZoom);
                bApplyZoom = sal_True;
            }
            break;
            case WID_VIEWSET_ZOOM_TYPE:
            {
                sal_Int16 nZoom = *(sal_Int16*)aValue.getValue();
                SvxZoomType eZoom = (SvxZoomType)USHRT_MAX;
                switch(nZoom)
                {
                    case /*DocumentZoomType_OPTIMAL       */0:
                        eZoom = SVX_ZOOM_OPTIMAL;
                    break;
                    case /*DocumentZoomType_PAGE_WIDTH  */  1:
                        eZoom = SVX_ZOOM_PAGEWIDTH;
                    break;
                    case /*DocumentZoomType_ENTIRE_PAGE */  2:
                        eZoom = SVX_ZOOM_WHOLEPAGE;
                    break;
                    case /*DocumentZoomType_BY_VALUE    */  3:
                        eZoom = SVX_ZOOM_PERCENT;
                    break;
                }
                if(eZoom < USHRT_MAX)
                {
                    aVOpt.SetZoomType( eZoom );
                    bApplyZoom = sal_True;
                }
            }
            break;
            case WID_VIEWSET_ONLINE_LAYOUT :
            {
                bApply = sal_False;
                if(pView)
                    pView->GetDocShell()->ToggleBrowserMode(bVal, pView );
            }
            break;
            default:
                bApply = sal_False;
                DBG_ERROR("Diese Id gibt's nicht!")
        }
    }
    else
        throw beans::UnknownPropertyException();
    if(bApplyZoom && pView)
    {
        pView->SetZoom( (SvxZoomType)aVOpt.GetZoomType(), aVOpt.GetZoom(), sal_True );
    }
    else if(bApply)
        SW_MOD()->ApplyUsrPref(aVOpt, pView, pView ? VIEWOPT_DEST_VIEW_ONLY : bWeb ? VIEWOPT_DEST_WEB : VIEWOPT_DEST_TEXT );

}
/*-- 18.12.98 11:01:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXViewSettings::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    const SwViewOption* pVOpt = 0;
    if(pView)
    {
        if(!IsValid())
            return uno::Any();
        pVOpt = pView->GetWrtShell().GetViewOptions();
    }
    else
        pVOpt = SW_MOD()->GetViewOption(bWeb);
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if(pCur)
    {
        sal_Bool bBool = TRUE;
        sal_Bool bBoolVal;
        switch( pCur->nWID )
        {
            case  WID_VIEWSET_HRULER :                  bBoolVal = pVOpt->IsViewTabwin();   break;
            case  WID_VIEWSET_VRULER :                  bBoolVal = pVOpt->IsViewVLin();break;
            case  WID_VIEWSET_HSCROLL:                  bBoolVal = pVOpt->IsViewHScrollBar();break;
            case  WID_VIEWSET_VSCROLL:                  bBoolVal = pVOpt->IsViewVScrollBar();break;
            case  WID_VIEWSET_GRAPHICS              :   bBoolVal = pVOpt->IsGraphic();break;
            case  WID_VIEWSET_TABLES                :   bBoolVal = pVOpt->IsTable();    break;
            case  WID_VIEWSET_DRAWINGS              :   bBoolVal = pVOpt->IsDraw(); break;
            case  WID_VIEWSET_FIELD_COMMANDS        :   bBoolVal = pVOpt->IsFldName();  break;
            case  WID_VIEWSET_ANNOTATIONS           :   bBoolVal = pVOpt->IsPostIts();  break;
            case  WID_VIEWSET_INDEX_MARK_BACKGROUND :   bBoolVal = pVOpt->IsTox();  break;
            case  WID_VIEWSET_FOOTNOTE_BACKGROUND   :   bBoolVal = pVOpt->IsFootNote(); break;
            case  WID_VIEWSET_TEXT_FIELD_BACKGROUND :   bBoolVal = pVOpt->IsField();    break;
            case  WID_VIEWSET_PARA_BREAKS           :   bBoolVal = pVOpt->IsParagraph(sal_True);    break;
            case  WID_VIEWSET_SOFT_HYPHENS          :   bBoolVal = pVOpt->IsSoftHyph(); break;
            case  WID_VIEWSET_SPACES                :   bBoolVal = pVOpt->IsBlank(sal_True);    break;
            case  WID_VIEWSET_PROTECTED_SPACES      :   bBoolVal = pVOpt->IsHardBlank();    break;
            case  WID_VIEWSET_TABSTOPS              :   bBoolVal = pVOpt->IsTab(sal_True);  break;
            case  WID_VIEWSET_BREAKS                :   bBoolVal = pVOpt->IsLineBreak(sal_True); break;
            case  WID_VIEWSET_HIDDEN_TEXT           :   bBoolVal = pVOpt->IsHidden();   break;
            case  WID_VIEWSET_HIDDEN_PARAGRAPHS     :   bBoolVal = pVOpt->IsShowHiddenPara();   break;
            case  WID_VIEWSET_TABLE_BOUNDARIES      :   bBoolVal = pVOpt->IsSubsTable();    break;
            case  WID_VIEWSET_TEXT_BOUNDARIES       :   bBoolVal = pVOpt->IsSubsLines();    break;
            case  WID_VIEWSET_SMOOTH_SCROLLING      :   bBoolVal = pVOpt->IsSmoothScroll(); break;
            case  WID_VIEWSET_SOLID_MARK_HANDLES    :   bBoolVal = pVOpt->IsSolidMarkHdl(); break;
            case  WID_VIEWSET_ZOOM                  :
                    bBool = FALSE;
                    aRet <<= (sal_Int16)pVOpt->GetZoom();
            break;
            case WID_VIEWSET_ZOOM_TYPE:
            {
                bBool = FALSE;
                sal_Int16 nRet;
                switch(pVOpt->GetZoomType())
                {
                    case SVX_ZOOM_OPTIMAL:
                        nRet = /*DocumentZoomType_OPTIMAL*/ 0;
                    break;
                    case SVX_ZOOM_PAGEWIDTH:
                        nRet = /*DocumentZoomType_PAGE_WIDTH    */1;
                    break;
                    case SVX_ZOOM_WHOLEPAGE:
                        nRet = /*DocumentZoomType_ENTIRE_PAGE */  2;
                    break;
                    case SVX_ZOOM_PERCENT:
                        nRet = /*DocumentZoomType_BY_VALUE  */  3;
                    break;
                }
                aRet <<= nRet;
            }
            break;
            case WID_VIEWSET_ONLINE_LAYOUT:
                if(pView)
                    bBoolVal = pView->GetWrtShell().GetDoc()->IsBrowseMode();
            break;
            default: DBG_ERROR("Diese Id gibt's nicht!");
        }
        if(bBool)
            aRet.setValue(&bBoolVal, ::getBooleanCppuType());
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}
/*-- 18.12.98 11:01:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXViewSettings::addPropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 18.12.98 11:01:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXViewSettings::removePropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 18.12.98 11:01:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXViewSettings::addVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 18.12.98 11:01:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXViewSettings::removeVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
OUString SwXViewSettings::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXViewSettings");
}
BOOL SwXViewSettings::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.ViewSettings") == rServiceName;
}
Sequence< OUString > SwXViewSettings::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ViewSettings");
    return aRet;
}

/******************************************************************
 *
 ******************************************************************/

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.40  2000/09/18 16:06:15  willem.vandorp
    OpenOffice header added.

    Revision 1.39  2000/05/31 12:02:13  os
    SAL_CALL

    Revision 1.38  2000/04/11 08:05:00  os
    UNICODE

    Revision 1.37  2000/04/05 10:03:10  os
    #67584# scroll bar settings in read-only documents corrected

    Revision 1.36  2000/03/27 10:36:31  os
    UNO III

    Revision 1.35  2000/03/21 15:39:44  os
    UNOIII

    Revision 1.34  2000/02/11 14:59:53  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.33  1999/12/16 14:18:17  hjs
    includes

    Revision 1.32  1999/12/14 16:11:33  os
    #70234# Set Browser Mode via API

    Revision 1.31  1999/11/22 10:37:38  os
    missing headers added

    Revision 1.30  1999/11/19 16:38:03  os
    modules renamed

    Revision 1.29  1999/07/20 07:48:06  OS
    #67585# ZoomValue/ZoomType :SetZoom changed


      Rev 1.28   20 Jul 1999 09:48:06   OS
   #67585# ZoomValue/ZoomType :SetZoom changed

      Rev 1.27   20 Jul 1999 09:04:56   OS
   #67585# ZoomType/ZoomValue

      Rev 1.26   13 Jul 1999 08:50:56   OS
   #67584# Scrollbar settings via StarOne; #67585# Zoom-Property

      Rev 1.25   22 Apr 1999 16:09:04   OS
   #65194# throw -> throw

      Rev 1.24   22 Apr 1999 15:28:52   OS
   #65124# not implemented - nur noch DBG_WARNING

      Rev 1.23   30 Mar 1999 15:28:12   OS
   #63930# Services am ProcessServiceManager anmelden

      Rev 1.22   15 Mar 1999 14:38:24   OS
   #62845# Makro fuer ServiceInfo jetzt auch fuer OS/2

      Rev 1.21   12 Mar 1999 09:57:04   OS
   #62845# lang::XServiceInfo impl.

      Rev 1.20   09 Mar 1999 12:38:34   OS
   #62008# Solar-Mutex

      Rev 1.19   05 Mar 1999 14:27:26   OS
   #62874# Schreibfehler

      Rev 1.18   04 Mar 1999 15:04:12   OS
   #62191# UINT nicht mehr verwenden

      Rev 1.17   23 Feb 1999 16:19:56   OS
   #62281# UsrPrefs per UNO nur auf aktuelle sdbcx::View anwenden

      Rev 1.16   28 Jan 1999 16:27:50   OS
   #56371# keine Objekte fuer DEBUG anlegen

      Rev 1.15   27 Jan 1999 12:06:06   OS
   #56371# TF_ONE51

      Rev 1.14   18 Dec 1998 11:40:32   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.13   10 Dec 1998 15:54:04   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.12   23 Nov 1998 17:37:22   JP
   Bug #59754#: TerminateHdl wird nicht mehr fuers Clipboard benoetigt

      Rev 1.11   04 Nov 1998 10:24:56   OS
   #58315# im disposing beim frame::XDesktop abmelden

      Rev 1.10   01 Oct 1998 11:40:24   HR
   Typo beseitigt

      Rev 1.9   24 Sep 1998 13:33:14   OS
   #52654# #56685# frame::XTerminateListener fuer die Anmeldung an der Application

      Rev 1.8   10 Jul 1998 18:10:04   OS
   PropertySetInfo und IdlClass static

      Rev 1.7   17 Jun 1998 11:40:12   MH
   tmp fuer ICC

      Rev 1.6   04 Jun 1998 09:40:30   OS
// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//   getIdlClasses


      Rev 1.5   14 May 1998 17:49:52   OS
   div. Namensaenderungen

      Rev 1.4   09 Apr 1998 15:10:34   OS
   Uno-Umstellung

      Rev 1.3   08 Apr 1998 12:51:08   OS
   text::ViewSettings auch fuer die TextView

      Rev 1.2   16 Mar 1998 12:38:54   OS
   ; im default

      Rev 1.1   16 Mar 1998 10:31:08   OS
   sdbcx::View- und PrintSettings vervollstaendigt

      Rev 1.0   15 Mar 1998 15:22:24   OS
   Initial revision.


------------------------------------------------------------------------*/

