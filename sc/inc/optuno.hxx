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

#ifndef INCLUDED_SC_INC_OPTUNO_HXX
#define INCLUDED_SC_INC_OPTUNO_HXX

#include "docuno.hxx"
#include "docoptio.hxx"

#define PROP_UNO_CALCASSHOWN    1
#define PROP_UNO_DEFTABSTOP     2
#define PROP_UNO_IGNORECASE     3
#define PROP_UNO_ITERENABLED    4
#define PROP_UNO_ITERCOUNT      5
#define PROP_UNO_ITEREPSILON    6
#define PROP_UNO_LOOKUPLABELS   7
#define PROP_UNO_MATCHWHOLE     8
#define PROP_UNO_NULLDATE       9
#define PROP_UNO_SPELLONLINE    10
#define PROP_UNO_STANDARDDEC    11
#define PROP_UNO_REGEXENABLED   12

class ScDocOptionsHelper
{
public:
    static const SfxItemPropertyMapEntry* GetPropertyMap();

    static bool setPropertyValue( ScDocOptions& rOptions,
                                    const SfxItemPropertyMap& rPropMap,
                                    const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue );
    static ::com::sun::star::uno::Any getPropertyValue(
                                    const ScDocOptions& rOptions,
                                    const SfxItemPropertyMap& rPropMap,
                                    const OUString& PropertyName );
};

//  empty doc object to supply only doc options

class ScDocOptionsObj : public ScModelObj
{
private:
    ScDocOptions    aOptions;

public:
                            ScDocOptionsObj( const ScDocOptions& rOpt );
    virtual                 ~ScDocOptionsObj();

    // get/setPropertyValue override to used stored options instead of document

    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
