/*************************************************************************
 *
 *  $RCSfile: formmetadata.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:47:47 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_FORMHELPID_HRC_
#include "formhelpid.hrc"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;

    //========================================================================
    //= OPropertyInfoImpl
    //========================================================================
    struct OPropertyInfoImpl
    {
        String          sName;
        String          sTranslation;
        sal_uInt32      nHelpId;
        sal_Int32       nId;
        sal_uInt16      nPos;
        sal_uInt32      nUIFlags;

        OPropertyInfoImpl(
                        const ::rtl::OUString&      rName,
                        sal_Int32                   _nId,
                        const String&               aTranslation,
                        sal_uInt16                  nPosId,
                        sal_uInt32                  nHelpId,
                        sal_uInt32                  _nUIFlags);
    };

    //------------------------------------------------------------------------
    OPropertyInfoImpl::OPropertyInfoImpl(const ::rtl::OUString& _rName, sal_Int32 _nId,
                                   const String& aString, sal_uInt16 nP, sal_uInt32 nHid, sal_uInt32 _nUIFlags)
       :sName(_rName)
       ,sTranslation(aString)
       ,nHelpId(nHid)
       ,nId(_nId)
       ,nPos(nP)
       ,nUIFlags(_nUIFlags)
    {
    }

    //------------------------------------------------------------------------
    // Vergleichen von PropertyInfo
    static int
    #if defined( WNT )
     __cdecl
    #endif
    #if defined( ICC ) && defined( OS2 )
    _Optlink
    #endif
        PropertyInfoCompare(const void* pFirst, const void* pSecond)
    {
        return reinterpret_cast<const OPropertyInfoImpl*>(pFirst)->sName.CompareTo(reinterpret_cast<const OPropertyInfoImpl*>(pSecond)->sName);
    }

    //========================================================================
    //= OFormPropertyInfoService
    //========================================================================
#define DEF_INFO( ident, uinameres, helpid, flags )   \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
            String( ModuleRes( RID_STR_##uinameres ) ), nPos++, HID_PROP_##helpid, flags )

#define DEF_INFO_1( ident, uinameres, helpid, flag1 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 )

#define DEF_INFO_2( ident, uinameres, helpid, flag1, flag2 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 )

#define DEF_INFO_3( ident, uinameres, helpid, flag1, flag2, flag3 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 )

#define DEF_INFO_4( ident, uinameres, helpid, flag1, flag2, flag3, flag4 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 | PROP_FLAG_##flag3 | PROP_FLAG_##flag4 )

    sal_uInt16              OFormPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OFormPropertyInfoService::s_pPropertyInfos = NULL;
    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OFormPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        OModuleResourceClient aResourceAccess;
        // this ensures that we have our resource file loaded

        sal_uInt16 nPos = 1;

        static OPropertyInfoImpl __READONLY_DATA aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resoure id,         help id,           flags ),
        */
        DEF_INFO_2( NAME,              NAME,               NAME,              FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( TITLE,             TITLE,              TITLE,             FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( LABEL,             LABEL,              LABEL,             FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( CONTROLLABEL,      LABELCONTROL,       CONTROLLABEL,      FORM_VISIBLE ),
        DEF_INFO_1( TEXT,              TEXT,               TEXT,              DIALOG_VISIBLE ),
        DEF_INFO_2( MAXTEXTLEN,        MAXTEXTLEN,         MAXTEXTLEN,        FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( EDITMASK,          EDITMASK,           EDITMASK,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( LITERALMASK,       LITERALMASK,        LITERALMASK,       FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( STRICTFORMAT,      STRICTFORMAT,       STRICTFORMAT,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( ENABLED,           ENABLED,            ENABLED,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( READONLY,          READONLY,           READONLY,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( PRINTABLE,         PRINTABLE,          PRINTABLE,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( STEP,              STEP,               STEP,              FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( TABSTOP,           TABSTOP,            TABSTOP,           FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_2( TABINDEX,          TABINDEX,           TABINDEX,          FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_4( BOUND_CELL,        BOUND_CELL,         BOUND_CELL,        FORM_VISIBLE, DATA_PROPERTY, VIRTUAL_PROP, ACTUATING ),
        DEF_INFO_4( CELL_EXCHANGE_TYPE,CELL_EXCHANGE_TYPE, CELL_EXCHANGE_TYPE,FORM_VISIBLE, DATA_PROPERTY, VIRTUAL_PROP, ENUM ),
        DEF_INFO_4( LIST_CELL_RANGE,   LIST_CELL_RANGE,    LIST_CELL_RANGE,   FORM_VISIBLE, DATA_PROPERTY, VIRTUAL_PROP, ACTUATING ),
        DEF_INFO_3( CONTROLSOURCE,     CONTROLSOURCE,      CONTROLSOURCE,     FORM_VISIBLE, DATA_PROPERTY, ACTUATING ),
        DEF_INFO_3( DATASOURCE,        DATASOURCE,         DATASOURCE,        FORM_VISIBLE, DATA_PROPERTY, ACTUATING ),
        DEF_INFO_4( COMMANDTYPE,       CURSORSOURCETYPE,   CURSORSOURCETYPE,  FORM_VISIBLE, DATA_PROPERTY, ENUM, ACTUATING ),
        DEF_INFO_3( COMMAND,           CURSORSOURCE,       CURSORSOURCE,      FORM_VISIBLE, DATA_PROPERTY, ACTUATING ),
        DEF_INFO_3( ESCAPE_PROCESSING, ESCAPE_PROCESSING,  ESCAPE_PROCESSING, FORM_VISIBLE, DATA_PROPERTY, ACTUATING ),
        DEF_INFO_2( FILTER,            FILTER,             FILTER,            FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( SORT,              SORT_CRITERIA,      SORT_CRITERIA,     FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( MASTERFIELDS,      MASTERFIELDS,       MASTERFIELDS,      FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( DETAILFIELDS,      SLAVEFIELDS,        SLAVEFIELDS,       FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( ALLOWADDITIONS,    ALLOW_ADDITIONS,    ALLOW_ADDITIONS,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( ALLOWEDITS,        ALLOW_EDITS,        ALLOW_EDITS,       FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( ALLOWDELETIONS,    ALLOW_DELETIONS,    ALLOW_DELETIONS,   FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( INSERTONLY,        DATAENTRY,          DATAENTRY,         FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_3( NAVIGATION,        NAVIGATION,         NAVIGATION,        FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_3( CYCLE,             CYCLE,              CYCLE,             FORM_VISIBLE, DATA_PROPERTY, ENUM ),
        DEF_INFO_2( EMPTY_IS_NULL,     EMPTY_IS_NULL,      EMPTY_IS_NULL,     FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_2( FILTERPROPOSAL,    FILTERPROPOSAL,     FILTERPROPOSAL,    FORM_VISIBLE, DATA_PROPERTY ),
        DEF_INFO_4( LISTSOURCETYPE,    LISTSOURCETYPE,     LISTSOURCETYPE,    FORM_VISIBLE, DATA_PROPERTY, ACTUATING, ENUM ),
        DEF_INFO_3( LISTSOURCE,        LISTSOURCE,         LISTSOURCE,        FORM_VISIBLE, DATA_PROPERTY, ACTUATING ),
        DEF_INFO_2( BOUNDCOLUMN,       BOUNDCOLUMN,        BOUNDCOLUMN,       FORM_VISIBLE, DATA_PROPERTY ),

        DEF_INFO_1( HIDDEN_VALUE,      VALUE,              HIDDEN_VALUE,      FORM_VISIBLE ),
        DEF_INFO_1( VALUE,             VALUE,              VALUE,             DIALOG_VISIBLE ),
        DEF_INFO_2( VALUEMIN,          VALUEMIN,           VALUEMIN,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( VALUEMAX,          VALUEMAX,           VALUEMAX,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( VALUESTEP,         VALUESTEP,          VALUESTEP,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( DEFAULT_VALUE,     DEFAULTVALUE,       DEFAULT_LONG_VALUE,FORM_VISIBLE ),
        DEF_INFO_2( DECIMAL_ACCURACY,  DECIMAL_ACCURACY,   DECIMAL_ACCURACY,  FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( SHOWTHOUSANDSEP,   SHOWTHOUSANDSEP,    SHOWTHOUSANDSEP,   FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_1( REFVALUE,          REFVALUE,           REFVALUE,          FORM_VISIBLE ),
        DEF_INFO_2( CURRENCYSYMBOL,    CURRENCYSYMBOL,     CURRENCYSYMBOL,    FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( CURRSYM_POSITION,  CURRSYM_POSITION,   CURRSYM_POSITION,  FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_1( DATE,              DATE,               DATE,              DIALOG_VISIBLE ),
        DEF_INFO_2( DATEMIN,           DATEMIN,            DATEMIN,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( DATEMAX,           DATEMAX,            DATEMAX,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( DATEFORMAT,        DATEFORMAT,         DATEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM ),
        DEF_INFO_1( DEFAULT_DATE,      DEFAULTDATE,        DEFAULT_DATE,      FORM_VISIBLE ),

        DEF_INFO_1( TIME,              TIME,               TIME,              DIALOG_VISIBLE ),
        DEF_INFO_2( TIMEMIN,           TIMEMIN,            TIMEMIN,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( TIMEMAX,           TIMEMAX,            TIMEMAX,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( TIMEFORMAT,        TIMEFORMAT,         TIMEFORMAT,        FORM_VISIBLE, DIALOG_VISIBLE, ENUM ),
        DEF_INFO_1( DEFAULT_TIME,      DEFAULTTIME,        DEFAULT_TIME,      FORM_VISIBLE ),

        DEF_INFO_1( EFFECTIVE_VALUE,   VALUE,              VALUE,             DIALOG_VISIBLE ),
        DEF_INFO_2( EFFECTIVE_MIN,     VALUEMIN,           EFFECTIVEMIN,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( EFFECTIVE_MAX,     VALUEMAX,           EFFECTIVEMAX,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( EFFECTIVE_DEFAULT, DEFAULTVALUE,       EFFECTIVEDEFAULT,  FORM_VISIBLE ),
        DEF_INFO_2( FORMATKEY,         FORMATKEY,          FORMATKEY,         FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_2( PROGRESSVALUE,     PROGRESSVALUE,      PROGRESSVALUE,     FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( PROGRESSVALUE_MIN, PROGRESSVALUE_MIN,  PROGRESSVALUE_MIN, FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( PROGRESSVALUE_MAX, PROGRESSVALUE_MAX,  PROGRESSVALUE_MAX, FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_1( SCROLLVALUE,       SCROLLVALUE,        SCROLLVALUE,       DIALOG_VISIBLE ),
        DEF_INFO_2( SCROLLVALUE_MIN,   SCROLLVALUE_MIN,    SCROLLVALUE_MIN,   FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( SCROLLVALUE_MAX,   SCROLLVALUE_MAX,    SCROLLVALUE_MAX,   FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( DEFAULT_SCROLLVALUE,DEFAULT_SCROLLVALUE,DEFAULT_SCROLLVALUE,FORM_VISIBLE ),
        DEF_INFO_2( LINEINCREMENT,     LINEINCREMENT,      LINEINCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( BLOCKINCREMENT,    BLOCKINCREMENT,     BLOCKINCREMENT,    FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_1( SPINVALUE,        VALUE,               SPINVALUE,         DIALOG_VISIBLE ),
        DEF_INFO_2( SPINVALUE_MIN,    VALUEMIN,            SPINVALUE_MIN,     FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( SPINVALUE_MAX,    VALUEMAX,            SPINVALUE_MAX,     FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( DEFAULT_SPINVALUE,DEFAULTVALUE,        DEFAULT_SPINVALUE, FORM_VISIBLE ),
        DEF_INFO_2( SPININCREMENT,    VALUESTEP,           SPININCREMENT,     FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_2( SPIN,              SPIN,               SPIN,              FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( REPEAT,            REPEAT,             REPEAT,            FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_2( REPEAT_DELAY,      REPEAT_DELAY,       REPEAT_DELAY,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( TOGGLE,            TOGGLE,             TOGGLE,            FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( FOCUSONCLICK,      FOCUSONCLICK,       FOCUSONCLICK,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( VISIBLESIZE,       VISIBLESIZE,        VISIBLESIZE,       FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( ORIENTATION,       ORIENTATION,        ORIENTATION,       FORM_VISIBLE, DIALOG_VISIBLE, ENUM ),

        DEF_INFO_1( CLASSID,           CLASSID,            CLASSID,           FORM_VISIBLE ),
        DEF_INFO_2( HEIGHT,            HEIGHT,             HEIGHT,            FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( WIDTH,             WIDTH,              WIDTH,             FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( POSITIONX,         POSITIONX,          POSITIONX,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( POSITIONY,         POSITIONY,          POSITIONY,         FORM_VISIBLE, DIALOG_VISIBLE ),

        DEF_INFO_1( LISTINDEX,         LISTINDEX,          LISTINDEX,         FORM_VISIBLE ),
        DEF_INFO_3( STRINGITEMLIST,    STRINGITEMLIST,     STRINGITEMLIST,    FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_1( DEFAULT_TEXT,      DEFAULTTEXT,        DEFAULTVALUE,      FORM_VISIBLE ),
        DEF_INFO_2( FONT_NAME,         FONT,               FONT,              FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( VISUALEFFECT,      VISUALEFFECT,       VISUALEFFECT,      FORM_VISIBLE, DIALOG_VISIBLE, ENUM_ONE ),
        DEF_INFO_3( ALIGN,             ALIGN,              ALIGN,             FORM_VISIBLE, DIALOG_VISIBLE, ENUM ),
        DEF_INFO_1( ROWHEIGHT,         ROWHEIGHT,          ROWHEIGHT,         FORM_VISIBLE ),
        DEF_INFO_2( BACKGROUNDCOLOR,   BACKGROUNDCOLOR,    BACKGROUNDCOLOR,   FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( SYMBOLCOLOR,       SYMBOLCOLOR,        SYMBOLCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( FILLCOLOR,         FILLCOLOR,          FILLCOLOR,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( LINECOLOR,         LINECOLOR,          LINECOLOR,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_4( BORDER,            BORDER,             BORDER,            FORM_VISIBLE, DIALOG_VISIBLE, ENUM, ACTUATING ),
        DEF_INFO_2( BORDERCOLOR,       BORDERCOLOR,        BORDERCOLOR,       FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( ICONSIZE,          ICONSIZE,           ICONSIZE,          FORM_VISIBLE, ENUM ),
        DEF_INFO_1( SHOW_POSITION,     SHOW_POSITION,      SHOW_POSITION,     FORM_VISIBLE ),
        DEF_INFO_1( SHOW_NAVIGATION,   SHOW_NAVIGATION,    SHOW_NAVIGATION,   FORM_VISIBLE ),
        DEF_INFO_1( SHOW_RECORDACTIONS,SHOW_RECORDACTIONS, SHOW_RECORDACTIONS,FORM_VISIBLE ),
        DEF_INFO_1( SHOW_FILTERSORT,   SHOW_FILTERSORT,    SHOW_FILTERSORT,   FORM_VISIBLE ),

        DEF_INFO_3( DROPDOWN,          DROPDOWN,           DROPDOWN,          FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_2( LINECOUNT,         LINECOUNT,          LINECOUNT,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( AUTOCOMPLETE,      AUTOCOMPLETE,       AUTOCOMPLETE,      FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( MULTILINE,         MULTILINE,          MULTILINE,         FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_2( LINEEND_FORMAT,    LINEEND_FORMAT,     LINEEND_FORMAT,    FORM_VISIBLE, ENUM_ONE ),
        DEF_INFO_2( WORDBREAK,         WORDBREAK,          WORDBREAK,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_4( TEXTTYPE,          TEXTTYPE,           TEXTTYPE,          FORM_VISIBLE, VIRTUAL_PROP, ENUM, ACTUATING ),
        DEF_INFO_2( MULTISELECTION,    MULTISELECTION,     MULTISELECTION,    FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_4( SHOW_SCROLLBARS,   SHOW_SCROLLBARS,    SHOW_SCROLLBARS,   FORM_VISIBLE, DIALOG_VISIBLE, VIRTUAL_PROP, ENUM ),
        DEF_INFO_2( HSCROLL,           HSCROLL,            HSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( VSCROLL,           VSCROLL,            VSCROLL,           FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_4( BUTTONTYPE,        BUTTONTYPE,         BUTTONTYPE,        FORM_VISIBLE, ACTUATING, ENUM, VIRTUAL_PROP ),
        DEF_INFO_2( PUSHBUTTONTYPE,    PUSHBUTTONTYPE,     PUSHBUTTONTYPE,    DIALOG_VISIBLE, ENUM ),
        DEF_INFO_3( TARGET_URL,        TARGET_URL,         TARGET_URL,        FORM_VISIBLE, ACTUATING, VIRTUAL_PROP ),
        DEF_INFO_1( TARGET_FRAME,      TARGET_FRAME,       TARGET_FRAME,      FORM_VISIBLE ),
        DEF_INFO_1( SUBMIT_ACTION,     SUBMIT_ACTION,      SUBMIT_ACTION,     FORM_VISIBLE ),
        DEF_INFO_1( SUBMIT_TARGET,     SUBMIT_TARGET,      SUBMIT_TARGET,     FORM_VISIBLE ),
        DEF_INFO_3( SUBMIT_ENCODING,   SUBMIT_ENCODING,    SUBMIT_ENCODING,   FORM_VISIBLE, ACTUATING, ENUM ),
        DEF_INFO_2( SUBMIT_METHOD,     SUBMIT_METHOD,      SUBMIT_METHOD,     FORM_VISIBLE, ENUM ),
        DEF_INFO_2( STATE,             STATE,              STATE,             DIALOG_VISIBLE, ENUM ),
        DEF_INFO_2( DEFAULTCHECKED,    DEFAULT_CHECKED,    DEFAULT_CHECKED,   FORM_VISIBLE, ENUM ),
        DEF_INFO_2( DEFAULTBUTTON,     DEFAULT_BUTTON,     DEFAULT_BUTTON,    FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_3( IMAGE_URL,         IMAGE_URL,          IMAGE_URL,         FORM_VISIBLE, DIALOG_VISIBLE, ACTUATING ),
        DEF_INFO_3( IMAGEPOSITION,     IMAGEPOSITION,      IMAGEPOSITION,     FORM_VISIBLE, DIALOG_VISIBLE, ENUM ),
        DEF_INFO_2( SCALEIMAGE,        SCALEIMAGE,         SCALEIMAGE,        FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( DEFAULT_SELECT_SEQ,DEFAULT_SELECT_SEQ, DEFAULT_SELECT_SEQ,FORM_VISIBLE ),
        DEF_INFO_1( SELECTEDITEMS,     SELECTEDITEMS,      SELECTEDITEMS,     DIALOG_VISIBLE ),
        DEF_INFO_2( ECHO_CHAR,         ECHO_CHAR,          ECHO_CHAR,         FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( HIDEINACTIVESELECTION, HIDEINACTIVESELECTION, HIDEINACTIVESELECTION, FORM_VISIBLE, DIALOG_VISIBLE  ),
        DEF_INFO_2( TRISTATE,          TRISTATE,           TRISTATE,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_1( HASNAVIGATION,     NAVIGATION,         NAVIGATIONBAR,     FORM_VISIBLE ),
        DEF_INFO_1( RECORDMARKER,      RECORDMARKER,       RECORDMARKER,      FORM_VISIBLE ),
        DEF_INFO_2( TAG,               TAG,                TAG,               FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( HELPTEXT,          HELPTEXT,           HELPTEXT,          FORM_VISIBLE, DIALOG_VISIBLE ),
        DEF_INFO_2( HELPURL,           HELPURL,            HELPURL,           FORM_VISIBLE, DIALOG_VISIBLE )
        };

        s_pPropertyInfos = const_cast<OPropertyInfoImpl*>(aPropertyInfos);
        s_nCount = sizeof(aPropertyInfos) / sizeof(OPropertyInfoImpl);

        // sort
        qsort((void*) aPropertyInfos,
                s_nCount,
                sizeof(OPropertyInfoImpl),
                &PropertyInfoCompare);

        return s_pPropertyInfos;
    }

    //------------------------------------------------------------------------
    sal_Int32 OFormPropertyInfoService::getPropertyId(const String& _rName) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_rName);
        return pInfo ? pInfo->nId : -1;
    }

    //------------------------------------------------------------------------
    String OFormPropertyInfoService::getPropertyTranslation(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sTranslation : String();
    }

    //------------------------------------------------------------------------
    sal_Int32 OFormPropertyInfoService::getPropertyHelpId(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nHelpId : 0;
    }

    //------------------------------------------------------------------------
    sal_Int16 OFormPropertyInfoService::getPropertyPos(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nPos : 0xFFFF;
    }

    //------------------------------------------------------------------------
    sal_uInt32 OFormPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nUIFlags : 0;
    }

    //------------------------------------------------------------------------
    ::std::vector< String > OFormPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId) const
    {
        OSL_ENSURE( ( ( getPropertyUIFlags( _nId ) & PROP_FLAG_ENUM ) != 0 ) || ( _nId == PROPERTY_ID_TARGET_FRAME ),
            "OFormPropertyInfoService::getPropertyEnumRepresentations: this is no enum property!" );

        sal_Int16 nCommaSeparatedListResId = 0;
        sal_Int16 nStringItemsResId = 0;
        switch ( _nId )
        {
            case PROPERTY_ID_IMAGEPOSITION:
                nStringItemsResId = RID_RSC_ENUM_IMAGE_POSITION;
                break;
            case PROPERTY_ID_BORDER:
                nCommaSeparatedListResId = RID_STR_BORDER_TYPE;
                break;
            case PROPERTY_ID_ICONSIZE:
                nCommaSeparatedListResId = RID_STR_ICONSIZE_TYPE;
                break;
            case PROPERTY_ID_COMMANDTYPE:
                nStringItemsResId = RID_RSC_ENUM_COMMAND_TYPE;
                break;
            case PROPERTY_ID_LISTSOURCETYPE:
                nCommaSeparatedListResId = RID_STR_LISTSOURCE_TYPE;
                break;
            case PROPERTY_ID_ALIGN:
                nCommaSeparatedListResId = RID_STR_ALIGNMENT;
                break;
            case PROPERTY_ID_BUTTONTYPE:
                nCommaSeparatedListResId = RID_STR_ENUM_BUTTONTYPE;
                break;
            case PROPERTY_ID_PUSHBUTTONTYPE:
                nCommaSeparatedListResId = RID_STR_ENUM_PUSHBUTTONTYPE;
                break;
            case PROPERTY_ID_SUBMIT_METHOD:
                nCommaSeparatedListResId = RID_STR_ENUM_SUBMIT_METHOD;
                break;
            case PROPERTY_ID_SUBMIT_ENCODING:
                nCommaSeparatedListResId = RID_STR_ENUM_SUBMIT_ENCODING;
                break;
            case PROPERTY_ID_DATEFORMAT:
                nCommaSeparatedListResId = RID_STR_DATEFORMAT_LIST;
                break;
            case PROPERTY_ID_TIMEFORMAT:
                nCommaSeparatedListResId = RID_STR_TIMEFORMAT_LIST;
                break;
            case PROPERTY_ID_DEFAULTCHECKED:
            case PROPERTY_ID_STATE:
                nCommaSeparatedListResId = RID_STR_ENUM_CHECKED;
                break;
            case PROPERTY_ID_CYCLE:
                nCommaSeparatedListResId = RID_STR_ENUM_CYCLE;
                break;
            case PROPERTY_ID_NAVIGATION:
                nCommaSeparatedListResId = RID_STR_ENUM_NAVIGATION;
                break;
            case PROPERTY_ID_TARGET_FRAME:
                nCommaSeparatedListResId = RID_STR_ENUM_SUBMIT_TARGET;
                break;
            case PROPERTY_ID_ORIENTATION:
                nCommaSeparatedListResId = RID_STR_ENUM_ORIENTATION;
                break;
            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                nCommaSeparatedListResId = RID_STR_ENUM_CELL_EXCHANGE_TYPE;
                break;
            case PROPERTY_ID_SHOW_SCROLLBARS:
                nStringItemsResId = RID_RSC_ENUM_SCROLLBARS;
                break;
            case PROPERTY_ID_VISUALEFFECT:
                nStringItemsResId = RID_RSC_ENUM_VISUALEFFECT;
                break;
            case PROPERTY_ID_TEXTTYPE:
                nStringItemsResId = RID_RSC_ENUM_TEXTTYPE;
                break;
            case PROPERTY_ID_LINEEND_FORMAT:
                nStringItemsResId = RID_RSC_ENUM_LINEEND_FORMAT;
                break;
            default:
                OSL_ENSURE( sal_False, "OFormPropertyInfoService::getPropertyEnumRepresentations: unknown enum property!" );
        }

        ::std::vector< String > aReturn;

        if ( nCommaSeparatedListResId )
        {
            String sSeparatedList = String( ModuleRes( nCommaSeparatedListResId ) );
            xub_StrLen nTokens = sSeparatedList.GetTokenCount(';');
            aReturn.reserve( nTokens );
            for ( xub_StrLen i = 0; i < nTokens; ++i )
                aReturn.push_back( sSeparatedList.GetToken( i ) );
        }
        else if ( nStringItemsResId )
        {
            ModuleRes aResId( nStringItemsResId );
            ::svt::OLocalResourceAccess aEnumStrings( aResId, RSC_RESOURCE );

            sal_Int16 i = 1;
            ResId aLocalId( i );
            while ( aEnumStrings.IsAvailableRes( aLocalId.SetRT( RSC_STRING ) ) )
            {
                aReturn.push_back( String( aLocalId ) );
                aLocalId = ResId( ++i );
            }
        }

        return aReturn;
    }

    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OFormPropertyInfoService::getPropertyInfo(const String& _rName)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0L, String(), 0, 0, 0);

        const OPropertyInfoImpl* pPropInfo = (OPropertyInfoImpl*) bsearch(&aSearch,
                                        static_cast<void*>(s_pPropertyInfos),
                                         s_nCount,
                                         sizeof(OPropertyInfoImpl),
                                         &PropertyInfoCompare);

        return pPropInfo;
    }


    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OFormPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();

        // TODO: a real structure which allows quick access by name as well as by id
        for (sal_uInt16 i = 0; i < s_nCount; i++)
            if (s_pPropertyInfos[i].nId == _nId)
                return &s_pPropertyInfos[i];

        return NULL;
    }

    //========================================================================
    //= event meta data handling
    //========================================================================
    //------------------------------------------------------------------------
    // Vergleichen von PropertyInfo
    static int
    #if defined( WNT )
     __cdecl
    #endif
    #if defined( ICC ) && defined( OS2 )
    _Optlink
    #endif
        EventDisplayDescriptionCompareByName(const void* pFirst, const void* pSecond)
    {
        return
            static_cast<const EventDisplayDescription*>(pFirst)->sName.compareTo(
                static_cast<const EventDisplayDescription*>(pSecond)->sName);
    }

#define EVT_DESC( asciiname, id_postfix ) \
    EventDisplayDescription( nCount++, asciiname, RID_STR_EVT_##id_postfix, HID_EVT_##id_postfix, UID_BRWEVT_##id_postfix )

    //------------------------------------------------------------------------
    EventDisplayDescription* GetEvtTranslation(const ::rtl::OUString& rName)
    {
        static EventDisplayDescription* s_pEventTranslation = NULL;
        static sal_Int32 nCount = 0;
        if (!s_pEventTranslation)
        {
            static EventDisplayDescription __READONLY_DATA aEventDisplayDescriptions[] =
            {
                EVT_DESC( "approveAction",     APPROVEACTIONPERFORMED ),
                EVT_DESC( "actionPerformed",   ACTIONPERFORMED ),
                EVT_DESC( "changed",           CHANGED ),
                EVT_DESC( "textChanged",       TEXTCHANGED ),
                EVT_DESC( "itemStateChanged",  ITEMSTATECHANGED ),
                EVT_DESC( "focusGained",       FOCUSGAINED ),
                EVT_DESC( "focusLost",         FOCUSLOST ),
                EVT_DESC( "keyPressed",        KEYTYPED ),
                EVT_DESC( "keyReleased",       KEYUP ),
                EVT_DESC( "mouseEntered",      MOUSEENTERED ),
                EVT_DESC( "mouseDragged",      MOUSEDRAGGED ),
                EVT_DESC( "mouseMoved",        MOUSEMOVED ),
                EVT_DESC( "mousePressed",      MOUSEPRESSED ),
                EVT_DESC( "mouseReleased",     MOUSERELEASED ),
                EVT_DESC( "mouseExited",       MOUSEEXITED ),
                EVT_DESC( "approveReset",      APPROVERESETTED ),
                EVT_DESC( "resetted",          RESETTED ),
                EVT_DESC( "approveSubmit",     SUBMITTED ),
                EVT_DESC( "approveUpdate",     BEFOREUPDATE ),
                EVT_DESC( "updated",           AFTERUPDATE ),
                EVT_DESC( "loaded",            LOADED ),
                EVT_DESC( "reloading",         RELOADING ),
                EVT_DESC( "reloaded",          RELOADED ),
                EVT_DESC( "unloading",         UNLOADING ),
                EVT_DESC( "unloaded",          UNLOADED ),
                EVT_DESC( "confirmDelete",     CONFIRMDELETE ),
                EVT_DESC( "approveRowChange",  APPROVEROWCHANGE ),
                EVT_DESC( "rowChanged",        ROWCHANGE ),
                EVT_DESC( "approveCursorMove", POSITIONING ),
                EVT_DESC( "cursorMoved",       POSITIONED ),
                EVT_DESC( "approveParameter",  APPROVEPARAMETER ),
                EVT_DESC( "errorOccured",      ERROROCCURED ),
                EVT_DESC( "adjustmentValueChanged", ADJUSTMENTVALUECHANGED ),
            };
            s_pEventTranslation = const_cast<EventDisplayDescription*>(aEventDisplayDescriptions);
            nCount = sizeof(aEventDisplayDescriptions) / sizeof(EventDisplayDescription);

            qsort(static_cast<void*>(s_pEventTranslation),
                 nCount,
                 sizeof(EventDisplayDescription),
                 &EventDisplayDescriptionCompareByName);
        }

        EventDisplayDescription aSearch( rName );
        return static_cast<EventDisplayDescription*>(bsearch(
            &aSearch,
            static_cast<void*>(s_pEventTranslation),
            nCount,
            sizeof(EventDisplayDescription),
            &EventDisplayDescriptionCompareByName
        ));
    }

//............................................................................
} // namespace pcr
//............................................................................

