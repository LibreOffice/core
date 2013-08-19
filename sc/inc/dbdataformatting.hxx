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
#ifndef SC_DBDATAFORMATTING_HXX
#define SC_DBDATAFORMATTING_HXX

#include "rtl/ustring.hxx"
#include "scdllapi.h"

class SC_DLLPUBLIC ScDBDataFormatting
{
        OUString maTableStyleName;
        OUString maFirstRowStripeStyle;
        OUString maSecondRowStripeStyle;
        OUString maFirstColStripeStyle;
        OUString maSecondColStripeStyle;
        sal_Int32 maFirstRowStripeSize;
        sal_Int32 maSecondRowStripeSize;
        sal_Int32 maFirstColStripeSize;
        sal_Int32 maSecondColStripeSize;
        bool bBandedRows;
        bool bBandedColumns;
    public:
        ScDBDataFormatting();
        ScDBDataFormatting(const OUString& rTableStyleName, const OUString& rFirstRowStripeStyle, const OUString& rSecondRowStripeStyle, const OUString& rFirstColStripeStyle, const OUString& rSecondColStripeStyle, bool bBRows, bool bBCols);
        ScDBDataFormatting( const ScDBDataFormatting& rTableFormatData );
        void SetTableStyleName( const OUString& rTableStyleName );
        const OUString& GetTableStyleName();
        void SetBandedRows( bool bBRows );
        bool GetBandedRows();
        void SetBandedColumns( bool bBCols );
        bool GetBandedColumns();
        const OUString& GetFirstRowStripeStyle() const;
        const OUString& GetSecondRowStripeStyle() const;
        const OUString& GetFirstColStripeStyle() const;
        const OUString& GetSecondColStripeStyle() const;
        sal_Int32 GetFirstRowStripeSize() const { return maFirstRowStripeSize; }
        sal_Int32 GetSecondRowStripeSize() const { return maSecondRowStripeSize; }
        sal_Int32 GetFirstColStripeSize() const { return maFirstColStripeSize; }
        sal_Int32 GetSecondColStripeSize() const { return maSecondColStripeSize; }
        void SetFirstRowStripeSize( const sal_Int32 nSize ){ maFirstRowStripeSize = nSize; }
        void SetSecondRowStripeSize( const sal_Int32 nSize ){ maSecondRowStripeSize = nSize; }
        void SetFirstColStripeSize( const sal_Int32 nSize ){ maFirstColStripeSize = nSize; }
        void SetSecondColStripeSize( const sal_Int32 nSize ){ maSecondColStripeSize = nSize; }
        void SetFirstRowStripeStyle( const OUString& aStyleName );
        void SetSecondRowStripeStyle( const OUString& aStyleName );
        void SetFirstColStripeStyle( const OUString& aStyleName );
        void SetSecondColStripeStyle( const OUString& aStyleName );
};

#endif
