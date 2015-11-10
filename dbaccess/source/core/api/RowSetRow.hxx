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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETROW_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETROW_HXX

#include <rtl/ref.hxx>
#include <connectivity/CommonTools.hxx>
#include "connectivity/FValue.hxx"
#include <comphelper/types.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace dbaccess
{
    typedef connectivity::ORowVector< connectivity::ORowSetValue >  ORowSetValueVector;
    typedef ::rtl::Reference< ORowSetValueVector >                      ORowSetRow;
    typedef ::std::vector< ORowSetRow >                             ORowSetMatrix;

    class ORowSetOldRowHelper : public salhelper::SimpleReferenceObject
    {
        ORowSetRow                  m_aRow;

        ORowSetOldRowHelper& operator=(const ORowSetOldRowHelper& _rRH) = delete;
        ORowSetOldRowHelper(const ORowSetOldRowHelper& _rRh) = delete;
    public:
        ORowSetOldRowHelper(){}
        explicit ORowSetOldRowHelper(const ORowSetRow& _rRow)
            : m_aRow(_rRow)
        {}

        inline ORowSetRow getRow() const { return m_aRow; }
        inline void clearRow() { m_aRow = nullptr; }
        inline void setRow(const ORowSetRow& _rRow) { m_aRow = _rRow; }
    };

    typedef ::rtl::Reference< ORowSetOldRowHelper > TORowSetOldRowHelperRef;

    class ORowSetValueCompare
    {
        const css::uno::Any& m_rAny;
    public:
        explicit ORowSetValueCompare(const css::uno::Any& _rAny) : m_rAny(_rAny){}

        bool operator ()(const ORowSetRow& _rRH)
        {
            switch((_rRH->get())[0].getTypeKind())
            {
                case css::sdbc::DataType::TINYINT:
                case css::sdbc::DataType::SMALLINT:
                case css::sdbc::DataType::INTEGER:
                    return comphelper::getINT32(m_rAny) == (sal_Int32)(_rRH->get())[0];
                default:
                {
                    css::uno::Sequence<sal_Int8> aSeq;
                    m_rAny >>= aSeq;
                    return aSeq == (_rRH->get())[0];
                }
            }
        }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETROW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
