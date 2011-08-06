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
#ifndef DBAUI_TABLEROW_HXX
#define DBAUI_TABLEROW_HXX

#include <comphelper/stl_types.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "TypeInfo.hxx"


namespace dbaui
{
    class OFieldDescription;
    class OTypeInfo;
    class OTableRow
    {
    private:
        OFieldDescription*      m_pActFieldDescr;
        long                    m_nPos;
        bool                    m_bReadOnly;
        bool                    m_bOwnsDescriptions;

    protected:
    public:
        OTableRow();
        OTableRow(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAffectedCol);
        OTableRow( const OTableRow& rRow, long nPosition = -1 );
        ~OTableRow();

        inline OFieldDescription* GetActFieldDescr() const { return m_pActFieldDescr; }
        inline bool isValid() const { return GetActFieldDescr() != NULL; }

        void SetFieldType( const TOTypeInfoSP& _pType, sal_Bool _bForce = sal_False );

        void SetPrimaryKey( sal_Bool bSet );
        sal_Bool IsPrimaryKey() const;

        /** returns the current position in the table.
            @return
                the current position in the table
        */
        inline long GetPos() const { return m_nPos; }
        inline void SetPos(sal_Int32 _nPos) { m_nPos = _nPos; }

        /** set the row readonly
            @param  _bRead
                if <TRUE/> then the row is redonly, otherwise not
        */
        inline void SetReadOnly( bool _bRead=true ){ m_bReadOnly = _bRead; }

        /** returns if the row is readonly
            @return
                <TRUE/> if readonly, otherwise <FALSE/>
        */
        inline bool IsReadOnly() const { return m_bReadOnly; }

        friend SvStream& operator<<( SvStream& rStr,const OTableRow& _rRow );
        friend SvStream& operator>>( SvStream& rStr, OTableRow& _rRow );
    };
}
#endif // DBAUI_TABLEROW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
