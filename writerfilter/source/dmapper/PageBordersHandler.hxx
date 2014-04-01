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
#ifndef INCLUDED_PAGEBORDERSHANDLER_HXX
#define INCLUDED_PAGEBORDERSHANDLER_HXX

#include "BorderHandler.hxx"
#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

#include <com/sun/star/table/BorderLine2.hpp>

#include <vector>


namespace writerfilter {
namespace dmapper {

class _PgBorder
{
public:
    com::sun::star::table::BorderLine2 m_rLine;
    sal_Int32   m_nDistance;
    BorderPosition m_ePos;
    bool m_bShadow;

    _PgBorder( );
    ~_PgBorder( );
};

class PageBordersHandler : public LoggedProperties
{
private:

    // See implementation of SectionPropertyMap::ApplyBorderToPageStyles
    sal_Int32 m_nDisplay;
    sal_Int32 m_nOffset;
    vector<_PgBorder> m_aBorders;

    // Properties
    virtual void lcl_attribute( Id eName, Value& rVal ) SAL_OVERRIDE;
    virtual void lcl_sprm( Sprm& rSprm ) SAL_OVERRIDE;

public:
    PageBordersHandler( );
    virtual ~PageBordersHandler( );

    inline sal_Int32 GetDisplayOffset( )
    {
        return ( m_nOffset << 5 ) + m_nDisplay;
    };
    void SetBorders( SectionPropertyMap* pSectContext );
};
typedef boost::shared_ptr< PageBordersHandler > PageBordersHandlerPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
