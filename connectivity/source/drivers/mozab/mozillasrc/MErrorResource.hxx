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

#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#define CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX

#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozab
    {
        class ErrorDescriptor
        {
        private:
            sal_uInt16      m_nErrorResourceId;
            sal_Int32       m_nErrorCondition;
            ::rtl::OUString m_sParameter;

        public:
            ErrorDescriptor()
                :m_nErrorResourceId(0)
                ,m_nErrorCondition(0)
                ,m_sParameter()
            {
            }

            inline void set( const sal_uInt16 _nErrorResourceId, const sal_Int32 _nErrorCondition, const ::rtl::OUString& _rParam )
            {
                m_nErrorResourceId = _nErrorResourceId;
                m_nErrorCondition = _nErrorCondition;
                m_sParameter = _rParam;
            }
            inline void setResId( const sal_uInt16 _nErrorResourceId )
            {
                m_nErrorResourceId = _nErrorResourceId;
            }
            inline void reset()
            {
                m_nErrorResourceId = 0;
                m_nErrorCondition = 0;
            }

            inline sal_uInt16 getResId() const                  { return m_nErrorResourceId; }
            inline sal_Int32  getErrorCondition() const         { return m_nErrorCondition; }
            inline const ::rtl::OUString& getParameter() const  { return m_sParameter; }

            inline bool is() const { return ( m_nErrorResourceId != 0 ) || ( m_nErrorCondition != 0 ); }
        };
    }
}

#endif // CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
