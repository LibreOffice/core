/*************************************************************************
 *
 *  $RCSfile: unofield.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 14:46:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#include <cmdid.h>
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif

#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX //autogen
#include <svtools/itemprop.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIMERANGE_HPP_
#include <com/sun/star/util/DateTimeRange.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PAGENUMBERTYPE_HPP_
#include <com/sun/star/text/PageNumberType.hpp>
#endif
#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef  _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef  _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef  _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef  _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef  _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::rtl;


/******************************************************************************
 *
 ******************************************************************************/
//Abbildung der Properties auf den Descriptor
#define FIELD_PROP_PAR1             10
#define FIELD_PROP_PAR2             11
#define FIELD_PROP_PAR3             12
#define FIELD_PROP_FORMAT           13
#define FIELD_PROP_SUBTYPE          14
#define FIELD_PROP_BOOL1            15
#define FIELD_PROP_BOOL2            16
#define FIELD_PROP_DATE             17
#define FIELD_PROP_USHORT1          18
#define FIELD_PROP_USHORT2          19
#define FIELD_PROP_BYTE1            20
#define FIELD_PROP_DOUBLE           21
#define FIELD_PROP_BOOL3            22
#define FIELD_PROP_PAR4             23
#define FIELD_PROP_SHORT1           24
#define FIELD_PROP_DATE_TIME        25

//static SfxItemPropertyMap aSetRefFieldPropMap     [] = {{0,0,0,0}};
//static SfxItemPropertyMap aInetFieldPropMap       [] = {{0,0,0,0}};


class SwFieldPropMapProvider
{
public:
    static const SfxItemPropertyMap* GetPropertyMap(USHORT nServiceId);
};
/* -----------------------------27.03.00 09:59--------------------------------

 ---------------------------------------------------------------------------*/
