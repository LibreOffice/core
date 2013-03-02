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

#ifndef _FLTCALL_HXX
#define _FLTCALL_HXX
#include <sal/types.h>
#include <vcl/field.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/FilterConfigItem.hxx>

class FilterConfigItem;
class SvStream;
class Graphic;
class Window;

struct FltCallDialogParameter
{

    Window*     pWindow;
    ResMgr*     pResMgr;
    FieldUnit   eFieldUnit;
    String      aFilterExt;

    // In and Out PropertySequence for all filter dialogs
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;

    FltCallDialogParameter( Window* pW, ResMgr* pRsMgr, FieldUnit eFiUni ) :
        pWindow         ( pW ),
        pResMgr         ( pRsMgr ),
        eFieldUnit      ( eFiUni ) {};
};

typedef sal_Bool (*PFilterCall)(SvStream & rStream, Graphic & rGraphic,
                                FilterConfigItem* pConfigItem, sal_Bool bPrefDialog);
    // Von diesem Typ sind sowohl Export-Filter-Funktionen als auch Import-Filter-Funktionen.
    // rFileName ist der komplette Pfadname der zu importierenden bzw. zu exportierenden Datei.
    // pCallBack darf auch NULL sein. pCallerData wird der Callback-Funktion uebergeben.
    // pOptionsConfig darf NULL sein. Anderenfalls ist die Gruppe des Config schon gesetzt
    // und darf von dem Filter nicht geaendert werden!
    // Wenn bPrefDialog==sal_True gilt, wird ggf. ein Preferences-Dialog durchgefuehrt.

typedef sal_Bool ( *PFilterDlgCall )( FltCallDialogParameter& );
    // Von diesem Typ sind sowohl Export-Filter-Funktionen als auch Import-Filter-Funktionen.
    // Uebergeben wird ein Pointer auf ein Parent-Fenster und auf die Options-Config.
    // pOptions und pWindow duerfen NULL sein, in diesem Fall wird sal_False zurueckgeliefert.
    // Anderenfalls ist die Gruppe der Config schon gesetzt
    // und darf von dem Filter nicht geaendert werden!

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
