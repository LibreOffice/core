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

#ifndef INCLUDED_TOOLKIT_INC_HELPER_UNOPROPERTYARRAYHELPER_HXX
#define INCLUDED_TOOLKIT_INC_HELPER_UNOPROPERTYARRAYHELPER_HXX

#include <toolkit/dllapi.h>
#include <cppuhelper/propshlp.hxx>

#include <list>
#include <set>


//  class UnoPropertyArrayHelper

class TOOLKIT_DLLPUBLIC UnoPropertyArrayHelper : public ::cppu::IPropertyArrayHelper
{
private:
    std::set<sal_Int32>       maIDs;

protected:
    bool    ImplHasProperty( sal_uInt16 nPropId ) const;

public:
                UnoPropertyArrayHelper( const ::com::sun::star::uno::Sequence<sal_Int32>& rIDs );
                UnoPropertyArrayHelper( const std::list< sal_uInt16 > &rIDs );

    // ::cppu::IPropertyArrayHelper
    sal_Bool SAL_CALL fillPropertyMembersByHandle( OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle ) override;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties() override;
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const OUString& rPropertyName) throw (::com::sun::star::beans::UnknownPropertyException) override;
    sal_Bool SAL_CALL hasPropertyByName(const OUString& rPropertyName) override;
    sal_Int32 SAL_CALL getHandleByName( const OUString & rPropertyName ) override;
    sal_Int32 SAL_CALL fillHandles( sal_Int32* pHandles, const ::com::sun::star::uno::Sequence< OUString > & rPropNames ) override;
};



#endif // INCLUDED_TOOLKIT_INC_HELPER_UNOPROPERTYARRAYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
