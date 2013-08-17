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

#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#define _DBAUI_PROPERTYSETITEM_HXX_

#include <svl/poolitem.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace dbaui
{

    // OPropertySetItem
    /** <type>SfxPoolItem</type> which transports a XPropertySet
    */
    class OPropertySetItem : public SfxPoolItem
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xSet;

    public:
        TYPEINFO();
        OPropertySetItem(sal_Int16 nWhich);
        OPropertySetItem(sal_Int16 nWhich,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSet);
        OPropertySetItem(const OPropertySetItem& _rSource);

        virtual int              operator==(const SfxPoolItem& _rItem) const;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                getPropertySet() const { return m_xSet; }
    };

}   // namespace dbaui

#endif // _DBAUI_PROPERTYSETITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
