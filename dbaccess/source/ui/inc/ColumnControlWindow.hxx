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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

namespace dbaui
{
    // OColumnControlWindow
    class OColumnControlWindow : public OFieldDescControl
    {
        ::com::sun::star::lang::Locale      m_aLocale;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        OTypeInfoMap                m_aDestTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aDestTypeInfoIndex;

        mutable TOTypeInfoSP        m_pTypeInfo; // default type
        OUString                    m_sTypeNames;       // these type names are the ones out of the resource file
        OUString                    m_sAutoIncrementValue;
        sal_Bool                    m_bAutoIncrementEnabled;
    protected:
        virtual void        ActivateAggregate( EControlType eType ) SAL_OVERRIDE;
        virtual void        DeactivateAggregate( EControlType eType ) SAL_OVERRIDE;

        virtual ::com::sun::star::lang::Locale  GetLocale() const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const SAL_OVERRIDE;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos) SAL_OVERRIDE;
        virtual sal_Bool            isAutoIncrementValueEnabled() const SAL_OVERRIDE;
        virtual OUString     getAutoIncrementValue() const SAL_OVERRIDE;
        virtual void                CellModified(long nRow, sal_uInt16 nColId ) SAL_OVERRIDE;

    public:
        OColumnControlWindow(Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);
        virtual ~OColumnControlWindow();

        void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xCon);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() SAL_OVERRIDE;
        virtual const OTypeInfoMap* getTypeInfo() const SAL_OVERRIDE;
        TOTypeInfoSP getDefaultTyp() const;
    };
}   // namespace dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_COLUMNCONTROLWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