const SfxItemPropertyMap* SwFieldPropMapProvider::GetPropertyMap(USHORT nServiceId)
{
    const SfxItemPropertyMap* pRet = 0;
    switch(nServiceId)
    {
        case  SW_SERVICE_FIELDTYPE_DATETIME:
        {
            static SfxItemPropertyMap aDateTimeFieldPropMap[] =
            {
                {SW_PROP_NAME(UNO_NAME_ADJUST), FIELD_PROP_SUBTYPE,     &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATE_TIME_VALUE), FIELD_PROP_DATE_TIME,  &::getCppuType((util::DateTime*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {SW_PROP_NAME(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aDateTimeFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_USER     :
        {
            static SfxItemPropertyMap aUserFieldPropMap[] =
            {
                {SW_PROP_NAME(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_VISIBLE),     FIELD_PROP_BOOL1,   &::getBooleanCppuType(), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };

            pRet = aUserFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_SET_EXP  :
        {
            static SfxItemPropertyMap aSetExpFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_HINT),               FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT,  &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_INPUT),       FIELD_PROP_BOOL1,   &::getBooleanCppuType(),    PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL3,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   &::getBooleanCppuType(),    PROPERTY_NONE, 0},
                //TODO: UNO_NAME_VARIABLE_NAME gibt es das wirklich?
                {SW_PROP_NAME(UNO_NAME_SEQUENCE_VALUE), FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  &::getCppuType((const Double*)0),   PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_VARIABLE_NAME),  FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0),   PropertyAttribute::READONLY, 0},
                {0,0,0,0}
            };
            pRet = aSetExpFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_GET_EXP  :
        {
            static SfxItemPropertyMap aGetExpFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  &::getCppuType((const Double*)0), PropertyAttribute::READONLY,  0},
                {SW_PROP_NAME(UNO_NAME_VARIABLE_SUBTYPE),   FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aGetExpFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_FILE_NAME:
        {
            static SfxItemPropertyMap aFileNameFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, &::getBooleanCppuType(),      PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aFileNameFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_PAGE_NUM :
        {
            static SfxItemPropertyMap aPageNumFieldPropMap      [] =
            {
                {SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_FORMAT,  &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_OFFSET),             FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),  PROPERTY_NONE,    0},
                {SW_PROP_NAME(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, &::getCppuType((const PageNumberType*)0), PROPERTY_NONE,    0},
                {SW_PROP_NAME(UNO_NAME_USERTEXT),           FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aPageNumFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_AUTHOR   :
        {
            static SfxItemPropertyMap aAuthorFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),    PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, &::getBooleanCppuType(),      PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_FULL_NAME),FIELD_PROP_BOOL1, &::getBooleanCppuType(),        PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aAuthorFieldPropMap;
        }
        break;
        case  SW_SERVICE_FIELDTYPE_CHAPTER  :
        {
            static SfxItemPropertyMap aChapterFieldPropMap      [] =
            {
                {SW_PROP_NAME(UNO_NAME_CHAPTER_FORMAT),FIELD_PROP_USHORT1,  &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_LEVEL        ),FIELD_PROP_BYTE1,         &::getCppuType((const sal_Int8*)0),     PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aChapterFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_GET_REFERENCE         :
        {
            static SfxItemPropertyMap aGetRefFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_REFERENCE_FIELD_PART),FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),   PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_REFERENCE_FIELD_SOURCE),FIELD_PROP_USHORT2, &::getCppuType((const sal_Int16*)0),     PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_SEQUENCE_NUMBER),    FIELD_PROP_SHORT1,  &::getCppuType((const sal_Int16*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_SOURCE_NAME),        FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aGetRefFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT      :
        {
            static SfxItemPropertyMap aConditionedTxtFieldPropMap   [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_FALSE_CONTENT),  FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_TRUE_CONTENT) ,  FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aConditionedTxtFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT :
        {
            static SfxItemPropertyMap aHiddenTxtFieldPropMap    [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CONTENT) ,       FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aHiddenTxtFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_ANNOTATION            :
        {
            static SfxItemPropertyMap aAnnotationFieldPropMap   [] =
            {
                {SW_PROP_NAME(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CONTENT),    FIELD_PROP_PAR2,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATE),   FIELD_PROP_DATE,    &::getCppuType((const util::Date*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aAnnotationFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_INPUT_USER:
        case SW_SERVICE_FIELDTYPE_INPUT                 :
        {
            static SfxItemPropertyMap aInputFieldPropMap        [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_HINT),       FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aInputFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_MACRO                 :
        {
            static SfxItemPropertyMap aMacroFieldPropMap        [] =
            {
                {SW_PROP_NAME(UNO_NAME_HINT), FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_MACRO),FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aMacroFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DDE                   :
        {
            static SfxItemPropertyMap aDDEFieldPropMap          [] = {{0,0,0,0}};
            pRet = aDDEFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_HIDDEN_PARA           :
        {
            static SfxItemPropertyMap aHiddenParaFieldPropMap   [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONDITION),FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aHiddenParaFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOC_INFO              :
        {
            static SfxItemPropertyMap aDocInfoFieldPropMap      [] =
            {
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   &::getBooleanCppuType(),        PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_INFO_FORMAT),    FIELD_PROP_USHORT2, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_INFO_TYPE),  FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aDocInfoFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME         :
        {
            static SfxItemPropertyMap aTmplNameFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aTmplNameFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_USER_EXT              :
        {
            static SfxItemPropertyMap aUsrExtFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),           FIELD_PROP_BOOL1,   &::getBooleanCppuType(),        PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_USER_DATA_TYPE), FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aUsrExtFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_SET          :
        {
            static SfxItemPropertyMap aRefPgSetFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_OFFSET),     FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_ON),     FIELD_PROP_BOOL1,   &::getBooleanCppuType(),        PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aRefPgSetFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_GET          :
        {
            static SfxItemPropertyMap aRefPgGetFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aRefPgGetFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_JUMP_EDIT             :
        {
            static SfxItemPropertyMap aJumpEdtFieldPropMap      [] =
            {
                {SW_PROP_NAME(UNO_NAME_HINT),               FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_PLACEHOLDER),        FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_PLACEHOLDER_TYPE), FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),  PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aJumpEdtFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_SCRIPT                :
        {
            static SfxItemPropertyMap aScriptFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),        FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_SCRIPT_TYPE),    FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_URL_CONTENT),    FIELD_PROP_BOOL1, &::getBooleanCppuType(),      PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aScriptFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET     :
        {
            static SfxItemPropertyMap aDBNextSetFieldPropMap    [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONDITION)   ,     FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aDBNextSetFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET      :
        {
            static SfxItemPropertyMap aDBNumSetFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONDITION),        FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME   ), FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME  ), FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, &::getCppuType((const sal_Int32*)0), PROPERTY_NONE,   0},
                {0,0,0,0}
            };
            pRet = aDBNumSetFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM      :
        {
            static SfxItemPropertyMap aDBSetNumFieldPropMap [] =
            {
                {SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),       FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,   0},
                {SW_PROP_NAME(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, &::getCppuType((const sal_Int32*)0), PROPERTY_NONE,   0},
                {0,0,0,0}
            };
            pRet = aDBSetNumFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE              :
        {
            static SfxItemPropertyMap aDBFieldPropMap           [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_DATA_BASE_FORMAT),FIELD_PROP_BOOL1, &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT, &::getCppuType((const sal_Int32*)0), PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aDBFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NAME         :
        {
            static SfxItemPropertyMap aDBNameFieldPropMap       [] =
            {
                {SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aDBNameFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_PAGE_COUNT            :
        case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       :
        case SW_SERVICE_FIELDTYPE_WORD_COUNT            :
        case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       :
        case SW_SERVICE_FIELDTYPE_TABLE_COUNT           :
        case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  :
        case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT :
        {
            static SfxItemPropertyMap aDocstatFieldPropMap      [] =
            {
                {SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
            //  {UNO_NAME_STATISTIC_TYPE_ID,FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aDocstatFieldPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR :
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR      :
        {
            static SfxItemPropertyMap aDocInfoAuthorPropMap         [] =
            {
                {SW_PROP_NAME(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {0,0,0,0}
            };
            pRet = aDocInfoAuthorPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME   :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME:
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME:
        {
            static SfxItemPropertyMap aDocInfoDateTimePropMap           [] =
            {
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  &::getCppuType((const Double*)0), PropertyAttribute::READONLY,  0},
                {SW_PROP_NAME(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,   0},
                {0,0,0,0}
            };
            pRet = aDocInfoDateTimePropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME     :
        {
            static SfxItemPropertyMap aDocInfoEditTimePropMap           [] =
            {
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  &::getCppuType((const Double*)0), PropertyAttribute::READONLY,  0},
                {SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,   0},
                {0,0,0,0}
            };
            pRet = aDocInfoEditTimePropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION   :
        case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0:
        case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1:
        case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2:
        case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3:
        case SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS         :
        case SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT           :
        case SW_SERVICE_FIELDTYPE_DOCINFO_TITLE             :
        {
            static SfxItemPropertyMap aDocInfoStringContentPropMap          [] =
            {
                {SW_PROP_NAME(UNO_NAME_CONTENT),    FIELD_PROP_PAR1,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {0,0,0,0}
            };
            pRet = aDocInfoStringContentPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_REVISION          :
        {
            static SfxItemPropertyMap aDocInfoRevisionPropMap   [] =
            {
                {SW_PROP_NAME(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, &::getCppuType((const OUString*)0),  PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_REVISION),   FIELD_PROP_USHORT1, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   &::getBooleanCppuType()  , PROPERTY_NONE,0},
                {0,0,0,0}
            };
            pRet = aDocInfoRevisionPropMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_DUMMY_0 :
        case SW_SERVICE_FIELDTYPE_DUMMY_2:
        case SW_SERVICE_FIELDTYPE_DUMMY_3:
        case SW_SERVICE_FIELDTYPE_DUMMY_4:
        case SW_SERVICE_FIELDTYPE_DUMMY_5:
        case SW_SERVICE_FIELDTYPE_DUMMY_6:
        case SW_SERVICE_FIELDTYPE_DUMMY_7:
        case SW_SERVICE_FIELDTYPE_DUMMY_8:
        case SW_SERVICE_FIELDTYPE_TABLEFIELD            :
        {
            static SfxItemPropertyMap aEmptyPropMap         [] =
            {
                {0,0,0,0}
            };
            pRet = aEmptyPropMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_USER :
        {
            static SfxItemPropertyMap aUserFieldTypePropMap[] =
            {
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  new uno::Type(::getCppuType((Sequence<Reference<XDependentTextField> >*)0)), PropertyAttribute::READONLY, 0},
#else
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  &::getCppuType((Sequence<Reference<XDependentTextField> >*)0), PropertyAttribute::READONLY, 0},
#endif
                {SW_PROP_NAME(UNO_NAME_IS_EXPRESSION),      0,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_NAME),               0,  &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_VALUE),          0,  &::getCppuType((const Double*)0), PROPERTY_NONE,    0},
                {SW_PROP_NAME(UNO_NAME_CONTENT),            0,  &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aUserFieldTypePropMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_DDE         :
        {
            static SfxItemPropertyMap aDDEFieldTypePropMap[] =
            {
                {SW_PROP_NAME(UNO_NAME_DDE_COMMAND_ELEMENT ), 0,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_DDE_COMMAND_FILE    ), 0,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_DDE_COMMAND_TYPE    ), 0,    &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC_UPDATE), 0,  &::getBooleanCppuType(), PROPERTY_NONE,    0},
                {SW_PROP_NAME(UNO_NAME_NAME),               0,  &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aDDEFieldTypePropMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_SET_EXP     :
        {
            static SfxItemPropertyMap aSetExpFieldTypePropMap[] =
            {
                {SW_PROP_NAME(UNO_NAME_CHAPTER_NUMBERING_LEVEL),0,  &::getCppuType((sal_Int8*)0), PROPERTY_NONE,    0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  new uno::Type(::getCppuType((Sequence<Reference<XDependentTextField> >*)0)), PropertyAttribute::READONLY, 0},
#else
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  &::getCppuType((Sequence<Reference<XDependentTextField> >*)0), PropertyAttribute::READONLY, 0},
#endif
                {SW_PROP_NAME(UNO_NAME_NAME),               0,  &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_NUMBERING_SEPARATOR), 0, &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {SW_PROP_NAME(UNO_NAME_SUB_TYPE),           0,  &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                {0,0,0,0}
            };
            pRet = aSetExpFieldTypePropMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_DATABASE    :
        {
            static SfxItemPropertyMap aDBFieldTypePropMap           [] =
            {
                {SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME   ), 0, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME  ), 0, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_DATA_COLUMN_NAME ), 0, &::getCppuType((const OUString*)0),   PROPERTY_NONE, 0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  new uno::Type(::getCppuType((Sequence<Reference<XDependentTextField> >*)0)), PropertyAttribute::READONLY, 0},
#else
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  &::getCppuType((Sequence<Reference<XDependentTextField> >*)0), PropertyAttribute::READONLY, 0},
#endif
                {0,0,0,0}
            };
            pRet = aDBFieldTypePropMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_DUMMY2      :
        case SW_SERVICE_FIELDMASTER_DUMMY3      :
        case SW_SERVICE_FIELDMASTER_DUMMY4      :
        case SW_SERVICE_FIELDMASTER_DUMMY5      :
        {
            static SfxItemPropertyMap aStandardFieldMasterMap[] =
            {
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  new uno::Type(::getCppuType((Sequence<Reference<XDependentTextField> >*)0)), PropertyAttribute::READONLY, 0},
#else
                {SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  &::getCppuType((Sequence<Reference<XDependentTextField> >*)0), PropertyAttribute::READONLY, 0},
#endif
                {SW_PROP_NAME(UNO_NAME_NAME),               0,  &::getCppuType((const OUString*)0), PROPERTY_NONE,  0},
                {0,0,0,0}
            };
            pRet = aStandardFieldMasterMap;
        }
        break;
        case SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY:
        {
            static SfxItemPropertyMap aBibliographyFieldMap[] =
            {
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_FIELDS  )    , 0, new uno::Type(::getCppuType((Sequence<PropertyValue>*)0)),PROPERTY_NONE, 0},
#else
                {SW_PROP_NAME(UNO_NAME_FIELDS  )    , 0, &::getCppuType((Sequence<PropertyValue>*)0),PROPERTY_NONE, 0},
#endif
                {0,0,0,0}
            };
            pRet = aBibliographyFieldMap;
        }
        break;
        case SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY:
        {
            static SfxItemPropertyMap aBibliographyFieldMasterMap[] =
            {
                {SW_PROP_NAME(UNO_NAME_BRACKET_BEFORE     ) , 0, &::getCppuType((OUString*)0),               PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_BRACKET_AFTER      ) , 0, &::getCppuType((OUString*)0),               PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_NUMBER_ENTRIES  ) , 0, &::getBooleanCppuType(),                    PROPERTY_NONE, 0},
                {SW_PROP_NAME(UNO_NAME_IS_SORT_BY_POSITION) , 0, &::getBooleanCppuType(),                    PROPERTY_NONE, 0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                {SW_PROP_NAME(UNO_NAME_SORT_KEYS          ) , 0, new uno::Type(::getCppuType((Sequence<PropertyValues>*)0)),PROPERTY_NONE, 0},
#else
                {SW_PROP_NAME(UNO_NAME_SORT_KEYS          ) , 0, &::getCppuType((Sequence<PropertyValues>*)0),PROPERTY_NONE, 0},
#endif
                {0,0,0,0}
            };
            pRet = aBibliographyFieldMasterMap;
        }
        break;
    }
    DBG_ASSERT(pRet, "illegal service id")
    return pRet;
};

const sal_uInt16 aDocInfoSubTypeFromService[] =
{
    DI_CHANGE|DI_SUB_AUTHOR,    //SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR
    DI_CHANGE|DI_SUB_DATE,      //SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME
    DI_EDIT|DI_SUB_TIME,        //SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME
    DI_COMMENT,                 //SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION
    DI_CREATE|DI_SUB_AUTHOR,    //SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR
    DI_CREATE|DI_SUB_DATE,      //SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME
    DI_INFO1,                   //SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0
    DI_INFO2,                   //SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1
    DI_INFO3,                   //SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2
    DI_INFO4,                   //SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3
    DI_PRINT|DI_SUB_AUTHOR,     //SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR
    DI_PRINT|DI_SUB_DATE,       //SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME
    DI_KEYS,                    //SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS
    DI_THEMA,                   //SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT
    DI_TITEL,                   //SW_SERVICE_FIELDTYPE_DOCINFO_TITLE
    DI_DOCNO                    //SW_SERVICE_FIELDTYPE_DOCINFO_REVISION
};
struct ServiceIdResId
{
    USHORT nResId;
    USHORT nServiceId;
};
const ServiceIdResId aServiceToRes[] =
{
    {RES_DATETIMEFLD,   SW_SERVICE_FIELDTYPE_DATETIME               },
    {RES_USERFLD,       SW_SERVICE_FIELDTYPE_USER                   },
    {RES_SETEXPFLD,         SW_SERVICE_FIELDTYPE_SET_EXP            }    ,
    {RES_GETEXPFLD,         SW_SERVICE_FIELDTYPE_GET_EXP            }    ,
    {RES_FILENAMEFLD,   SW_SERVICE_FIELDTYPE_FILE_NAME              },
    {RES_PAGENUMBERFLD,     SW_SERVICE_FIELDTYPE_PAGE_NUM           }    ,
    {RES_AUTHORFLD,         SW_SERVICE_FIELDTYPE_AUTHOR             }    ,
    {RES_CHAPTERFLD,    SW_SERVICE_FIELDTYPE_CHAPTER                },
    {RES_GETREFFLD,         SW_SERVICE_FIELDTYPE_GET_REFERENCE      }    ,
    {RES_HIDDENTXTFLD,  SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT       },
    {RES_POSTITFLD,         SW_SERVICE_FIELDTYPE_ANNOTATION         }    ,
    {RES_INPUTFLD,      SW_SERVICE_FIELDTYPE_INPUT                  },
    {RES_MACROFLD,      SW_SERVICE_FIELDTYPE_MACRO                  },
    {RES_DDEFLD,        SW_SERVICE_FIELDTYPE_DDE                    },
    {RES_HIDDENPARAFLD,     SW_SERVICE_FIELDTYPE_HIDDEN_PARA        }    ,
    {RES_DOCINFOFLD,    SW_SERVICE_FIELDTYPE_DOC_INFO               },
    {RES_TEMPLNAMEFLD,  SW_SERVICE_FIELDTYPE_TEMPLATE_NAME          },
    {RES_EXTUSERFLD,    SW_SERVICE_FIELDTYPE_USER_EXT               },
    {RES_REFPAGESETFLD,     SW_SERVICE_FIELDTYPE_REF_PAGE_SET       }    ,
    {RES_REFPAGEGETFLD,     SW_SERVICE_FIELDTYPE_REF_PAGE_GET       }    ,
    {RES_JUMPEDITFLD,   SW_SERVICE_FIELDTYPE_JUMP_EDIT              },
    {RES_SCRIPTFLD,         SW_SERVICE_FIELDTYPE_SCRIPT             }    ,
    {RES_DBNEXTSETFLD,  SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET      },
    {RES_DBNUMSETFLD,   SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET       },
    {RES_DBSETNUMBERFLD, SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM      } ,
    {RES_DBFLD,             SW_SERVICE_FIELDTYPE_DATABASE           }    ,
    {RES_DBNAMEFLD,     SW_SERVICE_FIELDTYPE_DATABASE_NAME          },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_PAGE_COUNT             },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT        },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_WORD_COUNT             },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_CHARACTER_COUNT        },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_TABLE_COUNT            },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT   },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT  },
    {RES_DOCINFOFLD,    SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR  },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME},
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME       },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION     },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR   },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME},
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0          },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1          },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2          },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3          },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR    },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS       },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT         },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_TITLE           },
    {RES_INPUTFLD,      SW_SERVICE_FIELDTYPE_INPUT_USER                  },
    {RES_HIDDENTXTFLD,  SW_SERVICE_FIELDTYPE_HIDDEN_TEXT                 },
    {RES_AUTHORITY,     SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY                },
    {USHRT_MAX,         USHRT_MAX                                        }
};
//-----------------------------------------------------------------
sal_uInt16 lcl_ServiceIdToResId(sal_uInt16 nServiceId)
{
    USHORT nIndex = 0;
    while(aServiceToRes[nIndex].nServiceId != USHRT_MAX)
    {
        if(aServiceToRes[nIndex].nServiceId == nServiceId)
            return aServiceToRes[nIndex].nResId;
        nIndex++;
    }
    DBG_ERROR("service id not found")
    return USHRT_MAX;
}
//-----------------------------------------------------------------
sal_uInt16 lcl_GetServiceForResId(sal_uInt16 nWhich)
{
    USHORT nIndex = 0;
    while(aServiceToRes[nIndex].nResId != USHRT_MAX)
    {
        if(aServiceToRes[nIndex].nResId == nWhich)
            return aServiceToRes[nIndex].nServiceId;
        nIndex++;
    }
    DBG_ERROR("resid id not found")
    return USHRT_MAX;
}
/******************************************************************
 * SwXFieldMaster
 ******************************************************************/
