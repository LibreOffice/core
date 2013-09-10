/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _LINGUISTIC_HHConvDic_HXX_
#define _LINGUISTIC_HHConvDic_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>

#include "linguistic/misc.hxx"
#include "defs.hxx"
#include "convdic.hxx"


class HHConvDic :
    public ConvDic
{
    // disallow copy-constructor and assignment-operator for now
    HHConvDic(const HHConvDic &);
    HHConvDic & operator = (const HHConvDic &);

public:
    HHConvDic( const OUString &rName, const OUString &rMainURL );
    virtual ~HHConvDic();

    // XConversionDictionary
    virtual void SAL_CALL addEntry( const OUString& aLeftText, const OUString& aRightText ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< OUString >
        getSupportedServiceNames_Static() throw();
};

inline OUString HHConvDic::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu2.HHConvDic" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
