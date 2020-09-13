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
#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include "TypeInfo.hxx"

class SvStream;

namespace dbaui
{
    class OFieldDescription;
    class OTableRow
    {
    private:
        OFieldDescription*      m_pActFieldDescr;
        sal_Int32               m_nPos;
        bool                    m_bReadOnly;
        bool                    m_bOwnsDescriptions;

    protected:
    public:
        OTableRow();
        OTableRow(const css::uno::Reference< css::beans::XPropertySet >& xAffectedCol);
        OTableRow( const OTableRow& rRow, long nPosition = -1 );
        ~OTableRow();

        OFieldDescription* GetActFieldDescr() const { return m_pActFieldDescr; }
        bool isValid() const { return GetActFieldDescr() != nullptr; }

        void SetFieldType( const TOTypeInfoSP& _pType, bool _bForce = false );

        void SetPrimaryKey( bool bSet );
        bool IsPrimaryKey() const;

        /** returns the current position in the table.
            @return
                the current position in the table
        */
        sal_Int32 GetPos() const { return m_nPos; }
        void SetPos(sal_Int32 _nPos) { m_nPos = _nPos; }

        /** set the row readonly
            @param  _bRead
                if <TRUE/> then the row is readonly, otherwise not
        */
        void SetReadOnly( bool _bRead=true ){ m_bReadOnly = _bRead; }

        /** returns if the row is readonly
            @return
                <TRUE/> if readonly, otherwise <FALSE/>
        */
        bool IsReadOnly() const { return m_bReadOnly; }

        friend SvStream& WriteOTableRow( SvStream& rStr,const OTableRow& _rRow );
        friend SvStream& ReadOTableRow( SvStream& rStr, OTableRow& _rRow );
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