TYPEINIT1(SwXFieldMaster, SwClient);
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const ::com::sun::star::uno::Sequence< sal_Int8 > & SwXFieldMaster::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXFieldMaster::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldMaster::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFieldMaster");
}
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFieldMaster::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextFieldMaster") == rServiceName;
}
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFieldMaster::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextFieldMaster");
    return aRet;
}
/*-- 14.12.98 11:08:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFieldMaster::SwXFieldMaster(SwDoc* pDoc, sal_uInt16 nResId) :
    m_pDoc(pDoc),
    aLstnrCntnr( (XPropertySet*)this),
    nResTypeId(nResId),
    m_bIsDescriptor(sal_True),
    fParam1(0.),
    bParam1(FALSE),
    nParam1(-1)
{
}
/*-- 14.12.98 11:08:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFieldMaster::SwXFieldMaster(SwFieldType& rType, SwDoc* pDoc) :
    SwClient(&rType),
    aLstnrCntnr( (XPropertySet*)this),
    m_pDoc(pDoc),
    nResTypeId(rType.Which()),
    m_bIsDescriptor(sal_False),
    fParam1(0.),
    nParam1(-1),
    bParam1(FALSE)
{

}
/*-- 14.12.98 11:08:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFieldMaster::~SwXFieldMaster()
{

}
/*-- 14.12.98 11:08:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySetInfo >  SwXFieldMaster::getPropertySetInfo(void)
                                            throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    const SfxItemPropertyMap* pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_DUMMY2);
    if(nResTypeId == RES_USERFLD)
        pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_USER);
    else if(nResTypeId == RES_DBFLD)
        pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_DATABASE);
    else if(nResTypeId == RES_SETEXPFLD)
        pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_SET_EXP);
    else if(nResTypeId == RES_DDEFLD)
        pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_DDE);
    else if(nResTypeId == RES_AUTHORITY)
        pCreate = SwFieldPropMapProvider::GetPropertyMap(SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY);
    uno::Reference< XPropertySetInfo >  aRef = new SfxItemPropertySetInfo(pCreate);
    return aRef;
}
/*-- 14.12.98 11:08:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
            IllegalArgumentException, WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFieldType* pType = GetFldType();
    if(pType)
    {
        sal_Bool bSetValue = sal_True;
        if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_SUB_TYPE))
        {
            const SvStringsDtor& rExtraArr = m_pDoc->GetExtraNmArray();
            String sTypeName = pType->GetName();
            static sal_uInt16 nIds[] =
            {
                RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
                0
            };
            for(const sal_uInt16 * pIds = nIds; *pIds; ++pIds)
            {
                if(sTypeName == *rExtraArr[ *pIds ] )
                {
                    bSetValue = sal_False;
                    break;
                }
            }
        }
        if(bSetValue)
            pType->PutValue(aValue, rPropertyName);
    }
    else if(!pType && m_pDoc &&
        ( COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_NAME)
            || COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_BASE_NAME)))
    {
        OUString uTmp;
        aValue >>= uTmp;
        String sTypeName(uTmp);
        SwFieldType* pType = m_pDoc->GetFldType(nResTypeId, sTypeName);
        if(pType ||
            (RES_SETEXPFLD == nResTypeId && sTypeName != String(SwXFieldMaster::GetSetExpProgrammaticName(sTypeName))))
        {
            throw IllegalArgumentException();
        }
        else
        {
            switch(nResTypeId)
            {
                case RES_USERFLD :
                {
                    SwUserFieldType aType(m_pDoc, sTypeName);
                    pType = m_pDoc->InsertFldType(aType);
                    ((SwUserFieldType*)pType)->SetContent(sParam1);
                    ((SwUserFieldType*)pType)->SetValue(fParam1);
                    ((SwUserFieldType*)pType)->SetType(bParam1 ? GSE_EXPR : GSE_STRING);
                }
                break;
                case RES_DBFLD :
                {
                    String sDBName = sTypeName;//dbname
                    sDBName += DB_DELIM;
                    sDBName += sParam2; //table
                    //sParam3 //column
                    SwDBFieldType aType(m_pDoc, sParam3,  sDBName);
                    pType = m_pDoc->InsertFldType(aType);
                }
                break;
                case RES_DDEFLD :
                {
                    SwDDEFieldType aType(sTypeName, sParam1,
                                bParam1 ? LINKUPDATE_ALWAYS : LINKUPDATE_ONCALL);
                    pType = m_pDoc->InsertFldType(aType);
                }
                break;
                case RES_SETEXPFLD :
                {
                    SwSetExpFieldType aType(m_pDoc, sTypeName);
                    if(sParam1.Len())
                        aType.SetDelimiter( sParam1.GetChar(0));
                    if(nParam1 > -1 && nParam1 < MAXLEVEL)
                        aType.SetOutlineLvl(nParam1);
                    pType = m_pDoc->InsertFldType(aType);
                }
                break;
            }
            if(pType)
                pType->Add(this);
            else
                throw uno::RuntimeException();
        }

        DBG_ASSERT(pType, "kein FieldType gefunden!" );
        pType->Add(this);
    }
    else
    {
        if(nResTypeId == RES_USERFLD)
        {
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CONTENT))
            {
                OUString uTmp;
                aValue >>= uTmp;
                sParam1 = String(uTmp);
            }
            else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_VALUE ))
            {
                if(aValue.getValueType() != ::getCppuType((const Double*)0))
                    throw IllegalArgumentException();
                fParam1 = *(Double*)aValue.getValue();
            }
            else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_IS_EXPRESSION ))
            {
                if(aValue.getValueType() != ::getBooleanCppuType())
                    throw IllegalArgumentException();
                bParam1 = *(sal_Bool*)aValue.getValue();
            }
        }
        else if(RES_DBFLD == nResTypeId)
        {
            OUString uTmp;
            aValue >>= uTmp;
            String sTmp(uTmp);
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_BASE_NAME))
                sParam1 = sTmp;
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_TABLE_NAME))
                sParam2 = sTmp;
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_COLUMN_NAME))
                sParam3 = sTmp;
        }
        else if(RES_SETEXPFLD == nResTypeId)
        {
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_NUMBERING_SEPARATOR))
            {
                OUString uTmp;
                aValue >>= uTmp;
                sParam1 = uTmp;
            }
            else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CHAPTER_NUMBERING_LEVEL))
            {
                aValue >>= nParam1;
            }
        }
        else if(RES_SETEXPFLD == nResTypeId)
        {
            USHORT nPart = COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DDE_COMMAND_TYPE)  ? 0 :
                COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DDE_COMMAND_FILE)  ? 1 :
                    COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DDE_COMMAND_ELEMENT)  ? 2 :
                    COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_IS_AUTOMATIC_UPDATE) ? 3 : USHRT_MAX;
            if(nPart  < 3 )
            {
                OUString uTmp;
                aValue >>= uTmp;
                if(!sParam1.Len())
                {
                    sParam1 = cTokenSeperator;
                    sParam1 += cTokenSeperator;
                }
                sParam1.SetToken(nPart, cTokenSeperator, uTmp);
            }
            else if(3 == nPart)
            {
                bParam1 = *(sal_Bool*)aValue.getValue();
            }
        }
        else
            throw UnknownPropertyException();
    }

}
/*-- 14.12.98 11:08:36---------------------------------------------------

  -----------------------------------------------------------------------*/
typedef SwFmtFld* SwFmtFldPtr;
SV_DECL_PTRARR(SwDependentFields, SwFmtFldPtr, 5, 5)
SV_IMPL_PTRARR(SwDependentFields, SwFmtFldPtr)

