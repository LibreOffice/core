/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltcall.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:18:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FLTCALL_HXX
#define _FLTCALL_HXX
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#include <vcl/field.hxx>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <svtools/FilterConfigItem.hxx>
#endif

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

#endif
