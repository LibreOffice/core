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

class ScDBDataFormatting
{
    private:
        OUString maFirstRowStripeStyle;
        OUString maSecondRowStripeStyle;
        OUString maFirstColStripeStyle;
        OUString maSecondColStripeStyle;
        bool bBandedRows;
        bool bBandedColumns;
    public:
        ScDBDataFormatting(const OUString& rFirstRowStripeStyle, const OUString& rSecondRowStripeStyle, const OUString& rFirstColStripeStyle, const OUString& rSecondColStripeStyle);
        void SetBandedRows( bool bBRows );
        bool GetBandedRows();
        void SetBandedColumns( bool bBCols );
        bool GetBandedColumns();
        const OUString& GetFirstRowStripeStyle() const;
        const OUString& GetSecondRowStripeStyle() const;
        const OUString& GetFirstColStripeStyle() const;
        const OUString& GetSecondColStripeStyle() const;
        void SetFirstRowStripeStyle( const OUString& aStyleName );
        void SetSecondRowStripeStyle( const OUString& aStyleName );
        void SetFirstColStripeStyle( const OUString& aStyleName );
        void SetSecondColStripeStyle( const OUString& aStyleName );
};

#endif
