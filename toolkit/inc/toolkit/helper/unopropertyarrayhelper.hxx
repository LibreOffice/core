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

#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#define _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_

#include <toolkit/dllapi.h>
#include <cppuhelper/propshlp.hxx>

#include <list>
#include <set>
#include "toolkit/dllapi.h"

//  ----------------------------------------------------
//  class UnoPropertyArrayHelper
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC UnoPropertyArrayHelper : public ::cppu::IPropertyArrayHelper
{
private:
    std::set<sal_Int32>       maIDs;

protected:
    sal_Bool    ImplHasProperty( sal_uInt16 nPropId ) const;

public:
                UnoPropertyArrayHelper( const ::com::sun::star::uno::Sequence<sal_Int32>& rIDs );
                UnoPropertyArrayHelper( const std::list< sal_uInt16 > &rIDs );

    // ::cppu::IPropertyArrayHelper
    sal_Bool SAL_CALL fillPropertyMembersByHandle( ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle );
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties();
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& rPropertyName) throw (::com::sun::star::beans::UnknownPropertyException);
    sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName);
    sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName );
    sal_Int32 SAL_CALL fillHandles( sal_Int32* pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames );
};



#endif // _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
