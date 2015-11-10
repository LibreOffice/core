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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RTFREADER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RTFREADER_HXX

#include <vector>
#include <svtools/parrtf.hxx>
#include "DExport.hxx"

class SvStream;

namespace dbaui
{
    class ORTFReader : public SvRTFParser , public ODatabaseExport
    {
        ::std::vector<sal_Int32>    m_vecColor;

    protected:
        virtual bool            CreateTable(int nToken) override;
        virtual void            NextToken( int nToken ) override; // base class
        virtual TypeSelectionPageFactory
                                getTypeSelectionPageFactory() override;

        virtual ~ORTFReader();

    public:
        ORTFReader( SvStream& rIn,
                    const SharedConnection& _rxConnection,
                    const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const TColumnVector* rList = nullptr,
                    const OTypeInfoMap* _pInfoMap = nullptr);
        // required for automatic type recognition
        ORTFReader( SvStream& rIn,
                    sal_Int32 nRows,
                    const TPositions &_rColumnPositions,
                    const css::uno::Reference< css::util::XNumberFormatter >& _rxNumberF,
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const TColumnVector* rList,
                    const OTypeInfoMap* _pInfoMap,
                    bool _bAutoIncrementEnabled);

        virtual SvParserState   CallParser() override;// base class
        /// @note Only recovers correct data if 2. CTOR has been used.
        ///       Otherwise, the SbaColumnList will be returned without changes
        virtual void            release() override;
    };

    typedef tools::SvRef<ORTFReader> ORTFReaderRef;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
