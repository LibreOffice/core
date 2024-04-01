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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <label.hxx>
#include <labimg.hxx>
#include <unoprnms.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;

void SwLabDlg::UpdateFieldInformation(uno::Reference< frame::XModel > const & xModel, const SwLabItem& rItem)
{
    uno::Reference< text::XTextFieldsSupplier >  xFields(xModel, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess >  xFieldMasters = xFields->getTextFieldMasters();

    static const struct SwLabItemMap {
        const char* pName;
        OUString SwLabItem::*pValue;
    }  aArr[] = {
        { "BC_PRIV_FIRSTNAME"  , &SwLabItem::m_aPrivFirstName },
        { "BC_PRIV_NAME"       , &SwLabItem::m_aPrivName },
        { "BC_PRIV_INITIALS"   , &SwLabItem::m_aPrivShortCut },
        { "BC_PRIV_FIRSTNAME_2", &SwLabItem::m_aPrivFirstName2 },
        { "BC_PRIV_NAME_2"     , &SwLabItem::m_aPrivName2 },
        { "BC_PRIV_INITIALS_2" , &SwLabItem::m_aPrivShortCut2 },
        { "BC_PRIV_STREET"     , &SwLabItem::m_aPrivStreet },
        { "BC_PRIV_ZIP"        , &SwLabItem::m_aPrivZip },
        { "BC_PRIV_CITY"       , &SwLabItem::m_aPrivCity },
        { "BC_PRIV_COUNTRY"    , &SwLabItem::m_aPrivCountry },
        { "BC_PRIV_STATE"      , &SwLabItem::m_aPrivState },
        { "BC_PRIV_TITLE"      , &SwLabItem::m_aPrivTitle },
        { "BC_PRIV_PROFESSION" , &SwLabItem::m_aPrivProfession },
        { "BC_PRIV_PHONE"      , &SwLabItem::m_aPrivPhone },
        { "BC_PRIV_MOBILE"     , &SwLabItem::m_aPrivMobile },
        { "BC_PRIV_FAX"        , &SwLabItem::m_aPrivFax },
        { "BC_PRIV_WWW"        , &SwLabItem::m_aPrivWWW },
        { "BC_PRIV_MAIL"       , &SwLabItem::m_aPrivMail },
        { "BC_COMP_COMPANY"    , &SwLabItem::m_aCompCompany },
        { "BC_COMP_COMPANYEXT" , &SwLabItem::m_aCompCompanyExt },
        { "BC_COMP_SLOGAN"     , &SwLabItem::m_aCompSlogan },
        { "BC_COMP_STREET"     , &SwLabItem::m_aCompStreet },
        { "BC_COMP_ZIP"        , &SwLabItem::m_aCompZip },
        { "BC_COMP_CITY"       , &SwLabItem::m_aCompCity },
        { "BC_COMP_COUNTRY"    , &SwLabItem::m_aCompCountry },
        { "BC_COMP_STATE"      , &SwLabItem::m_aCompState },
        { "BC_COMP_POSITION"   , &SwLabItem::m_aCompPosition },
        { "BC_COMP_PHONE"      , &SwLabItem::m_aCompPhone },
        { "BC_COMP_MOBILE"     , &SwLabItem::m_aCompMobile },
        { "BC_COMP_FAX"        , &SwLabItem::m_aCompFax },
        { "BC_COMP_WWW"        , &SwLabItem::m_aCompWWW },
        { "BC_COMP_MAIL"       , &SwLabItem::m_aCompMail },
        { nullptr, nullptr }
    };

    try
    {
        for( const SwLabItemMap* p = aArr; p->pName; ++p )
        {
            OUString uFieldName(
                "com.sun.star.text.FieldMaster.User."
                + OUString::createFromAscii(p->pName));
            if( xFieldMasters->hasByName( uFieldName ))
            {
                uno::Any aFirstName = xFieldMasters->getByName( uFieldName );
                uno::Reference< beans::XPropertySet >  xField;
                aFirstName >>= xField;
                uno::Any aContent;
                aContent <<= rItem.*p->pValue;
                xField->setPropertyValue( UNO_NAME_CONTENT, aContent );
            }
        }
    }
    catch (const uno::RuntimeException&)
    {

    }

    uno::Reference< container::XEnumerationAccess >  xFieldAcc = xFields->getTextFields();
    uno::Reference< util::XRefreshable >  xRefresh(xFieldAcc, uno::UNO_QUERY);
    xRefresh->refresh();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