uno::Any SwXFieldMaster::getPropertyValue(const OUString& rPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFieldType* pType = GetFldType();
    if(pType)
    {
        if(COMPARE_EQUAL == rPropertyName.compareToAscii("Name"))
        {
            aRet <<= OUString(SwXFieldMaster::GetProgrammaticName(*pType, *GetDoc()));
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DEPENDENT_TEXT_FIELDS))
        {
            //fill all text fields into a sequence
            SwClientIter aIter( *pType );
            SwDependentFields aFldArr;
            SwFmtFldPtr pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
            while(pFld)
            {
                if(pFld->IsFldInDoc())
                    aFldArr.Insert(pFld, aFldArr.Count());
                pFld = (SwFmtFld*)aIter.Next();
            }
            Sequence<Reference <XDependentTextField> > aRetSeq(aFldArr.Count());
            Reference<XDependentTextField>* pRetSeq = aRetSeq.getArray();
            SwXTextField* pInsert = 0;
            for(int i = 0; i < aFldArr.Count(); i++)
            {
                pFld = aFldArr.GetObject(i);
                SwXTextField* pTemp = (SwXTextField*)aIter.First(TYPE(SwXTextField));
                while(pTemp)
                {
                    if(pTemp->GetFldFmt() == pFld)
                    {
                        pInsert = pTemp;
                        break;
                    }
                    pTemp = (SwXTextField*)aIter.Next();
                }
                if(!pInsert)
                    pInsert = new SwXTextField( *pFld, GetDoc());
                pRetSeq[i] = Reference<XDependentTextField>(pInsert);
                pInsert = 0;
            }
            aRet <<= aRetSeq;
        }
        else if(pType)
        {   //TODO: Properties fuer die uebrigen Feldtypen einbauen
            pType->QueryValue(aRet, rPropertyName);
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 11:08:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:08:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:08:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:08:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

/*-- 25.02.99 11:01:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::dispose(void)          throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFieldType* pFldType = GetFldType();
    if(pFldType)
    {
        sal_uInt16 nTypeIdx = USHRT_MAX;
        const SwFldTypes* pTypes = GetDoc()->GetFldTypes();
        for( sal_uInt16 i = 0; i < pTypes->Count(); i++ )
        {
            if((*pTypes)[i] == pFldType)
                nTypeIdx = i;
        }

        // zuerst alle Felder loeschen
        SwClientIter aIter( *pFldType );
        SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
        while(pFld)
        {
            // Feld im Undo?
            SwTxtFld *pTxtFld = pFld->GetTxtFld();
            if(pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)*pTxtFld->GetpTxtNode();
                SwPaM aPam(rTxtNode, *pTxtFld->GetStart());
                aPam.SetMark();
                aPam.Move();
                GetDoc()->DeleteAndJoin(aPam);
            }
            pFld = (SwFmtFld*)aIter.Next();
        }
        // dann den FieldType loeschen
        GetDoc()->RemoveFldType(nTypeIdx);
    }
    else
        throw uno::RuntimeException();
}
/*-- 25.02.99 11:02:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::addEventListener(const uno::Reference< XEventListener > & aListener)
                                            throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 25.02.99 11:02:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::removeEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

/*-- 14.12.98 11:08:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFieldMaster::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        aLstnrCntnr.Disposing();
        m_pDoc = 0;
    }
}
/* -----------------------------06.11.00 09:44--------------------------------

 ---------------------------------------------------------------------------*/
const Programmatic2UIName_Impl* lcl_GetFieldNameTable()
{
    static BOOL bInitialized = FALSE;
    static Programmatic2UIName_Impl aFieldNames[5];
    if(!bInitialized)
    {
        bInitialized = TRUE;
        int nName = 0;
        aFieldNames[nName].sUIName = String             (SW_RES(STR_POOLCOLL_LABEL_ABB ));
        aFieldNames[nName++].sProgrammaticName = String (SW_RES(STR_POCO_PRGM_LABEL_ABB));
        aFieldNames[nName].sUIName = String             (SW_RES(STR_POOLCOLL_LABEL_TABLE ));
        aFieldNames[nName++].sProgrammaticName = String (SW_RES(STR_POCO_PRGM_LABEL_TABLE));
        aFieldNames[nName].sUIName = String             (SW_RES(STR_POOLCOLL_LABEL_FRAME));
        aFieldNames[nName++].sProgrammaticName = String (SW_RES(STR_POCO_PRGM_LABEL_FRAME));
        aFieldNames[nName].sUIName = String             (SW_RES(STR_POOLCOLL_LABEL_DRAWING ));
        aFieldNames[nName++].sProgrammaticName = String (SW_RES(STR_POCO_PRGM_LABEL_DRAWING));
    }
    return &aFieldNames[0];
}
/* -----------------------------06.11.00 10:26--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldMaster::GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc)
{
    OUString sRet(rType.GetName());
    if(RES_SETEXPFLD == rType.Which())
    {
        const SwFldTypes* pTypes = rDoc.GetFldTypes();
        for( sal_uInt16 i = 0; i <= INIT_FLDTYPES; i++ )
        {
            if((*pTypes)[i] == &rType)
            {
                const Programmatic2UIName_Impl* pTable = lcl_GetFieldNameTable();
                while(pTable->sUIName.Len())
                {
                    if(sRet == OUString(pTable->sUIName))
                    {
                        sRet = pTable->sProgrammaticName;
                        break;
                    }
                    ++pTable;
                }
                break;
            }
        }
    }
    return sRet;
}
/* -----------------------------06.11.00 10:57--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldMaster::GetSetExpProgrammaticName(const OUString& rUIName)
{
    const Programmatic2UIName_Impl* pTable = lcl_GetFieldNameTable();
    OUString sRet(rUIName);
    while(pTable->sUIName.Len())
    {
        if(sRet == OUString(pTable->sUIName))
        {
            sRet = pTable->sProgrammaticName;
            break;
        }
        ++pTable;
    }
    return sRet;
}
/* -----------------------------06.11.00 10:57--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldMaster::GetSetExpUIName(const rtl::OUString& rName)
{
    const Programmatic2UIName_Impl* pTable = lcl_GetFieldNameTable();
    OUString sRet(rName);
    while(pTable->sUIName.Len())
    {
        if(sRet == OUString(pTable->sProgrammaticName))
        {
            sRet = pTable->sUIName;
            break;
        }
        ++pTable;
    }
    return sRet;
}
/* -----------------------------06.11.00 14:12--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldMaster::LocalizeFormula(
    const SwSetExpField& rFld,
    const OUString& rFormula,
    sal_Bool bQuery)
{
    OUString sTypeName(rFld.GetTyp()->GetName());
    OUString sProgName = SwXFieldMaster::GetSetExpProgrammaticName(sTypeName);
    if(sProgName != sTypeName)
    {
        OUString sSource = bQuery ? sTypeName : sProgName;
        OUString sDest = bQuery ? sProgName : sTypeName;
        if(!rFormula.compareTo(sSource, sSource.getLength()))
        {
            OUString sTmpFormula = sDest;
            sTmpFormula += rFormula.copy(sSource.getLength());
            return sTmpFormula;
        }
    }
    return rFormula;
}
/******************************************************************
 *
 ******************************************************************/
struct SwFieldProperties_Impl
{
    String      sPar1;
    String      sPar2;
    String      sPar3;
    String      sPar4;
    sal_Int32       nSubType;
    sal_Int32       nFormat;
    sal_uInt16      nUSHORT1;
    sal_uInt16      nUSHORT2;
    sal_Int16       nSHORT1;
    sal_Int8        nByte1;
    sal_Bool        bBool1;
    sal_Bool        bBool2;
    sal_Bool        bBool3;
    Date            aDate;
    Double          fDouble;
    util::DateTime* pDateTime;

    SwFieldProperties_Impl():
        nSubType(0),
        nFormat(0),
        nUSHORT1(0),
        nUSHORT2(0),
        nSHORT1(0),
        nByte1(0),
        fDouble(0.),
        bBool1(sal_False),
        bBool2(sal_False),
        bBool3(sal_False),
        pDateTime(0)
        {}
    ~SwFieldProperties_Impl()
        {delete pDateTime;}

};

TYPEINIT1(SwXTextField, SwClient);
/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const ::com::sun::star::uno::Sequence< sal_Int8 > & SwXTextField::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextField::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/*-- 14.12.98 11:37:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextField::SwXTextField(sal_uInt16 nServiceId) :
    pFmtFld(0),
    aLstnrCntnr( (XTextContent*)this),
    m_pDoc(0),
    m_nServiceId(nServiceId),
    m_bIsDescriptor(nServiceId != USHRT_MAX),
    m_pProps(new SwFieldProperties_Impl)
{
    //Set visible as default!
    if(SW_SERVICE_FIELDTYPE_SET_EXP == nServiceId)
        m_pProps->bBool2 = sal_True;
}
/*-- 14.12.98 11:37:15---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextField::SwXTextField(const SwFmtFld& rFmt, SwDoc* pDc) :
    pFmtFld(&rFmt),
    aLstnrCntnr( (XTextContent*)this),
    m_pDoc(pDc),
    m_nServiceId(USHRT_MAX),
    m_bIsDescriptor(sal_False),
    m_pProps(0)
{
    pFmtFld->GetFld()->GetTyp()->Add(this);
    //TODO: GetObject impl., darin soll das fuer dieses FmtFld bereits vorhandene
    //Objekt gesucht werden
    USHORT nResId(pFmtFld->GetFld()->GetTyp()->Which());
    m_nServiceId = lcl_GetServiceForResId(nResId);
    //special handling of SwInputField

    if(RES_INPUTFLD == nResId && (((SwInputField*)pFmtFld->GetFld())->GetSubType()& 0x00ff) == INP_USR)
        m_nServiceId = SW_SERVICE_FIELDTYPE_INPUT_USER;
    else if(RES_DOCINFOFLD == nResId)
    {
        USHORT nSubType = ((SwDocInfoField*)pFmtFld->GetFld())->GetSubType();
        if(DI_CHANGE == (nSubType & 0xff))
        {
            m_nServiceId = ((nSubType&0x300) == DI_SUB_AUTHOR) ?
                SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR :
                    SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME;
        }
        else if(DI_CREATE == (nSubType & 0xff))
        {
            m_nServiceId = ((nSubType&0x300) == DI_SUB_AUTHOR) ?
                SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR :
                    SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME;
        }
        else if(DI_PRINT == (nSubType & 0xff))
        {
            m_nServiceId = ((nSubType&0x300) == DI_SUB_AUTHOR) ?
                SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR :
                    SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME;
        }
        else if(DI_EDIT == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME;
        else if(DI_COMMENT == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION;
        else if(DI_INFO1 == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0;
        else if(DI_INFO2 == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1;
        else if(DI_INFO3 == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2;
        else if(DI_INFO4 == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3;
        else if(DI_KEYS == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS;
        else if(DI_THEMA == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT;
        else if(DI_TITEL == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_TITLE;
        else if(DI_DOCNO == (nSubType & 0xff))
            m_nServiceId = SW_SERVICE_FIELDTYPE_DOCINFO_REVISION;
    }
    else if(RES_HIDDENTXTFLD == nResId)
    {
        m_nServiceId = ((SwHiddenTxtField*)pFmtFld->GetFld())->GetSubType() == TYP_CONDTXTFLD ?
            SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT : SW_SERVICE_FIELDTYPE_HIDDEN_TEXT;
    }
    else if(RES_DOCSTATFLD == nResId)
    {
        switch(((SwDocStatField*)pFmtFld->GetFld())->GetSubType())
        {
            case DS_PAGE: m_nServiceId = SW_SERVICE_FIELDTYPE_PAGE_COUNT; break;
            case DS_PARA: m_nServiceId = SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT; break;
            case DS_WORD: m_nServiceId = SW_SERVICE_FIELDTYPE_WORD_COUNT     ; break;
            case DS_CHAR: m_nServiceId = SW_SERVICE_FIELDTYPE_CHARACTER_COUNT; break;
            case DS_TBL:  m_nServiceId = SW_SERVICE_FIELDTYPE_TABLE_COUNT    ; break;
            case DS_GRF:  m_nServiceId = SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT; break;
            case DS_OLE:  m_nServiceId = SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT; break;
        }
    }
}
/*-- 14.12.98 11:37:15---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextField::~SwXTextField()
{
    delete m_pProps;
}
/*-- 14.12.98 11:37:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::attachTextFieldMaster(const uno::Reference< XPropertySet > & xFieldMaster)
                    throw( IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference< XUnoTunnel > xMasterTunnel(xFieldMaster, uno::UNO_QUERY);
    SwXFieldMaster* pMaster = (SwXFieldMaster*)xMasterTunnel->getSomething(
                SwXFieldMaster::getUnoTunnelId());

    SwFieldType* pFieldType = pMaster ? pMaster->GetFldType() : 0;
    if(pFieldType && pFieldType->Which() == lcl_ServiceIdToResId(m_nServiceId))
    {
        m_sTypeName = pFieldType->GetName();
    }
    else
        throw IllegalArgumentException();

}
/*-- 14.12.98 11:37:16---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySet >  SwXTextField::getTextFieldMaster(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFieldType* pType = GetFldType();
    if(!pType)
        throw uno::RuntimeException();
    SwXFieldMaster* pMaster = (SwXFieldMaster*)
                SwClientIter(*pType).First(TYPE(SwXFieldMaster));
    if(!pMaster)
        pMaster = new SwXFieldMaster(*pType, GetDoc());

    return pMaster;
}
/*-- 14.12.98 11:37:16---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextField::getPresentation(sal_Bool bShowCommand) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    OUString sRet;
    const SwField* pField = GetField();
    if(pField)
        sRet = pField->GetCntnt(bShowCommand);
    else
        throw uno::RuntimeException();
    return sRet;
}
/* -----------------18.02.99 13:39-------------------
 *
 * --------------------------------------------------*/
void SwXTextField::attachToRange(
        const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    //wurde ein FieldMaster attached, dann ist das Dokument schon festgelegt!
    if(pDoc && (!m_pDoc || m_pDoc == pDoc))
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        SwField* pFld = 0;
        switch(m_nServiceId)
        {
            case SW_SERVICE_FIELDTYPE_ANNOTATION:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_POSTITFLD);
                pFld = new SwPostItField((SwPostItFieldType*)pFldType,
                        m_pProps->sPar1, m_pProps->sPar2,
                        m_pProps->aDate);
            }
            break;
            case SW_SERVICE_FIELDTYPE_SCRIPT:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_SCRIPTFLD);
                pFld = new SwScriptField((SwScriptFieldType*)pFldType,
                        m_pProps->sPar1, m_pProps->sPar2,
                        m_pProps->bBool1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATETIME:
            {
                sal_uInt16 nSub = 0;
                if(m_pProps->bBool1)
                    nSub |= FIXEDFLD;
                if(m_pProps->bBool2)
                    nSub |= DATEFLD;
                else
                    nSub |= TIMEFLD;
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DATETIMEFLD);
                pFld = new SwDateTimeField((SwDateTimeFieldType*)pFldType,
                nSub, m_pProps->nFormat);
                if(m_pProps->fDouble > 0.)
                    ((SwDateTimeField*)pFld)->SetValue( m_pProps->fDouble );
                if(m_pProps->pDateTime)
                {
                    Any aVal; aVal <<= *m_pProps->pDateTime;
                    pFld->PutValue(aVal, C2U(UNO_NAME_DATE_TIME_VALUE.pName));
                }
                ((SwDateTimeField*)pFld)->SetOffset(m_pProps->nSubType);
            }
            break;
            case SW_SERVICE_FIELDTYPE_FILE_NAME:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_FILENAMEFLD);
                sal_Int32 nFormat = m_pProps->nFormat;
                if(m_pProps->bBool2)
                    nFormat |= FF_FIXED;
                pFld = new SwFileNameField((SwFileNameFieldType*)pFldType, nFormat);
                if(m_pProps->sPar3.Len())
                    ((SwFileNameField*)pFld)->SetExpansion(m_pProps->sPar3);
            }
            break;
            case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_TEMPLNAMEFLD);
                pFld = new SwTemplNameField((SwTemplNameFieldType*)pFldType,
                                                    m_pProps->nFormat);
            }
            break;
            case SW_SERVICE_FIELDTYPE_CHAPTER:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_CHAPTERFLD);
                pFld = new SwChapterField((SwChapterFieldType*)pFldType, m_pProps->nUSHORT1);
                ((SwChapterField*)pFld)->SetLevel(m_pProps->nByte1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_AUTHOR:
            {
                long nFormat = m_pProps->bBool1 ? AF_NAME : AF_SHORTCUT;
                if(m_pProps->bBool2)
                    nFormat |= AF_FIXED;

                SwFieldType* pFldType = pDoc->GetSysFldType(RES_AUTHORFLD);
                pFld = new SwAuthorField((SwAuthorFieldType*)pFldType, nFormat);
                ((SwAuthorField*)pFld)->SetExpansion(m_pProps->sPar1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT:
            case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_HIDDENTXTFLD);
                pFld = new SwHiddenTxtField(((SwHiddenTxtFieldType*)pFldType),
                        m_pProps->sPar1,
                        m_pProps->sPar2, m_pProps->sPar3,
                        SW_SERVICE_FIELDTYPE_HIDDEN_TEXT == m_nServiceId ?
                             TYP_HIDDENTXTFLD :
                                TYP_CONDTXTFLD);
            }
            break;
            case SW_SERVICE_FIELDTYPE_HIDDEN_PARA:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_HIDDENPARAFLD);
                pFld = new SwHiddenParaField((SwHiddenParaFieldType*)pFldType,
                                                m_pProps->sPar1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_GET_REFERENCE:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_GETREFFLD);
                pFld = new SwGetRefField((SwGetRefFieldType*)pFldType,
                            m_pProps->sPar1,
                            0,
                            0,
                            0);
                if(m_pProps->sPar3.Len())
                    ((SwGetRefField*)pFld)->SetExpand(m_pProps->sPar3);
                Any aVal; aVal <<=(sal_Int16)m_pProps->nUSHORT1;
                pFld->PutValue(aVal, C2U(UNO_NAME_REFERENCE_FIELD_PART));
                aVal <<=(sal_Int16)m_pProps->nUSHORT2;
                pFld->PutValue(aVal, C2U(UNO_NAME_REFERENCE_FIELD_SOURCE));
                aVal <<=(sal_Int16)m_pProps->nSHORT1;
                pFld->PutValue(aVal, C2U(UNO_NAME_SEQUENCE_NUMBER));
            }
            break;
            case SW_SERVICE_FIELDTYPE_JUMP_EDIT:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_JUMPEDITFLD);
                pFld = new SwJumpEditField((SwJumpEditFieldType*)pFldType,
                                m_pProps->nUSHORT1,
                                m_pProps->sPar1,
                                m_pProps->sPar2);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR     :
            case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME  :
            case SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME         :
            case SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION       :
            case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR     :
            case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME  :
            case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_0            :
            case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_1            :
            case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_2            :
            case SW_SERVICE_FIELDTYPE_DOCINFO_INFO_3            :
            case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR      :
            case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME   :
            case SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS         :
            case SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT           :
            case SW_SERVICE_FIELDTYPE_DOCINFO_TITLE             :
            case SW_SERVICE_FIELDTYPE_DOCINFO_REVISION          :
            case SW_SERVICE_FIELDTYPE_DOC_INFO:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DOCINFOFLD);
                sal_uInt16 nSubType = aDocInfoSubTypeFromService[
                        m_nServiceId - SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR];
                if(m_pProps->bBool2) //IsDate
                {
                    nSubType |= DI_SUB_DATE;
                    nSubType &= ~DI_SUB_TIME;
                }
                else
                {
                    nSubType |= DI_SUB_TIME;
                    nSubType &= ~DI_SUB_DATE;
                }
                if(m_pProps->bBool1)
                    nSubType |= DI_SUB_FIXED;
                pFld = new SwDocInfoField((SwDocInfoFieldType*)pFldType, nSubType, m_pProps->nFormat);
                if(m_pProps->sPar3.Len())
                    ((SwDocInfoField*)pFld)->SetExpansion(m_pProps->sPar3);
            }
            break;
            case SW_SERVICE_FIELDTYPE_USER_EXT:
            {
                sal_Int32 nFormat = 0;
                if(m_pProps->bBool1)
                    nFormat = AF_FIXED;

                SwFieldType* pFldType = pDoc->GetSysFldType(RES_EXTUSERFLD);
                pFld = new SwExtUserField((SwExtUserFieldType*)pFldType, m_pProps->nUSHORT1, nFormat);
                ((SwExtUserField*)pFld)->SetExpansion(m_pProps->sPar1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_USER:
            {
                SwFieldType* pFldType = pDoc->GetFldType(RES_USERFLD, m_sTypeName);
                if(!pFldType)
                    throw uno::RuntimeException();
                USHORT nUserSubType = m_pProps->bBool1 ? SUB_INVISIBLE : 0;
                if(m_pProps->bBool2)
                    nUserSubType |= SUB_CMD;
                pFld = new SwUserField((SwUserFieldType*)pFldType,
                                    nUserSubType,
                                    m_pProps->nFormat);
            }
            break;
            case SW_SERVICE_FIELDTYPE_REF_PAGE_SET:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_REFPAGESETFLD);
                pFld = new SwRefPageSetField( (SwRefPageSetFieldType*)pFldType,
                                    m_pProps->nUSHORT1,
                                    m_pProps->bBool1 );
            }
            break;
            case SW_SERVICE_FIELDTYPE_REF_PAGE_GET:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_REFPAGEGETFLD);
                pFld = new SwRefPageGetField( (SwRefPageGetFieldType*)pFldType,
                                                m_pProps->nUSHORT1 );
            }
            break;
            case SW_SERVICE_FIELDTYPE_PAGE_NUM:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_PAGENUMBERFLD);
                pFld = new SwPageNumberField((SwPageNumberFieldType*)pFldType,
                                                PG_RANDOM,
                                                m_pProps->nFormat,
                                                m_pProps->nUSHORT1);
                ((SwPageNumberField*)pFld)->SetUserString(m_pProps->sPar1);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DDE:
            {
                SwFieldType* pFldType = pDoc->GetFldType(RES_DDEFLD, m_sTypeName);
                if(!pFldType)
                    throw uno::RuntimeException();
                pFld = new SwDDEField( (SwDDEFieldType*)pFldType );
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATABASE_NAME:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBNAMEFLD);
                String sDBName(m_pProps->sPar1);
                sDBName += DB_DELIM;
                sDBName += m_pProps->sPar2;
                pFld = new SwDBNameField((SwDBNameFieldType*)pFldType, sDBName);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET:
            {
                String sDBName(m_pProps->sPar1);
                sDBName += DB_DELIM;
                sDBName += m_pProps->sPar2;
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBNEXTSETFLD);
                pFld = new SwDBNextSetField((SwDBNextSetFieldType*)pFldType,
                        m_pProps->sPar3, aEmptyStr,
                        sDBName);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET:
            {
                String sDBName(m_pProps->sPar1);
                sDBName += DB_DELIM;
                sDBName += m_pProps->sPar2;
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBNUMSETFLD);
                pFld = new SwDBNumSetField( (SwDBNumSetFieldType*)pFldType,
                    m_pProps->sPar3, String::CreateFromInt32(m_pProps->nFormat), sDBName);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM:
            {
                String sDBName(m_pProps->sPar1);
                sDBName += DB_DELIM;
                sDBName += m_pProps->sPar2;
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBSETNUMBERFLD);
                pFld = new SwDBSetNumberField(
                        (SwDBSetNumberFieldType*)pFldType,
                        sDBName,
                        m_pProps->nUSHORT1);
                ((SwDBSetNumberField*)pFld)->SetSetNumber(m_pProps->nFormat);
            }
            break;
            case SW_SERVICE_FIELDTYPE_DATABASE:
            {
                SwFieldType* pFldType = pDoc->GetFldType(RES_DBFLD, m_sTypeName);
                if(!pFldType)
                    throw uno::RuntimeException();
                pFld = new SwDBField((SwDBFieldType*)pFldType, m_pProps->nFormat);
                ((SwDBField*)pFld)->InitContent(m_pProps->sPar1);
                pFld->SetSubType(pFld->GetSubType()&SUB_OWN_FMT);
            }
            break;
            case SW_SERVICE_FIELDTYPE_SET_EXP:
            {
                SwFieldType* pFldType = pDoc->GetFldType(RES_SETEXPFLD, m_sTypeName);
                if(!pFldType)
                    throw uno::RuntimeException();
                pFld = new SwSetExpField((SwSetExpFieldType*)pFldType,
                    m_pProps->sPar2,
                    m_pProps->nUSHORT2);

                sal_uInt16  nSubType = pFld->GetSubType();
                if(m_pProps->bBool2)
                    nSubType &= ~SUB_INVISIBLE;
                else
                    nSubType |= SUB_INVISIBLE;
                if(m_pProps->bBool3)
                    nSubType |= SUB_CMD;
                else
                    nSubType &= ~SUB_CMD;
                pFld->SetSubType(nSubType);
                ((SwSetExpField*)pFld)->SetSeqNumber( m_pProps->nUSHORT1 );
                ((SwSetExpField*)pFld)->SetInputFlag(m_pProps->bBool1);
                ((SwSetExpField*)pFld)->SetPromptText(m_pProps->sPar3);
                if(m_pProps->sPar4.Len())
                    ((SwSetExpField*)pFld)->ChgExpStr(m_pProps->sPar4);

            }
            break;
            case SW_SERVICE_FIELDTYPE_GET_EXP:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_GETEXPFLD);
                if(!pFldType)
                    throw uno::RuntimeException();
                sal_uInt16 nSubType;
                switch(m_pProps->nSubType)
                {
                    case SetVariableType::STRING:   nSubType = GSE_STRING;  break;
                    case SetVariableType::VAR:      nSubType = GSE_EXPR;  break;
                    case SetVariableType::SEQUENCE: nSubType = GSE_SEQ;  break;
                    case SetVariableType::FORMULA:  nSubType = GSE_FORMULA; break;
                    default:
                        DBG_ERROR("wrong value");
                        nSubType = GSE_EXPR;
                }
                if(m_pProps->bBool2)
                    nSubType |= SUB_CMD;
                else
                    nSubType &= ~SUB_CMD;
                pFld = new SwGetExpField((SwGetExpFieldType*)pFldType,
                        m_pProps->sPar1, nSubType, m_pProps->nFormat);
                //TODO: SubType auswerten!
                if(m_pProps->sPar4.Len())
                    ((SwGetExpField*)pFld)->ChgExpStr(m_pProps->sPar4);
            }
            break;
            case SW_SERVICE_FIELDTYPE_INPUT_USER:
            case SW_SERVICE_FIELDTYPE_INPUT:
            {
                SwFieldType* pFldType = pDoc->GetFldType(RES_INPUTFLD, m_sTypeName);
                if(!pFldType)
                    throw uno::RuntimeException();
                USHORT nInpSubType = SW_SERVICE_FIELDTYPE_INPUT_USER == m_nServiceId ? INP_USR : INP_TXT;
                pFld = new SwInputField((SwInputFieldType*)pFldType,
                        m_pProps->sPar1, m_pProps->sPar2, nInpSubType);
            }
            break;
            case SW_SERVICE_FIELDTYPE_MACRO:
            {
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_MACROFLD);
                pFld = new SwMacroField((SwMacroFieldType*)pFldType,
                            m_pProps->sPar1, m_pProps->sPar2);
            }
            break;
            case SW_SERVICE_FIELDTYPE_PAGE_COUNT            :
            case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       :
            case SW_SERVICE_FIELDTYPE_WORD_COUNT            :
            case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       :
            case SW_SERVICE_FIELDTYPE_TABLE_COUNT           :
            case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  :
            case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT :
            {
                sal_uInt16 nSubType = DS_PAGE;
                switch(m_nServiceId)
                {
//                  case SW_SERVICE_FIELDTYPE_PAGE_COUNT            : break;
                    case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       : nSubType = DS_PARA;break;
                    case SW_SERVICE_FIELDTYPE_WORD_COUNT            : nSubType = DS_WORD;break;
                    case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       : nSubType = DS_CHAR;break;
                    case SW_SERVICE_FIELDTYPE_TABLE_COUNT           : nSubType = DS_TBL;break;
                    case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  : nSubType = DS_GRF;break;
                    case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT : nSubType = DS_OLE;break;
                }
                SwFieldType* pFldType = pDoc->GetSysFldType(RES_DOCSTATFLD);
                pFld = new SwDocStatField((SwDocStatFieldType*)pFldType, nSubType, m_pProps->nUSHORT2);
            }
            break;
            default: DBG_ERROR("was ist das fuer ein Typ?");
        }
        if(pFld)
        {
            SwFmtFld aFmt( *pFld );

            UnoActionContext aCont(pDoc);
            SwTxtAttr* pTxtAttr = 0;
            if(aPam.HasMark())
                pDoc->DeleteAndJoin(aPam);
            pDoc->Insert(aPam, aFmt);
            SwUnoCrsr* pCrsr = pDoc->CreateUnoCrsr( *aPam.Start() );
            pCrsr->SetMark();
            pCrsr->Left(1);
            pTxtAttr = pCrsr->GetNode()->GetTxtNode()->GetTxtAttr(pCrsr->GetPoint()->nContent, RES_TXTATR_FIELD);
            delete pCrsr;

            // was passiert mit dem Update der Felder ? (siehe fldmgr.cxx)
            if(pTxtAttr)
            {
                const SwFmtFld& rFld = pTxtAttr->GetFld();
                rFld.GetFld()->GetTyp()->Add(this);
                pFmtFld = &rFld;
            }
        }
        delete pFld;

        m_pDoc = pDoc;
        m_bIsDescriptor = sal_False;
        DELETEZ(m_pProps);
    }
    else
        throw IllegalArgumentException();

}
/*-- 14.12.98 11:37:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, uno::RuntimeException )
{
}
/*-- 14.12.98 11:37:18---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextField::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >   aRef;
    SwField* pField = (SwField*)GetField();
    if(pField)
    {
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();

        SwPaM aPam(rTxtNode, *pTxtFld->GetStart() + 1, rTxtNode, *pTxtFld->GetStart());

        aRef = ::CreateTextRangeFromPosition(m_pDoc,
                                *aPam.GetPoint(), aPam.GetMark());
    }
    return aRef;

}
/*-- 14.12.98 11:37:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwField* pField = (SwField*)GetField();
    if(pField)
    {
        UnoActionContext aContext(GetDoc());
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        SwTxtNode& rTxtNode = (SwTxtNode&)*pTxtFld->GetpTxtNode();
        SwPaM aPam(rTxtNode, *pTxtFld->GetStart());
        aPam.SetMark();
        aPam.Move();
        GetDoc()->DeleteAndJoin(aPam);
    }
}
/*-- 14.12.98 11:37:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::addEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 14.12.98 11:37:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 14.12.98 11:37:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySetInfo >  SwXTextField::getPropertySetInfo(void)
        throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    //kein static
    uno::Reference< XPropertySetInfo >  aRef;
    sal_uInt16 nPos = USHRT_MAX;
    if(GetFldType())
    {
        sal_uInt16 nResId = GetFldType()->Which();
        switch(nResId)
        {
            case  RES_DBFLD         : nPos = SW_SERVICE_FIELDTYPE_DATABASE;break;
            case  RES_USERFLD       : nPos = SW_SERVICE_FIELDTYPE_USER;break;
            case  RES_FILENAMEFLD   : nPos = SW_SERVICE_FIELDTYPE_FILE_NAME;break;
            case  RES_DBNAMEFLD     : nPos = SW_SERVICE_FIELDTYPE_DATABASE_NAME;break;
            case  RES_DATEFLD       : break;
            case  RES_TIMEFLD       : break;
            case  RES_PAGENUMBERFLD : nPos = SW_SERVICE_FIELDTYPE_PAGE_NUM;break;
            case  RES_AUTHORFLD     : nPos = SW_SERVICE_FIELDTYPE_AUTHOR;break;
            case  RES_CHAPTERFLD    : nPos = SW_SERVICE_FIELDTYPE_CHAPTER;break;
            case  RES_DOCSTATFLD    : //alle Statistik-Felder haben die gleiche PropertyMap
                nPos = SW_SERVICE_FIELDTYPE_PAGE_COUNT;
            break;
            case  RES_GETEXPFLD     : nPos = SW_SERVICE_FIELDTYPE_GET_EXP;break;
            case  RES_SETEXPFLD     : nPos = SW_SERVICE_FIELDTYPE_SET_EXP;break;
            case  RES_GETREFFLD     : nPos = SW_SERVICE_FIELDTYPE_GET_REFERENCE;break;
            case  RES_HIDDENTXTFLD  :
                nPos = m_nServiceId;
            break;
            case  RES_POSTITFLD     : nPos = SW_SERVICE_FIELDTYPE_ANNOTATION;break;
    //      case  RES_REGFLD        : nPos = ;break;
    //      case  RES_VARREGFLD     : nPos = ;break;
    //      case  RES_SETREFFLD     : nPos = ;break;
            case  RES_INPUTFLD      : nPos = SW_SERVICE_FIELDTYPE_INPUT;break;
            case  RES_MACROFLD      : nPos = SW_SERVICE_FIELDTYPE_MACRO;break;
            case  RES_DDEFLD        : nPos = SW_SERVICE_FIELDTYPE_DDE;break;
            case  RES_TABLEFLD      : nPos = SW_SERVICE_FIELDTYPE_TABLEFIELD;break;
            case  RES_HIDDENPARAFLD : nPos = SW_SERVICE_FIELDTYPE_HIDDEN_PARA;break;
            case  RES_DOCINFOFLD    : nPos = SW_SERVICE_FIELDTYPE_DOC_INFO;break;
            case  RES_TEMPLNAMEFLD  : nPos = SW_SERVICE_FIELDTYPE_TEMPLATE_NAME;break;
            case  RES_DBNEXTSETFLD  : nPos = SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET;break;
            case  RES_DBNUMSETFLD   : nPos = SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET;break;
            case  RES_DBSETNUMBERFLD: nPos = SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM;break;
            case  RES_EXTUSERFLD    : nPos = SW_SERVICE_FIELDTYPE_USER_EXT;break;
            case  RES_REFPAGESETFLD : nPos = SW_SERVICE_FIELDTYPE_REF_PAGE_SET;break;
            case  RES_REFPAGEGETFLD : nPos = SW_SERVICE_FIELDTYPE_REF_PAGE_GET;break;
            case  RES_INTERNETFLD   : 0;break;
            case  RES_JUMPEDITFLD   : nPos = SW_SERVICE_FIELDTYPE_JUMP_EDIT;break;
            case  RES_SCRIPTFLD     : nPos = SW_SERVICE_FIELDTYPE_SCRIPT;break;
            case  RES_DATETIMEFLD   :
            case  RES_FIXDATEFLD    :
            case  RES_FIXTIMEFLD    : nPos = SW_SERVICE_FIELDTYPE_DATETIME; break;
            case  RES_AUTHORITY:      nPos = SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY; break;
    //      case  RES_FIELDS_END    : nPos = ;break;
        }
    }
    else
        nPos = m_nServiceId;
    if(nPos != USHRT_MAX)
    {
        const SfxItemPropertyMap* pMap = SwFieldPropMapProvider::GetPropertyMap(nPos);
        uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(pMap);
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        aRef = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 14.12.98 11:37:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
        WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwField* pField = (SwField*)GetField();
    const SfxItemPropertyMap* _pMap = SwFieldPropMapProvider::GetPropertyMap(m_nServiceId);
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    if(pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
    if(pField)
    {
        // Sonderbehandlung Serienbrieffeld
        sal_uInt16 nWhich = GetFldType()->Which();
        if( RES_DBFLD == nWhich &&
            (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_BASE_NAME) ||
                COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_TABLE_NAME)||
                    COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DATA_COLUMN_NAME)))
        {
            // hier muss ein neuer Feldtyp angelegt werden und
            // das Feld an den neuen Typ umgehaengt werden
            DBG_WARNING("not implemented")
        }
        else
            pField->PutValue(aValue, rPropertyName);
    }
    else if(m_pProps)
    {
        switch(pMap->nWID)
        {
            case FIELD_PROP_PAR1:
            {
                OUString uTmp;
                aValue >>= uTmp;
                m_pProps->sPar1 = String(uTmp);
            }
            break;
            case FIELD_PROP_PAR2:
            {
                OUString uTmp;
                aValue >>= uTmp;
                m_pProps->sPar2 = String(uTmp);
            }
            break;
            case FIELD_PROP_PAR3:
            {
                OUString uTmp;
                aValue >>= uTmp;
                m_pProps->sPar3 = String(uTmp);
            }
            break;
            case FIELD_PROP_PAR4:
            {
                OUString uTmp;
                aValue >>= uTmp;
                m_pProps->sPar4 = String(uTmp);
            }
            break;
            case FIELD_PROP_FORMAT:
            {
                aValue >>= m_pProps->nFormat;
            }
            break;
            case FIELD_PROP_SUBTYPE:
            {
                aValue >>= m_pProps->nSubType;
            }
            break;
            case FIELD_PROP_BYTE1 :
                aValue >>= m_pProps->nByte1;
            break;
            case FIELD_PROP_BOOL1 :
            {
                if(aValue.getValueType() != getCppuBooleanType())
                    throw IllegalArgumentException();
                m_pProps->bBool1 = *(sal_Bool*)aValue.getValue();;
            }
            break;
            case FIELD_PROP_BOOL2 :
            {
                if(aValue.getValueType() != getCppuBooleanType())
                    throw IllegalArgumentException();
                m_pProps->bBool2 = *(sal_Bool*)aValue.getValue();;
            }
            break;
            case FIELD_PROP_BOOL3 :
            {
                if(aValue.getValueType() != getCppuBooleanType())
                    throw IllegalArgumentException();
                m_pProps->bBool3 = *(sal_Bool*)aValue.getValue();
            }
            break;
            case FIELD_PROP_DATE :
            {
                if(aValue.getValueType() != ::getCppuType((const util::Date*)0))
                    throw IllegalArgumentException();

                util::Date aTemp = *(const util::Date*)aValue.getValue();
                m_pProps->aDate = Date(aTemp.Day, aTemp.Month, aTemp.Year);
            }
            break;
            case FIELD_PROP_USHORT1:
            {
                sal_Int16 nVal;
                aValue >>= nVal;
                m_pProps->nUSHORT1 = nVal;
            }
            break;
            case FIELD_PROP_USHORT2:
            {
                sal_Int16 nVal;
                aValue >>= nVal;
                m_pProps->nUSHORT2 = nVal;
            }
            break;
            case FIELD_PROP_SHORT1:
            {
                aValue >>= m_pProps->nSHORT1;
            }
            break;
            case FIELD_PROP_DOUBLE:
            {
                if(aValue.getValueType() != ::getCppuType((const Double*)0))
                    throw IllegalArgumentException();
                m_pProps->fDouble = *(Double*)aValue.getValue();
            }
            break;
            case FIELD_PROP_DATE_TIME :
                if(!m_pProps->pDateTime)
                    m_pProps->pDateTime = new util::DateTime;
                aValue >>= (*m_pProps->pDateTime);
            break;
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 14.12.98 11:37:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextField::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    const SwField* pField = GetField();
    const SfxItemPropertyMap* _pMap = SwFieldPropMapProvider::GetPropertyMap(m_nServiceId);
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if(!pMap )
    {
        _pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS);
        pMap = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    }
    if(!pMap )
        throw UnknownPropertyException();
    if(FN_UNO_TEXT_WRAP == pMap->nWID)
    {
        aRet <<= (INT16) WrapTextMode_NONE;
    }
    else if(FN_UNO_ANCHOR_TYPE == pMap->nWID)
    {
        aRet <<= (INT16) TextContentAnchorType_AT_PARAGRAPH;
    }
    else if(FN_UNO_ANCHOR_TYPES == pMap->nWID)
    {
        uno::Sequence<TextContentAnchorType> aTypes(1);
        TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
        aRet.setValue(&aTypes, ::getCppuType((uno::Sequence<TextContentAnchorType>*)0));
    }
    else if(pField)
        pField->QueryValue(aRet, rPropertyName);
    else if(m_pProps)
    {
        switch(pMap->nWID)
        {
            case FIELD_PROP_PAR1:
                aRet <<= OUString(m_pProps->sPar1);
            break;
            case FIELD_PROP_PAR2:
                aRet <<= OUString(m_pProps->sPar2);
            break;
            case FIELD_PROP_PAR3:
                aRet <<= OUString(m_pProps->sPar3);
            break;
            case FIELD_PROP_PAR4:
                aRet <<= OUString(m_pProps->sPar4);
            break;
            case FIELD_PROP_FORMAT:
                aRet <<= m_pProps->nFormat;
            break;
            case FIELD_PROP_SUBTYPE:
                aRet <<= m_pProps->nSubType;
            break;
            case FIELD_PROP_BYTE1 :
                aRet <<= m_pProps->nByte1;
            break;
            case FIELD_PROP_BOOL1 :
                aRet.setValue(&m_pProps->bBool1, ::getCppuBooleanType());
            break;
            case FIELD_PROP_BOOL2 :
                aRet.setValue(&m_pProps->bBool2, ::getCppuBooleanType());
            break;
            case FIELD_PROP_DATE :
                aRet.setValue(&m_pProps->aDate, ::getCppuType((const util::Date*)0));
            break;
            case FIELD_PROP_USHORT1:
                aRet <<= (sal_Int16)m_pProps->nUSHORT1;
            break;
            case FIELD_PROP_USHORT2:
                aRet <<= (sal_Int16)m_pProps->nUSHORT2;
            break;
            case FIELD_PROP_SHORT1:
                aRet <<= m_pProps->nSHORT1;
            break;
            case FIELD_PROP_DOUBLE:
                aRet <<= m_pProps->fDouble;
            break;
            case FIELD_PROP_DATE_TIME :
                if(m_pProps->pDateTime)
                    aRet <<= (*m_pProps->pDateTime);
            break;
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 11:37:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:37:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:37:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 11:37:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextField::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/* -----------------19.03.99 14:11-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextField::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextField");
}
/* -----------------19.03.99 14:11-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextField::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    OUString sServiceName = SwXServiceProvider::GetProviderName(m_nServiceId);
    return sServiceName == rServiceName ||
        !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}
/* -----------------19.03.99 14:11-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextField::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    OUString sServiceName = SwXServiceProvider::GetProviderName(m_nServiceId);
    pArray[0] = sServiceName;
    pArray[1] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

/* -----------------14.12.98 12:00-------------------
 *
 * --------------------------------------------------*/
