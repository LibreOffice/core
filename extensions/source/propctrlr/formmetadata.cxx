/*************************************************************************
 *
 *  $RCSfile: formmetadata.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-06 12:36:31 $
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
       ,nId(_nId)
       ,sTranslation(aString)
       ,nPos(nP)
       ,nHelpId(nHid)
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
                        String( ModuleRes( uinameres ) ), nPos++, helpid, flags )

    sal_uInt16              OFormPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OFormPropertyInfoService::s_pPropertyInfos = NULL;
    //------------------------------------------------------------------------
    const OPropertyInfoImpl* OFormPropertyInfoService::getPropertyInfo()
    {
        sal_uInt16 nPos=1;
        if (s_pPropertyInfos == NULL)
        {
            OModuleResourceClient aResourceAccess;
            // this ensures that we have our resource file loaded

            // somewhat ugly ... but this way we easily ensure that the
            static OPropertyInfoImpl __READONLY_DATA aPropertyInfos[] =
            {
                DEF_INFO( NAME,             RID_STR_NAME,               HID_PROP_NAME,                      PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TITLE,            RID_STR_TITLE,              HID_PROP_TITLE,                     PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( LABEL,            RID_STR_LABEL,              HID_PROP_LABEL,                     PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( CONTROLLABEL,     RID_STR_LABELCONTROL,       HID_PROP_CONTROLLABEL,              PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( TEXT,             RID_STR_TEXT,               HID_PROP_TEXT,                                             PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( MAXTEXTLEN,       RID_STR_MAXTEXTLEN,         HID_PROP_MAXTEXTLEN,                PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( EDITMASK,         RID_STR_EDITMASK,           HID_PROP_EDITMASK,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( LITERALMASK,      RID_STR_LITERALMASK,        HID_PROP_LITERALMASK,               PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( STRICTFORMAT,     RID_STR_STRICTFORMAT,       HID_PROP_STRICTFORMAT,              PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( ENABLED,          RID_STR_ENABLED,            HID_PROP_ENABLED,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( READONLY,         RID_STR_READONLY,           HID_PROP_READONLY,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( PRINTABLE,        RID_STR_PRINTABLE,          HID_PROP_PRINTABLE,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( STEP,             RID_STR_STEP,               HID_PROP_STEP,                      PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TABSTOP,          RID_STR_TABSTOP,            HID_PROP_TABSTOP,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TABINDEX,         RID_STR_TABINDEX,           HID_PROP_TABINDEX,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( BOUND_CELL,       RID_STR_BOUND_CELL,         HID_PROP_BOUND_CELL,                PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY|PROP_FLAG_VIRTUAL_PROP),
                DEF_INFO( CELL_EXCHANGE_TYPE,RID_STR_CELL_EXCHANGE_TYPE,HID_PROP_CELL_EXCHANGE_TYPE,        PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY|PROP_FLAG_VIRTUAL_PROP),
                DEF_INFO( LIST_CELL_RANGE,  RID_STR_LIST_CELL_RANGE,    HID_PROP_LIST_CELL_RANGE,           PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY|PROP_FLAG_VIRTUAL_PROP),
                DEF_INFO( CONTROLSOURCE,    RID_STR_CONTROLSOURCE,      HID_PROP_CONTROLSOURCE,             PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( DATASOURCE,       RID_STR_DATASOURCE,         HID_PROP_DATASOURCE,                PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( COMMANDTYPE,      RID_STR_CURSORSOURCETYPE,   HID_PROP_CURSORSOURCETYPE,          PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( COMMAND,          RID_STR_CURSORSOURCE,       HID_PROP_CURSORSOURCE,              PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( ESCAPE_PROCESSING,RID_STR_ESCAPE_PROCESSING,  HID_PROP_ESCAPE_PROCESSING,         PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( FILTER_CRITERIA,  RID_STR_FILTER_CRITERIA,    HID_PROP_FILTER_CRITERIA,           PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( SORT,             RID_STR_SORT_CRITERIA,      HID_PROP_SORT_CRITERIA,             PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( ALLOWADDITIONS,   RID_STR_ALLOW_ADDITIONS,    HID_PROP_ALLOW_ADDITIONS,           PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( ALLOWEDITS,       RID_STR_ALLOW_EDITS,        HID_PROP_ALLOW_EDITS,               PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( ALLOWDELETIONS,   RID_STR_ALLOW_DELETIONS,    HID_PROP_ALLOW_DELETIONS,           PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( INSERTONLY,       RID_STR_DATAENTRY,          HID_PROP_DATAENTRY,                 PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( NAVIGATION,       RID_STR_NAVIGATION,         HID_PROP_NAVIGATION,                PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( CYCLE,            RID_STR_CYCLE,              HID_PROP_CYCLE,                     PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( EMPTY_IS_NULL,    RID_STR_EMPTY_IS_NULL,      HID_PROP_EMPTY_IS_NULL,             PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( FILTERPROPOSAL,   RID_STR_FILTERPROPOSAL,     HID_PROP_FILTERPROPOSAL,            PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( BOUNDCOLUMN,      RID_STR_BOUNDCOLUMN,        HID_PROP_BOUNDCOLUMN,               PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( LISTSOURCETYPE,   RID_STR_LISTSOURCETYPE,     HID_PROP_LISTSOURCETYPE,            PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( LISTSOURCE,       RID_STR_LISTSOURCE,         HID_PROP_LISTSOURCE,                PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( MASTERFIELDS,     RID_STR_MASTERFIELDS,       HID_PROP_MASTERFIELDS,              PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),
                DEF_INFO( DETAILFIELDS,     RID_STR_SLAVEFIELDS,        HID_PROP_SLAVEFIELDS,               PROP_FLAG_FORM_VISIBLE                         |PROP_FLAG_DATA_PROPERTY),

                DEF_INFO( HIDDEN_VALUE,     RID_STR_VALUE,              HID_PROP_HIDDEN_VALUE,              PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( VALUE,            RID_STR_VALUE,              HID_PROP_VALUE,                                            PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( VALUEMIN,         RID_STR_VALUEMIN,           HID_PROP_VALUEMIN,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( VALUEMAX,         RID_STR_VALUEMAX,           HID_PROP_VALUEMAX,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( VALUESTEP,        RID_STR_VALUESTEP,          HID_PROP_VALUESTEP,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULT_VALUE,    RID_STR_DEFAULTVALUE,       HID_PROP_DEFAULT_LONG_VALUE,        PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( DECIMAL_ACCURACY, RID_STR_DECIMAL_ACCURACY,   HID_PROP_DECIMAL_ACCURACY,          PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( SHOWTHOUSANDSEP,  RID_STR_SHOWTHOUSANDSEP,    HID_PROP_SHOWTHOUSANDSEP,           PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( REFVALUE,         RID_STR_REFVALUE,           HID_PROP_REFVALUE,                  PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( CURRENCYSYMBOL,   RID_STR_CURRENCYSYMBOL,     HID_PROP_CURRENCYSYMBOL,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( CURRSYM_POSITION, RID_STR_CURRSYM_POSITION,   HID_PROP_CURRSYM_POSITION,          PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( DATE,             RID_STR_DATE,               HID_PROP_DATE,                                             PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DATEMIN,          RID_STR_DATEMIN,            HID_PROP_DATEMIN,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DATEMAX,          RID_STR_DATEMAX,            HID_PROP_DATEMAX,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DATEFORMAT,       RID_STR_DATEFORMAT,         HID_PROP_DATEFORMAT,                PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULT_DATE,     RID_STR_DEFAULTVALUE,       HID_PROP_DEFAULT_DATE,              PROP_FLAG_FORM_VISIBLE                         ),

                DEF_INFO( TIME,             RID_STR_TIME,               HID_PROP_TIME,                                             PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TIMEMIN,          RID_STR_TIMEMIN,            HID_PROP_TIMEMIN,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TIMEMAX,          RID_STR_TIMEMAX,            HID_PROP_TIMEMAX,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TIMEFORMAT,       RID_STR_TIMEFORMAT,         HID_PROP_TIMEFORMAT,                PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULT_TIME,     RID_STR_DEFAULTVALUE,       HID_PROP_DEFAULT_TIME,              PROP_FLAG_FORM_VISIBLE                         ),

                DEF_INFO( EFFECTIVE_VALUE,  RID_STR_VALUE,              HID_PROP_VALUE,                                            PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( EFFECTIVE_MIN,    RID_STR_VALUEMIN,           HID_PROP_EFFECTIVEMIN,              PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( EFFECTIVE_MAX,    RID_STR_VALUEMAX,           HID_PROP_EFFECTIVEMAX,              PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( EFFECTIVE_DEFAULT,RID_STR_DEFAULTVALUE,       HID_PROP_EFFECTIVEDEFAULT,          PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( FORMATKEY,        RID_STR_FORMATKEY,          HID_PROP_FORMATKEY,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( PROGRESSVALUE,    RID_STR_PROGRESSVALUE,      HID_PROP_PROGRESSVALUE,             PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( PROGRESSVALUE_MIN,RID_STR_PROGRESSVALUE_MIN,  HID_PROP_PROGRESSVALUE_MIN,         PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( PROGRESSVALUE_MAX,RID_STR_PROGRESSVALUE_MAX,  HID_PROP_PROGRESSVALUE_MAX,         PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( SCROLLVALUE,      RID_STR_SCROLLVALUE,        HID_PROP_SCROLLVALUE,               PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( SCROLLVALUE_MAX,  RID_STR_SCROLLVALUE_MAX,    HID_PROP_SCROLLVALUE_MAX,           PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( LINEINCREMENT,    RID_STR_LINEINCREMENT,      HID_PROP_LINEINCREMENT,             PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( BLOCKINCREMENT,   RID_STR_BLOCKINCREMENT,     HID_PROP_BLOCKINCREMENT,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( VISIBLESIZE,      RID_STR_VISIBLESIZE,        HID_PROP_VISIBLESIZE,               PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( ORIENTATION,      RID_STR_ORIENTATION,        HID_PROP_ORIENTATION,               PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( CLASSID,          RID_STR_CLASSID,            HID_PROP_CLASSID,                   PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( HEIGHT,           RID_STR_HEIGHT,             HID_PROP_HEIGHT,                    PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( WIDTH,            RID_STR_WIDTH,              HID_PROP_WIDTH,                     PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( POSITIONX,        RID_STR_POSITIONX,          HID_PROP_POSITIONX,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( POSITIONY,        RID_STR_POSITIONY,          HID_PROP_POSITIONY,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),

                DEF_INFO( LISTINDEX,        RID_STR_LISTINDEX,          HID_PROP_LISTINDEX,                 PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( STRINGITEMLIST,   RID_STR_STRINGITEMLIST,     HID_PROP_STRINGITEMLIST,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULT_TEXT,     RID_STR_DEFAULTVALUE,       HID_PROP_DEFAULTVALUE,              PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( FONT_NAME,        RID_STR_FONT,               HID_PROP_FONT,                      PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( ALIGN,            RID_STR_ALIGN,              HID_PROP_ALIGN,                     PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( ROWHEIGHT,        RID_STR_ROWHEIGHT,          HID_PROP_ROWHEIGHT,                 PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( BACKGROUNDCOLOR,  RID_STR_BACKGROUNDCOLOR,    HID_PROP_BACKGROUNDCOLOR,           PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( FILLCOLOR,        RID_STR_FILLCOLOR,          HID_PROP_FILLCOLOR,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( LINECOLOR,        RID_STR_LINECOLOR,          HID_PROP_LINECOLOR,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( BORDER,           RID_STR_BORDER,             HID_PROP_BORDER,                    PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DROPDOWN,         RID_STR_DROPDOWN,           HID_PROP_DROPDOWN,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( AUTOCOMPLETE,     RID_STR_AUTOCOMPLETE,       HID_PROP_AUTOCOMPLETE,              PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( LINECOUNT,        RID_STR_LINECOUNT,          HID_PROP_LINECOUNT,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( MULTI,            RID_STR_MULTILINE,          HID_PROP_MULTILINE,                 PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( MULTILINE,        RID_STR_MULTILINE,          HID_PROP_MULTILINE,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( MULTISELECTION,   RID_STR_MULTISELECTION,     HID_PROP_MULTISELECTION,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( HARDLINEBREAKS,   RID_STR_HARDLINEBREAKS,     HID_PROP_HARDLINEBREAKS,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( HSCROLL,          RID_STR_HSCROLL,            HID_PROP_HSCROLL,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( VSCROLL,          RID_STR_VSCROLL,            HID_PROP_VSCROLL,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( SPIN,             RID_STR_SPIN,               HID_PROP_SPIN,                      PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( BUTTONTYPE,       RID_STR_BUTTONTYPE,         HID_PROP_BUTTONTYPE,                PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( PUSHBUTTONTYPE,   RID_STR_PUSHBUTTONTYPE,     HID_PROP_PUSHBUTTONTYPE,                                   PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TARGET_URL,       RID_STR_TARGET_URL,         HID_PROP_TARGET_URL,                PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( TARGET_FRAME,     RID_STR_TARGET_FRAME,       HID_PROP_TARGET_FRAME,              PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( SUBMIT_ACTION,    RID_STR_SUBMIT_ACTION,      HID_PROP_SUBMIT_ACTION,             PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( SUBMIT_TARGET,    RID_STR_SUBMIT_TARGET,      HID_PROP_SUBMIT_TARGET,             PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( SUBMIT_METHOD,    RID_STR_SUBMIT_METHOD,      HID_PROP_SUBMIT_METHOD,             PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( SUBMIT_ENCODING,  RID_STR_SUBMIT_ENCODING,    HID_PROP_SUBMIT_ENCODING,           PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( STATE,            RID_STR_STATE,              HID_PROP_STATE,                                            PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULTCHECKED,   RID_STR_DEFAULT_CHECKED,    HID_PROP_DEFAULT_CHECKED,           PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( DEFAULTBUTTON,    RID_STR_DEFAULT_BUTTON,     HID_PROP_DEFAULT_BUTTON,            PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( IMAGE_URL,        RID_STR_IMAGE_URL,          HID_PROP_IMAGE_URL,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( IMAGEALIGN,       RID_STR_ALIGN,              HID_PROP_IMAGE_ALIGN,               PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( SCALEIMAGE,       RID_STR_SCALEIMAGE,         HID_PROP_SCALEIMAGE,                PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( DEFAULT_SELECT_SEQ,RID_STR_DEFAULT_SELECT_SEQ,HID_PROP_DEFAULT_SELECT_SEQ,        PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( SELECTEDITEMS,    RID_STR_SELECTEDITEMS,      HID_PROP_SELECTEDITEMS,                                    PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( ECHO_CHAR,        RID_STR_ECHO_CHAR,          HID_PROP_ECHO_CHAR,                 PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( TRISTATE,         RID_STR_TRISTATE,           HID_PROP_TRISTATE,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( HASNAVIGATION,    RID_STR_NAVIGATION,         HID_PROP_NAVIGATIONBAR,             PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( RECORDMARKER,     RID_STR_RECORDMARKER,       HID_PROP_RECORDMARKER,              PROP_FLAG_FORM_VISIBLE                         ),
                DEF_INFO( TAG,              RID_STR_TAG,                HID_PROP_TAG,                       PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( HELPTEXT,         RID_STR_HELPTEXT,           HID_PROP_HELPTEXT,                  PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE),
                DEF_INFO( HELPURL,          RID_STR_HELPURL,            HID_PROP_HELPURL,                   PROP_FLAG_FORM_VISIBLE|PROP_FLAG_DIALOG_VISIBLE)
            };

            s_pPropertyInfos = const_cast<OPropertyInfoImpl*>(aPropertyInfos);
            s_nCount = sizeof(aPropertyInfos) / sizeof(OPropertyInfoImpl);

            // sort
            qsort((void*) aPropertyInfos,
                 s_nCount,
                 sizeof(OPropertyInfoImpl),
                 &PropertyInfoCompare);

        }
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
    Sequence< ::rtl::OUString > OFormPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId) const
    {
        String sSeparatedList;
        switch (_nId)
        {
            case PROPERTY_ID_IMAGEALIGN:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_SIDE_ALIGN));
                break;
            case PROPERTY_ID_BORDER:
                sSeparatedList = String(ModuleRes(RID_STR_BORDER_TYPE));
                break;
            case PROPERTY_ID_COMMANDTYPE:
                sSeparatedList = String(ModuleRes(RID_STR_COMMAND_TYPE));
                break;
            case PROPERTY_ID_LISTSOURCETYPE:
                sSeparatedList = String(ModuleRes(RID_STR_LISTSOURCE_TYPE));
                break;
            case PROPERTY_ID_ALIGN:
                sSeparatedList = String(ModuleRes(RID_STR_ALIGNMENT));
                break;
            case PROPERTY_ID_BUTTONTYPE:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_BUTTONTYPE));
                break;
            case PROPERTY_ID_PUSHBUTTONTYPE:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_PUSHBUTTONTYPE));
                break;
            case PROPERTY_ID_SUBMIT_METHOD:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_SUBMIT_METHOD));
                break;
            case PROPERTY_ID_SUBMIT_ENCODING:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_SUBMIT_ENCODING));
                break;
            case PROPERTY_ID_DATEFORMAT:
                sSeparatedList = String(ModuleRes(RID_STR_DATEFORMAT_LIST));
                break;
            case PROPERTY_ID_TIMEFORMAT:
                sSeparatedList = String(ModuleRes(RID_STR_TIMEFORMAT_LIST));
                break;
            case PROPERTY_ID_DEFAULTCHECKED:
            case PROPERTY_ID_STATE:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_CHECKED));
                break;
            case PROPERTY_ID_CYCLE:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_CYCLE));
                break;
            case PROPERTY_ID_NAVIGATION:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_NAVIGATION));
                break;
            case PROPERTY_ID_TARGET_FRAME:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_SUBMIT_TARGET));
                break;
            case PROPERTY_ID_ORIENTATION:
                sSeparatedList = String(ModuleRes(RID_STR_ENUM_ORIENTATION));
                break;
            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
                sSeparatedList = String( ModuleRes( RID_STR_ENUM_CELL_EXCHANGE_TYPE ) );
                break;
        }

        sal_Int32 nTokens = sSeparatedList.GetTokenCount(';');
        Sequence< ::rtl::OUString > aReturn(nTokens);
        ::rtl::OUString* pReturn = aReturn.getArray();
        for (sal_Int32 i=0; i<nTokens; ++i, ++pReturn)
            *pReturn = sSeparatedList.GetToken((sal_uInt16)i);

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

    //------------------------------------------------------------------------
    EventDisplayDescription* GetEvtTranslation(const ::rtl::OUString& rName)
    {
        static EventDisplayDescription* s_pEventTranslation = NULL;
        static sal_Int32 nCount = 0;
        if (!s_pEventTranslation)
        {
            static EventDisplayDescription __READONLY_DATA aEventDisplayDescriptions[] =
            {
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveAction"),             String(ModuleRes(RID_STR_EVT_APPROVEACTIONPERFORMED)),  HID_EVT_APPROVEACTIONPERFORMED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("actionPerformed"),           String(ModuleRes(RID_STR_EVT_ACTIONPERFORMED)),         HID_EVT_ACTIONPERFORMED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("changed"),                   String(ModuleRes(RID_STR_EVT_CHANGED)),                 HID_EVT_CHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("textChanged"),               String(ModuleRes(RID_STR_EVT_TEXTCHANGED)),             HID_EVT_TEXTCHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("itemStateChanged"),          String(ModuleRes(RID_STR_EVT_ITEMSTATECHANGED)),        HID_EVT_ITEMSTATECHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("focusGained"),               String(ModuleRes(RID_STR_EVT_FOCUSGAINED)),             HID_EVT_FOCUSGAINED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("focusLost"),                 String(ModuleRes(RID_STR_EVT_FOCUSLOST)),               HID_EVT_FOCUSLOST),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("keyPressed"),                String(ModuleRes(RID_STR_EVT_KEYTYPED)),                HID_EVT_KEYTYPED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("keyReleased"),               String(ModuleRes(RID_STR_EVT_KEYUP)),                   HID_EVT_KEYUP),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseEntered"),              String(ModuleRes(RID_STR_EVT_MOUSEENTERED)),            HID_EVT_MOUSEENTERED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseDragged"),              String(ModuleRes(RID_STR_EVT_MOUSEDRAGGED)),            HID_EVT_MOUSEDRAGGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseMoved"),                String(ModuleRes(RID_STR_EVT_MOUSEMOVED)),              HID_EVT_MOUSEMOVED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mousePressed"),              String(ModuleRes(RID_STR_EVT_MOUSEPRESSED)),            HID_EVT_MOUSEPRESSED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseReleased"),             String(ModuleRes(RID_STR_EVT_MOUSERELEASED)),           HID_EVT_MOUSERELEASED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseExited"),               String(ModuleRes(RID_STR_EVT_MOUSEEXITED)),             HID_EVT_MOUSEEXITED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveReset"),              String(ModuleRes(RID_STR_EVT_APPROVERESETTED)),         HID_EVT_APPROVERESETTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("resetted"),                  String(ModuleRes(RID_STR_EVT_RESETTED)),                HID_EVT_RESETTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveSubmit"),             String(ModuleRes(RID_STR_EVT_SUBMITTED)),               HID_EVT_SUBMITTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveUpdate"),             String(ModuleRes(RID_STR_EVT_BEFOREUPDATE)),            HID_EVT_BEFOREUPDATE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("updated"),                   String(ModuleRes(RID_STR_EVT_AFTERUPDATE)),             HID_EVT_AFTERUPDATE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("loaded"),                    String(ModuleRes(RID_STR_EVT_LOADED)),                  HID_EVT_LOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("reloading"),                 String(ModuleRes(RID_STR_EVT_RELOADING)),               HID_EVT_RELOADING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("reloaded"),                  String(ModuleRes(RID_STR_EVT_RELOADED)),                HID_EVT_RELOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("unloading"),                 String(ModuleRes(RID_STR_EVT_UNLOADING)),               HID_EVT_UNLOADING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("unloaded"),                  String(ModuleRes(RID_STR_EVT_UNLOADED)),                HID_EVT_UNLOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("confirmDelete"),             String(ModuleRes(RID_STR_EVT_CONFIRMDELETE)),           HID_EVT_CONFIRMDELETE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveRowChange"),          String(ModuleRes(RID_STR_EVT_APPROVEROWCHANGE)),        HID_EVT_APPROVEROWCHANGE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("rowChanged"),                String(ModuleRes(RID_STR_EVT_ROWCHANGE)),               HID_EVT_ROWCHANGE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveCursorMove"),         String(ModuleRes(RID_STR_EVT_POSITIONING)),             HID_EVT_POSITIONING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("cursorMoved"),               String(ModuleRes(RID_STR_EVT_POSITIONED)),              HID_EVT_POSITIONED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveParameter"),          String(ModuleRes(RID_STR_EVT_APPROVEPARAMETER)),        HID_EVT_APPROVEPARAMETER),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("errorOccured"),              String(ModuleRes(RID_STR_EVT_ERROROCCURED)),            HID_EVT_ERROROCCURED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("adjustmentValueChanged"),    String(ModuleRes(RID_STR_EVT_ADJUSTMENTVALUECHANGED)),  HID_EVT_ADJUSTMENTVALUECHANGED)
            };
            s_pEventTranslation = const_cast<EventDisplayDescription*>(aEventDisplayDescriptions);
            nCount = sizeof(aEventDisplayDescriptions) / sizeof(EventDisplayDescription);

            qsort(static_cast<void*>(s_pEventTranslation),
                 nCount,
                 sizeof(EventDisplayDescription),
                 &EventDisplayDescriptionCompareByName);
        }

        EventDisplayDescription aSearch(0, rName, String(), 0);
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

