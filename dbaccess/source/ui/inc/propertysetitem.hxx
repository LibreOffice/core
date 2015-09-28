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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSETITEM_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSETITEM_HXX

#include <svl/poolitem.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace dbaui
{

    // OPropertySetItem
    /** <type>SfxPoolItem</type> which transports a XPropertySet
    */
    class OPropertySetItem : public SfxPoolItem
    {
        css::uno::Reference< css::beans::XPropertySet >   m_xSet;

    public:
        TYPEINFO_OVERRIDE();
        OPropertySetItem(sal_Int16 nWhich);
        OPropertySetItem(sal_Int16 nWhich,
            const css::uno::Reference< css::beans::XPropertySet >& _rxSet);
        OPropertySetItem(const OPropertySetItem& _rSource);

        virtual bool             operator==(const SfxPoolItem& _rItem) const SAL_OVERRIDE;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const SAL_OVERRIDE;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_PROPERTYSETITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