void SwXTextField::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetFldType())
    {
        aLstnrCntnr.Disposing();
        m_pDoc = 0;
    }
}
/*-- 14.12.98 11:37:21---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwField*  SwXTextField::GetField() const
{
    SwFieldType* pType = GetFldType();
    if(pType && pFmtFld)
    {
        // simpler Test: das Feld wird am Typ gesucht
        SwClientIter aIter(*pType);
        SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
        do
        {
            if(pFld == pFmtFld)
                return pFld->GetFld();
        }while(pFld && 0 != (pFld = (SwFmtFld*)aIter.Next()));
    }
    return 0;
}
/******************************************************************
 *
 ******************************************************************/
/******************************************************************
 * SwXTextFieldMasters
 ******************************************************************/
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextFieldMasters::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFieldMasters");
}
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextFieldMasters::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextFieldMasters") == rServiceName;
}
/* -----------------------------06.04.00 13:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextFieldMasters::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextFieldMasters");
    return aRet;
}
/*-- 21.12.98 10:37:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFieldMasters::SwXTextFieldMasters(SwDoc* pDoc) :
    SwUnoCollection(pDoc)
{
}
/*-- 21.12.98 10:37:32---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFieldMasters::~SwXTextFieldMasters()
{

}
/*-- 21.12.98 10:37:33---------------------------------------------------
    Iteration ueber nicht-Standard Feldtypen
    USER/SETEXP/DDE/DATABASE
    Der Name ist demnach:
    "com.sun.star.text.FieldMaster.User" + <Feltypname>
    "com.sun.star.text.FieldMaster.DDE" + <Feltypname>
    "com.sun.star.text.FieldMaster.SetExpression" + <Feltypname>
    "com.sun.star.text.FieldMaster.DataBase" + <Feltypname>

    Falls wir grosszuegig werden wollen, dann koennte man com.sun.star.text
    auch optional weglassen
  -----------------------------------------------------------------------*/
