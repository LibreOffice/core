/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoredlines.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:33:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOREDLINES_HXX
#include <unoredlines.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#include "poolfmt.hxx"
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;
using ::rtl::OUString;

/*-- 11.01.01 15:28:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::SwXRedlines(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::~SwXRedlines()
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXRedlines::getCount(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count();
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXRedlines::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    uno::Any aRet;
    if(rRedTbl.Count() > nIndex && nIndex >= 0)
    {
        uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject((USHORT)nIndex), *GetDoc() );
        aRet <<= xRet;
    }
    else
        throw lang::IndexOutOfBoundsException();
    return aRet;
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXRedlines::createEnumeration(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXRedlines::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<beans::XPropertySet>*)0);
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXRedlines::hasElements(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count() > 0;
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXRedlines::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlines");
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlines::supportsService(const rtl::OUString& /*ServiceName*/)
    throw( uno::RuntimeException )
{
    DBG_ERROR("not implemented")
    return FALSE;
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXRedlines::getSupportedServiceNames(void)
    throw( uno::RuntimeException )
{
    DBG_ERROR("not implemented")
    return uno::Sequence< OUString >();
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::XPropertySet*    SwXRedlines::GetObject( SwRedline& rRedline, SwDoc& rDoc )
{
    SwPageDesc* pStdDesc = rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    SwClientIter aIter(*pStdDesc);
    SwXRedline* pxRedline = (SwXRedline*)aIter.First( TYPE( SwXRedline ));
    while(pxRedline)
    {
        if(pxRedline->GetRedline() == &rRedline)
            break;
        pxRedline = (SwXRedline*)aIter.Next();
    }
    if( !pxRedline )
        pxRedline = new SwXRedline(rRedline, rDoc);
    return pxRedline;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlineEnumeration::SwXRedlineEnumeration(SwDoc& rDoc) :
    pDoc(&rDoc),
    nCurrentIndex(0)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlineEnumeration::~SwXRedlineEnumeration()
{
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlineEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    if(!pDoc)
        throw uno::RuntimeException();
    return pDoc->GetRedlineTbl().Count() > nCurrentIndex;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXRedlineEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    if(!pDoc)
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = pDoc->GetRedlineTbl();
    if(!(rRedTbl.Count() > nCurrentIndex))
        throw container::NoSuchElementException();
    uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject(nCurrentIndex++), *pDoc );
    uno::Any aRet;
    aRet <<= xRet;
    return aRet;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString SwXRedlineEnumeration::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlineEnumeration");
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlineEnumeration::supportsService(const rtl::OUString& /*ServiceName*/) throw( uno::RuntimeException )
{
    return FALSE;
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return uno::Sequence< OUString >();
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedlineEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDoc = 0;
}
