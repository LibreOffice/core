/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FLTCALL_HXX
#define _FLTCALL_HXX
#include <tools/gen.hxx>
#include <vcl/field.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <bf_svtools/FilterConfigItem.hxx>

class FilterConfigItem;
class SvStream;
class Graphic;
class Window;

namespace binfilter
{

struct FltCallDialogParameter
{

    Window*		pWindow;
    ResMgr*		pResMgr;
    FieldUnit	eFieldUnit;
    String		aFilterExt;

    // In and Out PropertySequence for all filter dialogs
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;

    FltCallDialogParameter( Window* pW, ResMgr* pRsMgr, FieldUnit eFiUni ) :
        pWindow			( pW ),
        pResMgr			( pRsMgr ),
        eFieldUnit		( eFiUni ) {};
};

typedef BOOL (*PFilterCall)(SvStream & rStream, Graphic & rGraphic,
                                FilterConfigItem* pConfigItem, sal_Bool bPrefDialog);
    // Von diesem Typ sind sowohl Export-Filter-Funktionen als auch Import-Filter-Funktionen.
    // rFileName ist der komplette Pfadname der zu importierenden bzw. zu exportierenden Datei.
    // pCallBack darf auch NULL sein. pCallerData wird der Callback-Funktion uebergeben.
    // pOptionsConfig darf NULL sein. Anderenfalls ist die Gruppe des Config schon gesetzt
    // und darf von dem Filter nicht geaendert werden!
    // Wenn bPrefDialog==TRUE gilt, wird ggf. ein Preferences-Dialog durchgefuehrt.

typedef BOOL ( *PFilterDlgCall )( FltCallDialogParameter& );
    // Von diesem Typ sind sowohl Export-Filter-Funktionen als auch Import-Filter-Funktionen.
    // Uebergeben wird ein Pointer auf ein Parent-Fenster und auf die Options-Config.
    // pOptions und pWindow duerfen NULL sein, in diesem Fall wird FALSE zurueckgeliefert.
    // Anderenfalls ist die Gruppe der Config schon gesetzt
    // und darf von dem Filter nicht geaendert werden!

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
