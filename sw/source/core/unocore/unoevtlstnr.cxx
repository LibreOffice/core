/*************************************************************************
 *
 *  $RCSfile: unoevtlstnr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:42:44 $
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


#pragma hdrstop

#ifndef _UNOEVTLSTNR_HXX
#include <unoevtlstnr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

/* -----------------22.04.99 11:24-------------------
 *
 * --------------------------------------------------*/
SV_IMPL_PTRARR(SwEvtLstnrArray, XEventListenerPtr);

/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwEventListenerContainer::SwEventListenerContainer( uno::XInterface* pxParent) :
    pListenerArr(0),
    pxParent(pxParent)
{
}
/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwEventListenerContainer::~SwEventListenerContainer()
{
    if(pListenerArr && pListenerArr->Count())
    {
        pListenerArr->DeleteAndDestroy(0, pListenerArr->Count());
    }
    delete pListenerArr;
}
/*-- 22.04.99 11:24:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwEventListenerContainer::AddListener(const uno::Reference< lang::XEventListener > & rxListener)
{
    if(!pListenerArr)
        pListenerArr = new SwEvtLstnrArray;
    uno::Reference< lang::XEventListener > * pInsert = new uno::Reference< lang::XEventListener > ;
    *pInsert = rxListener;
    pListenerArr->Insert(pInsert, pListenerArr->Count());
}
/*-- 22.04.99 11:25:00---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SwEventListenerContainer::RemoveListener(const uno::Reference< lang::XEventListener > & rxListener)
{
    if(!pListenerArr)
        return sal_False;
    else
    {
         lang::XEventListener* pLeft = rxListener.get();
        for(sal_uInt16 i = 0; i < pListenerArr->Count(); i++)
        {
            XEventListenerPtr pElem = pListenerArr->GetObject(i);
             lang::XEventListener* pRight = pElem->get();
            if(pLeft == pRight)
            {
                pListenerArr->Remove(i);
                delete pElem;
                return sal_True;
            }
        }
    }
    return sal_False;
}
/*-- 22.04.99 11:25:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwEventListenerContainer::Disposing()
{
    if(!pListenerArr)
        return;

    lang::EventObject aObj(pxParent);
    for(sal_uInt16 i = 0; i < pListenerArr->Count(); i++)
    {
        XEventListenerPtr pElem = pListenerArr->GetObject(i);
        (*pElem)->disposing(aObj);
    }
    pListenerArr->DeleteAndDestroy(0, pListenerArr->Count());
}


