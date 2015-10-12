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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_DATASETTINGS_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_DATASETTINGS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/ustring.hxx>
#include <comphelper/propertystatecontainer.hxx>

namespace dbaccess
{

// ODataSettings_Base - a base class which implements the property member
//                 for an object implementing the sdb::DataSettings
//                 service
// the properties have to be registered when used
class ODataSettings_Base
{
public:
// <properties>
    OUString                       m_sFilter;
    OUString                       m_sHavingClause;
    OUString                       m_sGroupBy;
    OUString                       m_sOrder;
    bool                           m_bApplyFilter;     // no BitField ! the base class needs a pointer to this member !
    css::awt::FontDescriptor       m_aFont;
    css::uno::Any                  m_aRowHeight;
    css::uno::Any                  m_aTextColor;
    css::uno::Any                  m_aTextLineColor;
    sal_Int16                      m_nFontEmphasis;
    sal_Int16                      m_nFontRelief;
// </properties>

protected:
    ODataSettings_Base();
    ODataSettings_Base(const ODataSettings_Base& _rSource);
    ~ODataSettings_Base();
};
// ODataSettings - a base class which implements the property handling
//                 for an object implementing the sdb::DataSettings
//                 service

class ODataSettings : public ::comphelper::OPropertyStateContainer
                    , public ODataSettings_Base
{
    bool m_bQuery;
protected:
    ODataSettings(::cppu::OBroadcastHelper& _rBHelper,bool _bQuery = false);
    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _rDefault ) const override;

    /** register the properties from the param given. The parameter instance must be alive as long as tis object live.
        @param  _pItem
            The database settings, can be <br>this</br>
    */
    void registerPropertiesFor(ODataSettings_Base* _pItem);
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_DATASETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