BOOL lcl_ConvertDatabaseName(String& rName)
{
    rName.Erase(0, 9); //DataBase.
    USHORT nDotCount = rName.GetTokenCount('.');
    if(nDotCount < 2)
       return FALSE;
    rName.SearchAndReplace('.', DB_DELIM);
    xub_StrLen  nFound = rName.SearchBackward( '.' );
        rName.SetChar(nFound, DB_DELIM);
    rName.Insert(C2S("DataBase."), 0);
    return TRUE;
}
//-----------------------------------------------------------------------------
uno::Any SwXTextFieldMasters::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!GetDoc())
        throw uno::RuntimeException();
    String sName(rName);
    if(COMPARE_EQUAL == sName.CompareToAscii("com.sun.star.text.FieldMaster.", 30))
    {
        sName.Erase(0, 30);
    }
    sal_uInt16 nResId = USHRT_MAX;
    xub_StrLen nFound = 0;
    String sTypeName = sName.GetToken(0, '.', nFound);
    if(COMPARE_EQUAL == sTypeName.CompareToAscii("User"))
        nResId = RES_USERFLD;
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("DDE"))
        nResId = RES_DDEFLD;
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("SetExpression"))
    {
        nResId = RES_SETEXPFLD;
        OUString sTypeName(sName.GetToken(1, '.'));
        OUString sUIName = SwXFieldMaster::GetSetExpUIName(sTypeName);
        if(sUIName != sTypeName)
            sName.SetToken(1, '.', sUIName);
    }
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("DataBase"))
    {
        if(!lcl_ConvertDatabaseName(sName))
            throw NoSuchElementException();
        nResId = RES_DBFLD;
    }
    else
        throw NoSuchElementException();

    sName.Erase(0, nFound);
    SwFieldType* pType = GetDoc()->GetFldType(nResId, sName);
    if(!pType)
        throw NoSuchElementException();
    SwXFieldMaster* pMaster = (SwXFieldMaster*)
                SwClientIter(*pType).First(TYPE(SwXFieldMaster));
    if(!pMaster)
        pMaster = new SwXFieldMaster(*pType, GetDoc());
    uno::Reference< XPropertySet >  aRef = pMaster;
    uno::Any aRet(&aRef, ::getCppuType((const uno::Reference<XPropertySet>*)0));
    return aRet;
}
/*-- 21.12.98 10:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextFieldMasters::getElementNames(void)
    throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!GetDoc())
        throw uno::RuntimeException();

    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    sal_uInt16 nCount = pFldTypes->Count();

    SvStrings aFldNames;
    String sPrefix(C2S("com.sun.star.text.FieldMaster."));
    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        sal_uInt16 nWhich = rFldType.Which();
        if(RES_USERFLD == nWhich)
        {
            String* pString = new String(sPrefix);
            *pString +=  C2S("User.");
            *pString += rFldType.GetName();
            aFldNames.Insert(pString, aFldNames.Count());
        }
        if(RES_DDEFLD == nWhich)
        {
            String* pString = new String(sPrefix);
            *pString += C2S("DDE.");
            *pString += rFldType.GetName();
            aFldNames.Insert(pString, aFldNames.Count());
        }
        if(RES_SETEXPFLD == nWhich)
        {
            String* pString = new String(sPrefix);
            *pString += C2S("SetExpression.");
            *pString += String(SwXFieldMaster::GetSetExpProgrammaticName(rFldType.GetName()));
            aFldNames.Insert(pString, aFldNames.Count());
        }
        if(RES_DBFLD == nWhich)
        {
            String* pString = new String(sPrefix);
            *pString += C2S("DataBase.");
            String sDBName(rFldType.GetName());
            sDBName.SearchAndReplaceAll(DB_DELIM, '.');
            *pString += sDBName;
            aFldNames.Insert(pString, aFldNames.Count());
        }
    }

    uno::Sequence< OUString > aSeq(aFldNames.Count());
    OUString* pArray = aSeq.getArray();
    for(i = 0; i < aFldNames.Count();i++)
    {
        pArray[i] = *aFldNames.GetObject(i);
    }
    aFldNames.DeleteAndDestroy(0, aFldNames.Count());

    return aSeq;

}
/*-- 21.12.98 10:37:33---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFieldMasters::hasByName(const OUString& rName) throw( RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!GetDoc())
        throw uno::RuntimeException();
    String sName(rName);
    if(sName.CompareToAscii("com.sun.star.text.FieldMaster.", 30) == COMPARE_EQUAL)
        sName.Erase(0, 30);

    sal_uInt16 nResId = USHRT_MAX;
    xub_StrLen nFound = 0;
    String sTypeName = sName.GetToken(0, '.', nFound);
    if(COMPARE_EQUAL == sTypeName.CompareToAscii("User"))
        nResId = RES_USERFLD;
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("DDE"))
        nResId = RES_DDEFLD;
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("SetExpression"))
    {
        nResId = RES_SETEXPFLD;
        OUString sTypeName(sName.GetToken(1, '.'));
        OUString sUIName = SwXFieldMaster::GetSetExpUIName(sTypeName);
        if(sUIName != sTypeName)
            sName.SetToken(1, '.', sUIName);
    }
    else if(COMPARE_EQUAL == sTypeName.CompareToAscii("DataBase"))
    {
        if(!lcl_ConvertDatabaseName(sName))
            throw NoSuchElementException();
        nResId = RES_DBFLD;
    }
    sName.Erase(0, nFound);
    sal_Bool bRet = nResId != USHRT_MAX && 0 != GetDoc()->GetFldType(nResId, sName);
    return bRet;
}
/*-- 21.12.98 10:37:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextFieldMasters::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<XPropertySet>*)0);

}
/*-- 21.12.98 10:37:34---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFieldMasters::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    return sal_True;
}

/******************************************************************
 *
 ******************************************************************/
