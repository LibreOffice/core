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

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAPHELPER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAPHELPER_HXX

#if OSL_DEBUG_LEVEL > 1
#include "PropertyMap.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>

namespace writerfilter
{
namespace dmapper
{

void lcl_DumpTableColumnSeparators(const TagLogger::Pointer_t pLogger,
        const css::uno::Any & rTableColumnSeparators);
void lcl_DumpPropertyValues(const TagLogger::Pointer_t pLogger,
        css::beans::PropertyValues & rValues);

typedef css::uno::Sequence<css::beans::PropertyValues> PropertyValueSeq_t;
void lcl_DumpPropertyValueSeq(const TagLogger::Pointer_t pLogger,
     PropertyValueSeq_t & rPropValSeq);

typedef css::uno::Sequence<PropertyValueSeq_t> PropertyValueSeqSeq_t;
void lcl_DumpPropertyValueSeqSeq(const TagLogger::Pointer_t pLogger,
        PropertyValueSeqSeq_t & rPropValSeqSeq);
}
}
#endif // OSL_DEBUG_LEVEL > 1

#endif // INCLUDED_WRITERFILTER_SOURCE_DMAPPER_PROPERTYMAPHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
