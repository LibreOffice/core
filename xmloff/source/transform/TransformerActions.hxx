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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX

#include <rtl/ustring.hxx>
#include <xmloff/namespacemap.hxx>
#include "TransformerActionInit.hxx"
#include "TransformerAction.hxx"
#include <unordered_map>

struct TransformerAction_Impl
{
    sal_uInt32 m_nActionType;
    sal_uInt32 m_nParam1;
    sal_uInt32 m_nParam2;
    sal_uInt32 m_nParam3;

    TransformerAction_Impl() :
        m_nActionType( XML_TACTION_EOT ),
        m_nParam1( 0 ),
        m_nParam2( 0 ),
        m_nParam3( 0 )
    {
    }

    sal_Int32 GetTokenFromParam1() const
    {
        return static_cast< sal_Int32 >( m_nParam1 );
    }

    sal_Int32 GetTokenFromParam2() const
    {
        return static_cast< sal_Int32 >( m_nParam2 );
    }

    sal_Int32 GetTokenFromParam3() const
    {
        return static_cast< sal_Int32 >( m_nParam3 );
    }

    // ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam1() const
    // {
    //     return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam1 & 0xffff );
    // }

    // ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam2() const
    // {
    //     return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam2 & 0xffff );
    // }

    // ::xmloff::token::XMLTokenEnum GetQNameTokenFromParam3() const
    // {
    //     return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam3 & 0xffff );
    // }

};

class XMLTransformerActions :
    public std::unordered_map< sal_Int32, TransformerAction_Impl >
{
public:
    explicit XMLTransformerActions( XMLTransformerActionInit const *pInit );
    ~XMLTransformerActions();

    void Add( XMLTransformerActionInit const *pInit );
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_TRANSFORMERACTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
