/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UNOEVTLSTNR_HXX
#define _UNOEVTLSTNR_HXX

#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
namespace com{namespace sun{namespace star{
    namespace lang
        {
            class XEventListener;
        }
}}}
namespace binfilter {

//STRIP008 namespace com{namespace sun{namespace star{
//STRIP008 	namespace lang
//STRIP008 		{
//STRIP008 			class XEventListener;
//STRIP008 		}
//STRIP008 }}}
/* -----------------22.04.99 11:18-------------------
 *	Verwaltung der EventListener
 * --------------------------------------------------*/
typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > *  XEventListenerPtr;
SV_DECL_PTRARR(SwEvtLstnrArray, XEventListenerPtr, 4, 4)//STRIP008 ;
class SwEventListenerContainer
{
    protected:
        SwEvtLstnrArray*							pListenerArr;
        ::com::sun::star::uno::XInterface*			pxParent;
    public:
        SwEventListenerContainer( ::com::sun::star::uno::XInterface* pxParent);
        ~SwEventListenerContainer();

        void 	AddListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        BOOL	RemoveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & rxListener);
        void	Disposing();
};
} //namespace binfilter
#endif
