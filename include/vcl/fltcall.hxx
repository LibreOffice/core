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

#ifndef INCLUDED_VCL_FLTCALL_HXX
#define INCLUDED_VCL_FLTCALL_HXX
#include <sal/types.h>
#include <vcl/field.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/FilterConfigItem.hxx>

class FilterConfigItem;
class SvStream;
class Graphic;
namespace vcl { class Window; }

struct FltCallDialogParameter
{

    VclPtr<vcl::Window> pWindow;
    ResMgr*     pResMgr;
    FieldUnit   eFieldUnit;
    OUString    aFilterExt;

    // In and Out PropertySequence for all filter dialogs
    css::uno::Sequence< css::beans::PropertyValue > aFilterData;

    FltCallDialogParameter( vcl::Window* pW, ResMgr* pRsMgr, FieldUnit eFiUni ) :
        pWindow         ( pW ),
        pResMgr         ( pRsMgr ),
        eFieldUnit      ( eFiUni ) {};
};

typedef bool (*PFilterCall)(SvStream & rStream, Graphic & rGraphic,
                            FilterConfigItem* pConfigItem);
    // Of this type are both export-filter and import-filter functions
    // rFileName is the complete path to the file to be imported or exported
    // pCallBack can be NULL. pCallerData is handed to the callback function
    // pOptionsConfig can be NULL; if not, the group of the config is already set
    // and may not be changed by this filter!

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