/* -----------------------------06.04.00 13:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextFieldTypes::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFieldTypes");
}
/* -----------------------------06.04.00 13:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextFieldTypes::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextFieldTypes") == rServiceName;
}
/* -----------------------------06.04.00 13:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextFieldTypes::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextFieldTypes");
    return aRet;
}
/*-- 21.12.98 10:35:15---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFieldTypes::SwXTextFieldTypes(SwDoc* pDoc) :
    SwUnoCollection(pDoc)
{
}
/*-- 21.12.98 10:35:16---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFieldTypes::~SwXTextFieldTypes()
{
}
/*-- 21.12.98 10:35:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XEnumeration >  SwXTextFieldTypes::createEnumeration(void)
    throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    return new SwXFieldEnumeration(GetDoc());
}

/*-- 21.12.98 10:35:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextFieldTypes::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<XDependentTextField>*)0);
}
/*-- 21.12.98 10:35:17---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFieldTypes::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    //es gibt sie immer
    return sal_True;
}
/* -----------------24.02.99 16:19-------------------
 *
 * --------------------------------------------------*/
void SwXTextFieldTypes::refresh(void)  throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    UnoActionContext aContext(GetDoc());
    SwDocStat aDocStat;
    GetDoc()->UpdateDocStat(aDocStat);
    GetDoc()->UpdateFlds(0, sal_False);
}
/* -----------------24.02.99 16:19-------------------
 *
 * --------------------------------------------------*/
