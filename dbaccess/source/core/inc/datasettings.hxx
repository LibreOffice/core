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

#ifndef _DBA_CORE_DATASETTINGS_HXX_
#define _DBA_CORE_DATASETTINGS_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/ustring.hxx>
#include <comphelper/propertystatecontainer.hxx>

namespace dbaccess
{

//==========================================================================
//= ODataSettings_Base - a base class which implements the property member
//=                 for an object implementing the sdb::DataSettings
//=                 service
//= the properties have to to be registered when used
//==========================================================================
class ODataSettings_Base
{
public:
// <properties>
    ::rtl::OUString                             m_sFilter;
    ::rtl::OUString                             m_sHavingClause;
    ::rtl::OUString                             m_sGroupBy;
    ::rtl::OUString                             m_sOrder;
    sal_Bool                                    m_bApplyFilter;     // no BitField ! the base class needs a pointer to this member !
    ::com::sun::star::awt::FontDescriptor       m_aFont;
    ::com::sun::star::uno::Any                  m_aRowHeight;
    ::com::sun::star::uno::Any                  m_aTextColor;
    ::com::sun::star::uno::Any                  m_aTextLineColor;
    sal_Int16                                   m_nFontEmphasis;
    sal_Int16                                   m_nFontRelief;
// </properties>

protected:
    ODataSettings_Base();
    ODataSettings_Base(const ODataSettings_Base& _rSource);
    ~ODataSettings_Base();
};
//==========================================================================
//= ODataSettings - a base class which implements the property handling
//=                 for an object implementing the sdb::DataSettings
//=                 service
//==========================================================================

class ODataSettings : public ::comphelper::OPropertyStateContainer
                    , public ODataSettings_Base
{
    sal_Bool m_bQuery;
protected:
    ODataSettings(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bQuery = sal_False);
    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const;

    /** register the properties from the param given. The parameter instance must be alive as long as tis object live.
        @param  _pItem
            The database settings, can be <br>this</br>
    */
    void registerPropertiesFor(ODataSettings_Base* _pItem);
};

}   // namespace dbaccess

#endif // _DBA_CORE_DATASETTINGS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
