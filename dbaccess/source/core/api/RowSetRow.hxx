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
            osl_incrementInterlockedCount( &m_refCount );
        }
        void release()
        {
            if (! osl_decrementInterlockedCount( &m_refCount ))
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
