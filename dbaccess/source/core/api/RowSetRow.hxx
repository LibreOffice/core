/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowSetRow.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "connectivity/FValue.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

namespace dbaccess
{
    typedef connectivity::ORowVector< connectivity::ORowSetValue >  ORowSetValueVector;
    typedef ::vos::ORef< ORowSetValueVector >                       ORowSetRow;
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
//      ORowSetOldRowHelper(const ORowSetOldRowHelper& _rRh)
//          : m_refCount(0)
//          , m_aRow(_rRh.m_aRow)
//      {}

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

    typedef ::vos::ORef< ORowSetOldRowHelper >  TORowSetOldRowHelperRef;

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