void SwXTextFieldTypes::addRefreshListener(const uno::Reference< util::XRefreshListener > & l)
    throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/* -----------------24.02.99 16:19-------------------
 *
 * --------------------------------------------------*/
void SwXTextFieldTypes::removeRefreshListener(const uno::Reference< util::XRefreshListener > & l)
     throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/******************************************************************
 * SwXFieldEnumeration
 ******************************************************************/
/* -----------------------------06.04.00 13:25--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFieldEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFieldEnumeration");
}
/* -----------------------------06.04.00 13:25--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFieldEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FieldEnumeration") == rServiceName;
}
/* -----------------------------06.04.00 13:25--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFieldEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FieldEnumeration");
    return aRet;
}
/* -----------------21.12.98 14:57-------------------
 *
 * --------------------------------------------------*/
SwXFieldEnumeration::SwXFieldEnumeration(SwDoc* pDc) :
    pDoc(pDc),
    pLastFieldType(0),
    pLastFieldFmt(0),
    pNextFieldFmt(0),
    pNextFieldType(0)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/*-- 21.12.98 14:57:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFieldEnumeration::~SwXFieldEnumeration()
{

}
/*-- 21.12.98 14:57:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXFieldEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    pNextFieldFmt = 0;
    if(pDoc)
    {
        const SwFldTypes* pFldTypes = pDoc->GetFldTypes();
        sal_uInt16 nCount = pFldTypes->Count();
        //wenn man noch nicht hier war, dann muss man nichts ueberspringen
        sal_Bool bFieldTypeFound = pLastFieldType == 0;
        sal_Bool bFieldFmtFound = pLastFieldFmt == 0;
        for(sal_uInt16 nType = 0; nType < nCount; nType++)
        {
            const SwFieldType* pCurType = pFldTypes->GetObject(nType);
            if(!bFieldTypeFound)
            {
                if(pCurType != pLastFieldType)
                    continue;
                bFieldTypeFound = sal_True;
            }
            //hier sind wir etwa da, wo wir das letzte Mal aufgehoert haben
            //jetzt noch das richtige Feld
            //pLastFieldFmt
            SwClientIter aIter( *(SwFieldType*)pCurType );
            SwFmtFld* pCurFldFmt = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
            sal_Int32 nRet = 0;
            while( pCurFldFmt && !pNextFieldFmt)
            {
                if(!bFieldFmtFound)
                {
                    if(pCurFldFmt == pLastFieldFmt)
                    {
                        bFieldFmtFound = sal_True;

                    }
                }
                else if(pCurFldFmt->GetTxtFld() && //keine TextAttr, dann schon geloescht
                    pCurFldFmt->GetTxtFld()->GetpTxtNode()->GetNodes().IsDocNodes())
                {
                    pNextFieldFmt = pCurFldFmt;
                    pNextFieldType = pCurType;
                }
                pCurFldFmt = (SwFmtFld*)aIter.Next();
            }
            if(pNextFieldFmt)
            {
                bRet = sal_True;
                break;
            }
        }
    }
    else
        throw uno::RuntimeException();
    return bRet;
}
/*-- 21.12.98 14:57:42---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFieldEnumeration::nextElement(void)
    throw( NoSuchElementException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(pDoc)
    {
        if(pNextFieldFmt || hasMoreElements())
        {
            const SwFldTypes* pFldTypes = pDoc->GetFldTypes();
            sal_uInt16 nCount = pFldTypes->Count();
            sal_Bool bTypeFound = sal_False;
            for(sal_uInt16 nType = 0; nType < nCount; nType++)
            {
                if(pNextFieldType == pFldTypes->GetObject(nType))
                {
                    bTypeFound = sal_True;
                    break;
                }
            }
            if(!bTypeFound)
                throw NoSuchElementException();
            SwClientIter aIter(*(SwFieldType*)pNextFieldType);
            SwFmtFld* pCurFldFmt = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
            while( pCurFldFmt)
            {
                if(pCurFldFmt == pNextFieldFmt)
                    break;
                pCurFldFmt = (SwFmtFld*)aIter.Next();
            }
            if(!pCurFldFmt)
                throw NoSuchElementException();
            //jetzt ist alles wiedergefunden, kann also benutzt werden
            pLastFieldFmt = pNextFieldFmt;
            pLastFieldType = pNextFieldType;
            pNextFieldFmt = 0;
            pNextFieldType = 0;
        }
        else
            throw NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    uno::Reference< XTextField >  xFld = new SwXTextField(*pLastFieldFmt, pDoc);
    uno::Any aRet(&xFld, ::getCppuType((const uno::Reference<XTextField>*)0));
    return aRet;

}
/* -----------------21.12.98 15:08-------------------
 *
 * --------------------------------------------------*/
void SwXFieldEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDoc = 0;
}

