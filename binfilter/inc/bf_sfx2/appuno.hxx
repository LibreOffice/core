/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFX_APPUNO_HXX
#define _SFX_APPUNO_HXX

//____________________________________________________________________________________________________________________________________
//	generated header
//____________________________________________________________________________________________________________________________________

#include <com/sun/star/frame/XFrame.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/lang/XTypeProvider.hpp>

#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/util/URL.hpp>

#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <com/sun/star/frame/XDispatch.hpp>

#include <com/sun/star/frame/XSynchronousDispatch.hpp>

#include <com/sun/star/frame/XNotifyingDispatch.hpp>

#include <com/sun/star/frame/XDispatchResultListener.hpp>

#include <com/sun/star/frame/DispatchDescriptor.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//____________________________________________________________________________________________________________________________________
//	fix uno header
//____________________________________________________________________________________________________________________________________

#include <com/sun/star/uno/Any.h>

#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <cppuhelper/weak.hxx>

#include <cppuhelper/implbase3.hxx>

//____________________________________________________________________________________________________________________________________
//	something else header
//____________________________________________________________________________________________________________________________________

#include <tools/errcode.hxx>
#include <bf_svtools/svarray.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#include <bf_sfx2/sfxuno.hxx>

#include <bf_svtools/poolitem.hxx>

namespace binfilter {
class SfxAllItemSet;
class SfxItemSet;

class SfxUsrAnyItem : public SfxPoolItem
{
    ::com::sun::star::uno::Any  aValue;
public:
                                TYPEINFO();
                                SfxUsrAnyItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Any& rAny );
    ::com::sun::star::uno::Any  GetValue() const
                                { return aValue; }
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual String              GetValueText() const{DBG_BF_ASSERT(0, "STRIP"); return String();} //STRIP001 virtual String              GetValueText() const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
};

typedef SfxUsrAnyItem SfxUnoAnyItem;

void TransformParameters(           sal_uInt16                          nSlotId     ,
                            const	UNOSEQUENCE< UNOPROPERTYVALUE >&	seqArgs		,
                                    SfxAllItemSet&						aSet	);

void TransformItems(        sal_uInt16                          nSlotId     ,
                            const	SfxItemSet&							aSet		,
                                    UNOSEQUENCE< UNOPROPERTYVALUE >&	seqArgs	 );


}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
