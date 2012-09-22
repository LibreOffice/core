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
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#define DBACCESS_CORE_API_ROWSETROW_HXX

#include <rtl/ref.hxx>
#include <connectivity/CommonTools.hxx>
#include "connectivity/FValue.hxx"
#include <comphelper/types.hxx>

namespace dbaccess
{
    typedef connectivity::ORowVector< connectivity::ORowSetValue >  ORowSetValueVector;
    typedef ::rtl::Reference< ORowSetValueVector >                      ORowSetRow;
    typedef ::std::vector< ORowSetRow >                             ORowSetMatrix;

    class ORowSetOldRowHelper
    {
        oslInterlockedCount         m_refCount;
        ORowSetRow                  m_aRow;

        ORowSetOldRowHelper& operator=(const ORowSetOldRowHelper& _rRH);
        ORowSetOldRowHelper(const ORowSetOldRowHelper& _rRh);
    public:
        ORowSetOldRowHelper() : m_refCount(0){}
        ORowSetOldRowHelper(const ORowSetRow& _rRow)
            : m_refCount(0)
            , m_aRow(_rRow)
        {}

        void acquire()
        {
            osl_atomic_increment( &m_refCount );
        }
        void release()
        {
            if (! osl_atomic_decrement( &m_refCount ))
                delete this;
        }
        inline ORowSetRow getRow() const { return m_aRow; }
        inline void clearRow() { m_aRow = NULL; }
        inline void setRow(const ORowSetRow& _rRow) { m_aRow = _rRow; }
    };

    typedef ::rtl::Reference< ORowSetOldRowHelper > TORowSetOldRowHelperRef;

    class ORowSetValueCompare
    {
        const ::com::sun::star::uno::Any& m_rAny;
    public:
        ORowSetValueCompare(const ::com::sun::star::uno::Any& _rAny) : m_rAny(_rAny){}

        sal_Bool operator ()(const ORowSetRow& _rRH)
        {
            switch((_rRH->get())[0].getTypeKind())
            {
                case ::com::sun::star::sdbc::DataType::TINYINT:
                case ::com::sun::star::sdbc::DataType::SMALLINT:
                case ::com::sun::star::sdbc::DataType::INTEGER:
                    return comphelper::getINT32(m_rAny) == (sal_Int32)(_rRH->get())[0];
                default:
                {
                    ::com::sun::star::uno::Sequence<sal_Int8> aSeq;
                    m_rAny >>= aSeq;
                    return aSeq == (_rRH->get())[0];
                }
            }
        }
    };
}
#endif // DBACCESS_CORE_API_ROWSETROW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
