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
#ifndef DBAUI_RTFREADER_HXX
#define DBAUI_RTFREADER_HXX

#ifndef _VECTOR_
#include <vector>
#endif
#include <svtools/parrtf.hxx>
#include "DExport.hxx"
#include <tools/stream.hxx>

namespace dbaui
{
    class ORTFReader : public SvRTFParser , public ODatabaseExport
    {
        ::std::vector<sal_Int32>    m_vecColor;

    protected:
        virtual sal_Bool        CreateTable(int nToken);
        virtual void            NextToken( int nToken ); // base class
        virtual TypeSelectionPageFactory
                                getTypeSelectionPageFactory();

        ~ORTFReader();

    public:
        ORTFReader( SvStream& rIn,
                    const SharedConnection& _rxConnection,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList = 0,
                    const OTypeInfoMap* _pInfoMap = 0);
        // required for automatic type recognition
        ORTFReader( SvStream& rIn,
                    sal_Int32 nRows,
                    const TPositions &_rColumnPositions,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList,
                    const OTypeInfoMap* _pInfoMap,
                    sal_Bool _bAutoIncrementEnabled);

        virtual SvParserState   CallParser();// base class
        /// @note Only recovers correct data if 2. CTOR has been used.
        ///       Otherwise, the SbaColumnList will be returned without changes
        virtual void            release();
    };

    SV_DECL_IMPL_REF( ORTFReader );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
