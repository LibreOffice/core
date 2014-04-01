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

#ifndef INCLUDED_GRAPHICHELPERS_HXX
#define INCLUDED_GRAPHICHELPERS_HXX

#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>

#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class PositionHandler: public LoggedProperties
{
public:
    PositionHandler( bool vertical );
    virtual ~PositionHandler( );
    static void setPositionOffset(const OUString & sText, bool vertical);
    static int getPositionOffset(bool vertical);
    static void setAlignH(const OUString & sText);
    static void setAlignV(const OUString & sText);
    sal_Int16 orientation() const;
    sal_Int16 relation() const;
    sal_Int32 position() const;
 private:
    virtual void lcl_attribute( Id aName, Value& rVal ) SAL_OVERRIDE;
    virtual void lcl_sprm( Sprm& rSprm ) SAL_OVERRIDE;
    sal_Int16 m_nOrient;
    sal_Int16 m_nRelation;
    sal_Int32 m_nPosition;
    static int savedPositionOffsetV, savedPositionOffsetH;
    static int savedAlignV, savedAlignH;
};
typedef boost::shared_ptr<PositionHandler> PositionHandlerPtr;

class WrapHandler: public LoggedProperties
{
public:
    WrapHandler( );
    virtual ~WrapHandler( );

    sal_Int32 m_nType;
    sal_Int32 m_nSide;

    sal_Int32 getWrapMode( );

 private:
    virtual void lcl_attribute( Id aName, Value& rVal ) SAL_OVERRIDE;
    virtual void lcl_sprm( Sprm& rSprm ) SAL_OVERRIDE;
};
typedef boost::shared_ptr<WrapHandler> WrapHandlerPtr;

class GraphicZOrderHelper
{
public:
    void addItem( uno::Reference< beans::XPropertySet > props, sal_Int32 relativeHeight );
    sal_Int32 findZOrder( sal_Int32 relativeHeight, bool bOldStyle = false );
private:
    typedef std::map< sal_Int32, uno::Reference< beans::XPropertySet > > Items;
    Items items;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
