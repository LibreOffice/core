/*************************************************************************
 *
 *  $RCSfile: formmetadata.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-22 09:31:44 $
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
        sal_Bool        bMultiSelectable;

        OPropertyInfoImpl(
                        const String&               rName,
                        sal_Int32                   _nId,
                        sal_Bool                    bMSel,
                        const String&               aTranslation,
                        sal_uInt16                  nPosId,
                        sal_uInt32                  nHelpId);
    };

    //------------------------------------------------------------------------
    OPropertyInfoImpl::OPropertyInfoImpl(const String& _rName, sal_Int32 _nId, sal_Bool bMSel,
                                   const String& aString, sal_uInt16 nP, sal_uInt32 nHid)
       :sName(_rName)
       ,nId(_nId)
       ,bMultiSelectable(bMSel)
       ,sTranslation(aString)
       ,nPos(nP)
       ,nHelpId(nHid)
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
                OPropertyInfoImpl(PROPERTY_NAME,                PROPERTY_ID_NAME,               sal_False, String(ModuleRes(RID_STR_NAME)),             nPos++, HID_PROP_NAME) ,
                OPropertyInfoImpl(PROPERTY_LABEL,               PROPERTY_ID_LABEL,              sal_True,  String(ModuleRes(RID_STR_LABEL)),            nPos++, HID_PROP_LABEL),
                OPropertyInfoImpl(PROPERTY_CONTROLLABEL,        PROPERTY_ID_CONTROLLABEL,       sal_False, String(ModuleRes(RID_STR_LABELCONTROL)),     nPos++, HID_PROP_CONTROLLABEL),
                OPropertyInfoImpl(PROPERTY_MAXTEXTLEN,          PROPERTY_ID_MAXTEXTLEN,         sal_True,  String(ModuleRes(RID_STR_MAXTEXTLEN)),       nPos++, HID_PROP_MAXTEXTLEN),
                OPropertyInfoImpl(PROPERTY_EDITMASK,            PROPERTY_ID_EDITMASK,           sal_True,  String(ModuleRes(RID_STR_EDITMASK)),         nPos++, HID_PROP_EDITMASK),
                OPropertyInfoImpl(PROPERTY_LITERALMASK,         PROPERTY_ID_LITERALMASK,        sal_True,  String(ModuleRes(RID_STR_LITERALMASK)),      nPos++, HID_PROP_LITERALMASK),
                OPropertyInfoImpl(PROPERTY_STRICTFORMAT,        PROPERTY_ID_STRICTFORMAT,       sal_True,  String(ModuleRes(RID_STR_STRICTFORMAT)),     nPos++, HID_PROP_STRICTFORMAT),
                OPropertyInfoImpl(PROPERTY_ENABLED,             PROPERTY_ID_ENABLED,            sal_True,  String(ModuleRes(RID_STR_ENABLED)),          nPos++, HID_PROP_ENABLED) ,
                OPropertyInfoImpl(PROPERTY_READONLY,            PROPERTY_ID_READONLY,           sal_True,  String(ModuleRes(RID_STR_READONLY)),         nPos++, HID_PROP_READONLY) ,
                OPropertyInfoImpl(PROPERTY_PRINTABLE,           PROPERTY_ID_PRINTABLE,          sal_True,  String(ModuleRes(RID_STR_PRINTABLE)),        nPos++, HID_PROP_PRINTABLE) ,
                OPropertyInfoImpl(PROPERTY_CONTROLSOURCE,       PROPERTY_ID_CONTROLSOURCE,      sal_True,  String(ModuleRes(RID_STR_CONTROLSOURCE)),    nPos++, HID_PROP_CONTROLSOURCE),
                OPropertyInfoImpl(PROPERTY_TABSTOP,             PROPERTY_ID_TABSTOP,            sal_True,  String(ModuleRes(RID_STR_TABSTOP)),          nPos++, HID_PROP_TABSTOP),
                OPropertyInfoImpl(PROPERTY_TABINDEX,            PROPERTY_ID_TABINDEX,           sal_True,  String(ModuleRes(RID_STR_TABINDEX)),         nPos++, HID_PROP_TABINDEX) ,
                OPropertyInfoImpl(PROPERTY_DATASOURCE,          PROPERTY_ID_DATASOURCE,         sal_True,  String(ModuleRes(RID_STR_DATASOURCE)),       nPos++, HID_PROP_DATASOURCE) ,
                OPropertyInfoImpl(PROPERTY_COMMAND,             PROPERTY_ID_COMMAND,            sal_False, String(ModuleRes(RID_STR_CURSORSOURCE)),     nPos++, HID_PROP_CURSORSOURCE),
                OPropertyInfoImpl(PROPERTY_COMMANDTYPE,         PROPERTY_ID_COMMANDTYPE,        sal_False, String(ModuleRes(RID_STR_CURSORSOURCETYPE)), nPos++, HID_PROP_CURSORSOURCETYPE),
                OPropertyInfoImpl(PROPERTY_ESCAPE_PROCESSING,   PROPERTY_ID_ESCAPE_PROCESSING,  sal_False, String(ModuleRes(RID_STR_ESCAPE_PROCESSING)),    nPos++, HID_PROP_ESCAPE_PROCESSING),
                OPropertyInfoImpl(PROPERTY_FILTER_CRITERIA,     PROPERTY_ID_FILTER_CRITERIA,    sal_False,  String(ModuleRes(RID_STR_FILTER_CRITERIA)), nPos++, HID_PROP_FILTER_CRITERIA ),
                OPropertyInfoImpl(PROPERTY_SORT,                PROPERTY_ID_SORT,               sal_False,  String(ModuleRes(RID_STR_SORT_CRITERIA)),   nPos++, HID_PROP_SORT_CRITERIA) ,
                OPropertyInfoImpl(PROPERTY_ALLOWADDITIONS,      PROPERTY_ID_ALLOWADDITIONS,     sal_True,  String(ModuleRes(RID_STR_ALLOW_ADDITIONS)),  nPos++, HID_PROP_ALLOW_ADDITIONS) ,
                OPropertyInfoImpl(PROPERTY_ALLOWEDITS,          PROPERTY_ID_ALLOWEDITS,         sal_True,  String(ModuleRes(RID_STR_ALLOW_EDITS)),      nPos++, HID_PROP_ALLOW_EDITS ) ,
                OPropertyInfoImpl(PROPERTY_ALLOWDELETIONS,      PROPERTY_ID_ALLOWDELETIONS,     sal_True,  String(ModuleRes(RID_STR_ALLOW_DELETIONS)),  nPos++, HID_PROP_ALLOW_DELETIONS) ,
                OPropertyInfoImpl(PROPERTY_INSERTONLY,          PROPERTY_ID_INSERTONLY,         sal_True,  String(ModuleRes(RID_STR_DATAENTRY)),        nPos++, HID_PROP_DATAENTRY) ,
                OPropertyInfoImpl(PROPERTY_NAVIGATION,          PROPERTY_ID_NAVIGATION,         sal_True,  String(ModuleRes(RID_STR_NAVIGATION)),       nPos++, HID_PROP_NAVIGATION) ,
                OPropertyInfoImpl(PROPERTY_CYCLE,               PROPERTY_ID_CYCLE,              sal_True,  String(ModuleRes(RID_STR_CYCLE)),            nPos++, HID_PROP_CYCLE) ,
                OPropertyInfoImpl(PROPERTY_HIDDEN_VALUE,        PROPERTY_ID_HIDDEN_VALUE,       sal_True,  String(ModuleRes(RID_STR_VALUE)),            nPos++, HID_PROP_HIDDEN_VALUE),
                OPropertyInfoImpl(PROPERTY_VALUEMIN,            PROPERTY_ID_VALUEMIN,           sal_True,  String(ModuleRes(RID_STR_VALUEMIN)),         nPos++, HID_PROP_VALUEMIN) ,
                OPropertyInfoImpl(PROPERTY_VALUEMAX,            PROPERTY_ID_VALUEMAX,           sal_True,  String(ModuleRes(RID_STR_VALUEMAX)),         nPos++, HID_PROP_VALUEMAX) ,
                OPropertyInfoImpl(PROPERTY_VALUESTEP,           PROPERTY_ID_VALUESTEP,          sal_True,  String(ModuleRes(RID_STR_VALUESTEP)),        nPos++, HID_PROP_VALUESTEP) ,
                OPropertyInfoImpl(PROPERTY_DEFAULT_VALUE,       PROPERTY_ID_DEFAULT_VALUE,      sal_True,  String(ModuleRes(RID_STR_DEFAULTVALUE)),     nPos++, HID_PROP_DEFAULT_LONG_VALUE ),
                OPropertyInfoImpl(PROPERTY_DECIMAL_ACCURACY,    PROPERTY_ID_DECIMAL_ACCURACY,   sal_True,  String(ModuleRes(RID_STR_DECIMAL_ACCURACY)), nPos++, HID_PROP_DECIMAL_ACCURACY ),
                OPropertyInfoImpl(PROPERTY_SHOWTHOUSANDSEP,     PROPERTY_ID_SHOWTHOUSANDSEP,    sal_True,  String(ModuleRes(RID_STR_SHOWTHOUSANDSEP)),  nPos++, HID_PROP_SHOWTHOUSANDSEP),

                OPropertyInfoImpl(PROPERTY_REFVALUE,            PROPERTY_ID_REFVALUE,           sal_True,  String(ModuleRes(RID_STR_REFVALUE)),         nPos++, HID_PROP_REFVALUE),
                OPropertyInfoImpl(PROPERTY_CURRENCYSYMBOL,      PROPERTY_ID_CURRENCYSYMBOL,     sal_True,  String(ModuleRes(RID_STR_CURRENCYSYMBOL)),   nPos++, HID_PROP_CURRENCYSYMBOL),
                OPropertyInfoImpl(PROPERTY_CURRSYM_POSITION,    PROPERTY_ID_CURRSYM_POSITION,   sal_True,  String(ModuleRes(RID_STR_CURRSYM_POSITION)), nPos++, HID_PROP_CURRSYM_POSITION),

                OPropertyInfoImpl(PROPERTY_DATEMIN,             PROPERTY_ID_DATEMIN,            sal_True,  String(ModuleRes(RID_STR_DATEMIN)),          nPos++, HID_PROP_DATEMIN) ,
                OPropertyInfoImpl(PROPERTY_DATEMAX,             PROPERTY_ID_DATEMAX,            sal_True,  String(ModuleRes(RID_STR_DATEMAX)),          nPos++, HID_PROP_DATEMAX) ,
                OPropertyInfoImpl(PROPERTY_DATEFORMAT,          PROPERTY_ID_DATEFORMAT,         sal_True,  String(ModuleRes(RID_STR_DATEFORMAT)),       nPos++, HID_PROP_DATEFORMAT) ,
                OPropertyInfoImpl(PROPERTY_DATE_SHOW_CENTURY,   PROPERTY_ID_DATE_SHOW_CENTURY,  sal_True,  String(ModuleRes(RID_STR_DATE_SHOW_CENTURY)),nPos++, HID_PROP_DATE_SHOW_CENTURY ),
                OPropertyInfoImpl(PROPERTY_DEFAULT_DATE,        PROPERTY_ID_DEFAULT_DATE,       sal_True,  String(ModuleRes(RID_STR_DEFAULTVALUE)),     nPos++, HID_PROP_DEFAULT_DATE ),

                OPropertyInfoImpl(PROPERTY_TIMEMIN,             PROPERTY_ID_TIMEMIN,            sal_True,  String(ModuleRes(RID_STR_TIMEMIN)),          nPos++, HID_PROP_TIMEMIN) ,
                OPropertyInfoImpl(PROPERTY_TIMEMAX,             PROPERTY_ID_TIMEMAX,            sal_True,  String(ModuleRes(RID_STR_TIMEMAX)),          nPos++, HID_PROP_TIMEMAX) ,
                OPropertyInfoImpl(PROPERTY_TIMEFORMAT,          PROPERTY_ID_TIMEFORMAT,         sal_True,  String(ModuleRes(RID_STR_TIMEFORMAT)),       nPos++, HID_PROP_TIMEFORMAT) ,
                OPropertyInfoImpl(PROPERTY_DEFAULT_TIME,        PROPERTY_ID_DEFAULT_TIME,       sal_True,  String(ModuleRes(RID_STR_DEFAULTVALUE)),     nPos++, HID_PROP_DEFAULT_TIME ),

                OPropertyInfoImpl(PROPERTY_EFFECTIVE_MIN,       PROPERTY_ID_EFFECTIVE_MIN,      sal_False, String(ModuleRes(RID_STR_VALUEMIN)),         nPos++, HID_PROP_EFFECTIVEMIN),
                OPropertyInfoImpl(PROPERTY_EFFECTIVE_MAX,       PROPERTY_ID_EFFECTIVE_MAX,      sal_False, String(ModuleRes(RID_STR_VALUEMAX)),         nPos++, HID_PROP_EFFECTIVEMAX),
                OPropertyInfoImpl(PROPERTY_EFFECTIVE_DEFAULT,   PROPERTY_ID_EFFECTIVE_DEFAULT,  sal_False, String(ModuleRes(RID_STR_DEFAULTVALUE)),     nPos++, HID_PROP_EFFECTIVEDEFAULT),
                OPropertyInfoImpl(PROPERTY_FORMATKEY,           PROPERTY_ID_FORMATKEY,          sal_True,  String(ModuleRes(RID_STR_FORMATKEY)),        nPos++, HID_PROP_FORMATKEY),

                OPropertyInfoImpl(PROPERTY_CLASSID,             PROPERTY_ID_CLASSID,            sal_False, String(ModuleRes(RID_STR_CLASSID)),          nPos++, HID_PROP_CLASSID),
                OPropertyInfoImpl(PROPERTY_HEIGHT,              PROPERTY_ID_HEIGHT,             sal_True,  String(ModuleRes(RID_STR_HEIGHT)),           nPos++, HID_PROP_HEIGHT),
                OPropertyInfoImpl(PROPERTY_WIDTH,               PROPERTY_ID_WIDTH,              sal_True,  String(ModuleRes(RID_STR_WIDTH)),            nPos++, HID_PROP_WIDTH),
                OPropertyInfoImpl(PROPERTY_POSITIONX,           PROPERTY_ID_POSITIONX,          sal_True,  String(ModuleRes(RID_STR_POSITIONX)),        nPos++, 0),
                OPropertyInfoImpl(PROPERTY_POSITIONY,           PROPERTY_ID_POSITIONY,          sal_True,  String(ModuleRes(RID_STR_POSITIONY)),        nPos++, 0),

                OPropertyInfoImpl(PROPERTY_BOUNDCOLUMN,         PROPERTY_ID_BOUNDCOLUMN,        sal_True,  String(ModuleRes(RID_STR_BOUNDCOLUMN)),      nPos++, HID_PROP_BOUNDCOLUMN),
                OPropertyInfoImpl(PROPERTY_LISTSOURCETYPE,      PROPERTY_ID_LISTSOURCETYPE,     sal_True,  String(ModuleRes(RID_STR_LISTSOURCETYPE)),   nPos++, HID_PROP_LISTSOURCETYPE),
                OPropertyInfoImpl(PROPERTY_LISTSOURCE,          PROPERTY_ID_LISTSOURCE,         sal_True,  String(ModuleRes(RID_STR_LISTSOURCE)),       nPos++, HID_PROP_LISTSOURCE),
                OPropertyInfoImpl(PROPERTY_LISTINDEX,           PROPERTY_ID_LISTINDEX,          sal_True,  String(ModuleRes(RID_STR_LISTINDEX)),        nPos++, HID_PROP_LISTINDEX),
                OPropertyInfoImpl(PROPERTY_STRINGITEMLIST,      PROPERTY_ID_STRINGITEMLIST,     sal_True,  String(ModuleRes(RID_STR_STRINGITEMLIST)),   nPos++, HID_PROP_STRINGITEMLIST),
                OPropertyInfoImpl(PROPERTY_DEFAULT_TEXT,        PROPERTY_ID_DEFAULT_TEXT,       sal_True,  String(ModuleRes(RID_STR_DEFAULTVALUE)),     nPos++, HID_PROP_DEFAULTVALUE ),
                OPropertyInfoImpl(PROPERTY_FONT_NAME,           PROPERTY_ID_FONT_NAME,          sal_True,  String(ModuleRes(RID_STR_FONT)),             nPos++, HID_PROP_FONT),
                OPropertyInfoImpl(PROPERTY_ALIGN,               PROPERTY_ID_ALIGN,              sal_True,  String(ModuleRes(RID_STR_ALIGN)),            nPos++, HID_PROP_ALIGN),
                OPropertyInfoImpl(PROPERTY_ROWHEIGHT,           PROPERTY_ID_ROWHEIGHT,          sal_True,  String(ModuleRes(RID_STR_ROWHEIGHT)),        nPos++, HID_PROP_ROWHEIGHT),
                OPropertyInfoImpl(PROPERTY_BACKGROUNDCOLOR,     PROPERTY_ID_BACKGROUNDCOLOR,    sal_True,  String(ModuleRes(RID_STR_BACKGROUNDCOLOR)),  nPos++, HID_PROP_BACKGROUNDCOLOR),
                OPropertyInfoImpl(PROPERTY_FILLCOLOR,           PROPERTY_ID_FILLCOLOR,          sal_True,  String(ModuleRes(RID_STR_FILLCOLOR)),        nPos++, HID_PROP_FILLCOLOR),
                OPropertyInfoImpl(PROPERTY_LINECOLOR,           PROPERTY_ID_LINECOLOR,          sal_True,  String(ModuleRes(RID_STR_LINECOLOR)),        nPos++, HID_PROP_LINECOLOR),
                OPropertyInfoImpl(PROPERTY_BORDER,              PROPERTY_ID_BORDER,             sal_True,  String(ModuleRes(RID_STR_BORDER)),           nPos++, HID_PROP_BORDER),
                OPropertyInfoImpl(PROPERTY_DROPDOWN,            PROPERTY_ID_DROPDOWN,           sal_True,  String(ModuleRes(RID_STR_DROPDOWN)),         nPos++, HID_PROP_DROPDOWN),
                OPropertyInfoImpl(PROPERTY_AUTOCOMPLETE,        PROPERTY_ID_AUTOCOMPLETE,       sal_True,  String(ModuleRes(RID_STR_AUTOCOMPLETE)),     nPos++, HID_PROP_AUTOCOMPLETE),
                OPropertyInfoImpl(PROPERTY_LINECOUNT,           PROPERTY_ID_LINECOUNT,          sal_True,  String(ModuleRes(RID_STR_LINECOUNT)),        nPos++, HID_PROP_LINECOUNT),
                OPropertyInfoImpl(PROPERTY_MULTI,               PROPERTY_ID_MULTI,              sal_True,  String(ModuleRes(RID_STR_MULTILINE)),        nPos++, HID_PROP_MULTILINE),
                OPropertyInfoImpl(PROPERTY_MULTILINE,           PROPERTY_ID_MULTILINE,          sal_True,  String(ModuleRes(RID_STR_MULTILINE)),        nPos++, HID_PROP_MULTILINE),
                OPropertyInfoImpl(PROPERTY_MULTISELECTION,      PROPERTY_ID_MULTISELECTION,     sal_True,  String(ModuleRes(RID_STR_MULTISELECTION)),   nPos++, HID_PROP_MULTISELECTION),
                OPropertyInfoImpl(PROPERTY_HARDLINEBREAKS,      PROPERTY_ID_HARDLINEBREAKS,     sal_True,  String(ModuleRes(RID_STR_HARDLINEBREAKS)),   nPos++, HID_PROP_HARDLINEBREAKS),
                OPropertyInfoImpl(PROPERTY_HSCROLL,             PROPERTY_ID_HSCROLL,            sal_True,  String(ModuleRes(RID_STR_HSCROLL)),          nPos++, HID_PROP_HSCROLL),
                OPropertyInfoImpl(PROPERTY_VSCROLL,             PROPERTY_ID_VSCROLL,            sal_True,  String(ModuleRes(RID_STR_VSCROLL)),          nPos++, HID_PROP_VSCROLL),
                OPropertyInfoImpl(PROPERTY_SPIN,                PROPERTY_ID_SPIN,               sal_True,  String(ModuleRes(RID_STR_SPIN)),             nPos++, HID_PROP_SPIN),
                OPropertyInfoImpl(PROPERTY_BUTTONTYPE,          PROPERTY_ID_BUTTONTYPE,         sal_True,  String(ModuleRes(RID_STR_BUTTONTYPE)),       nPos++, HID_PROP_BUTTONTYPE),
                OPropertyInfoImpl(PROPERTY_TARGET_URL,          PROPERTY_ID_TARGET_URL,         sal_True,  String(ModuleRes(RID_STR_TARGET_URL)),       nPos++, HID_PROP_TARGET_URL ),
                OPropertyInfoImpl(PROPERTY_TARGET_FRAME,        PROPERTY_ID_TARGET_FRAME,       sal_True,  String(ModuleRes(RID_STR_TARGET_FRAME)),     nPos++, HID_PROP_TARGET_FRAME ),
                OPropertyInfoImpl(PROPERTY_SUBMIT_ACTION,       PROPERTY_ID_SUBMIT_ACTION,      sal_True,  String(ModuleRes(RID_STR_SUBMIT_ACTION)),    nPos++, HID_PROP_SUBMIT_ACTION ),
                OPropertyInfoImpl(PROPERTY_SUBMIT_TARGET,       PROPERTY_ID_SUBMIT_TARGET,      sal_True,  String(ModuleRes(RID_STR_SUBMIT_TARGET)),    nPos++, HID_PROP_SUBMIT_TARGET ),
                OPropertyInfoImpl(PROPERTY_SUBMIT_METHOD,       PROPERTY_ID_SUBMIT_METHOD,      sal_True,  String(ModuleRes(RID_STR_SUBMIT_METHOD)),    nPos++, HID_PROP_SUBMIT_METHOD ),
                OPropertyInfoImpl(PROPERTY_SUBMIT_ENCODING,     PROPERTY_ID_SUBMIT_ENCODING,    sal_True,  String(ModuleRes(RID_STR_SUBMIT_ENCODING)),  nPos++, HID_PROP_SUBMIT_ENCODING ),
                OPropertyInfoImpl(PROPERTY_DEFAULTCHECKED,      PROPERTY_ID_DEFAULT_CHECKED,    sal_True,  String(ModuleRes(RID_STR_DEFAULT_CHECKED)),  nPos++, HID_PROP_DEFAULT_CHECKED ),
                OPropertyInfoImpl(PROPERTY_DEFAULTBUTTON,       PROPERTY_ID_DEFAULT_BUTTON,     sal_True,  String(ModuleRes(RID_STR_DEFAULT_BUTTON)),   nPos++, HID_PROP_DEFAULT_BUTTON ),
                OPropertyInfoImpl(PROPERTY_IMAGE_URL,           PROPERTY_ID_IMAGE_URL,          sal_True,  String(ModuleRes(RID_STR_IMAGE_URL)),        nPos++, HID_PROP_IMAGE_URL ),
                OPropertyInfoImpl(PROPERTY_DEFAULT_SELECT_SEQ,  PROPERTY_ID_DEFAULT_SELECT_SEQ, sal_True,  String(ModuleRes(RID_STR_DEFAULT_SELECT_SEQ)),nPos++, HID_PROP_DEFAULT_SELECT_SEQ ),
                OPropertyInfoImpl(PROPERTY_ECHO_CHAR,           PROPERTY_ID_ECHO_CHAR,          sal_True,  String(ModuleRes(RID_STR_ECHO_CHAR)),        nPos++, HID_PROP_ECHO_CHAR ),
                OPropertyInfoImpl(PROPERTY_EMPTY_IS_NULL,       PROPERTY_ID_EMPTY_IS_NULL,      sal_True,  String(ModuleRes(RID_STR_EMPTY_IS_NULL)),    nPos++, HID_PROP_EMPTY_IS_NULL ),
                OPropertyInfoImpl(PROPERTY_TRISTATE,            PROPERTY_ID_TRISTATE    ,       sal_True,  String(ModuleRes(RID_STR_TRISTATE)),         nPos++, HID_PROP_TRISTATE ),
                OPropertyInfoImpl(PROPERTY_MASTERFIELDS,        PROPERTY_ID_MASTERFIELDS,       sal_True,  String(ModuleRes(RID_STR_MASTERFIELDS)),     nPos++, HID_PROP_MASTERFIELDS) ,
                OPropertyInfoImpl(PROPERTY_DETAILFIELDS,        PROPERTY_ID_DETAILFIELDS,       sal_True,  String(ModuleRes(RID_STR_SLAVEFIELDS)),      nPos++, HID_PROP_SLAVEFIELDS),
                OPropertyInfoImpl(PROPERTY_HASNAVIGATION,       PROPERTY_ID_HASNAVIGATION,      sal_True,  String(ModuleRes(RID_STR_NAVIGATION)),       nPos++, HID_PROP_NAVIGATIONBAR) ,
                OPropertyInfoImpl(PROPERTY_RECORDMARKER,        PROPERTY_ID_RECORDMARKER,       sal_True,  String(ModuleRes(RID_STR_RECORDMARKER)),     nPos++, HID_PROP_RECORDMARKER) ,
                OPropertyInfoImpl(PROPERTY_FILTERPROPOSAL,      PROPERTY_ID_FILTERPROPOSAL,     sal_True,  String(ModuleRes(RID_STR_FILTERPROPOSAL)),   nPos++, HID_PROP_FILTERPROPOSAL) ,
                OPropertyInfoImpl(PROPERTY_TAG,                 PROPERTY_ID_TAG,                sal_True,  String(ModuleRes(RID_STR_TAG)),              nPos++, HID_PROP_TAG ),
                OPropertyInfoImpl(PROPERTY_HELPTEXT,            PROPERTY_ID_HELPTEXT,           sal_False, String(ModuleRes(RID_STR_HELPTEXT)),         nPos++, HID_PROP_HELPTEXT),
                OPropertyInfoImpl(PROPERTY_HELPURL,             PROPERTY_ID_HELPURL,            sal_False, String(ModuleRes(RID_STR_HELPURL)),          nPos++, HID_PROP_HELPURL)
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
    sal_Bool OFormPropertyInfoService::getPropertyMultiFlag(sal_Int32 _nId) const
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->bMultiSelectable : sal_False;
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > OFormPropertyInfoService::getPropertyEnumRepresentations(sal_Int32 _nId) const
    {
        String sSeparatedList;
        switch (_nId)
        {
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
            case PROPERTY_ID_DEFAULT_CHECKED:
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
        OPropertyInfoImpl  aSearch(_rName, 0L, sal_False, String(), 0, 0);

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
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveAction"),         String(ModuleRes(RID_STR_EVT_APPROVEACTIONPERFORMED)),HID_EVT_APPROVEACTIONPERFORMED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("actionPerformed"),       String(ModuleRes(RID_STR_EVT_ACTIONPERFORMED)),     HID_EVT_ACTIONPERFORMED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("changed"),               String(ModuleRes(RID_STR_EVT_CHANGED)),             HID_EVT_CHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("textChanged"),           String(ModuleRes(RID_STR_EVT_TEXTCHANGED)),         HID_EVT_TEXTCHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("itemStateChanged"),      String(ModuleRes(RID_STR_EVT_ITEMSTATECHANGED)),    HID_EVT_ITEMSTATECHANGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("focusGained"),           String(ModuleRes(RID_STR_EVT_FOCUSGAINED)),         HID_EVT_FOCUSGAINED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("focusLost"),             String(ModuleRes(RID_STR_EVT_FOCUSLOST)),           HID_EVT_FOCUSLOST),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("keyPressed"),            String(ModuleRes(RID_STR_EVT_KEYTYPED)),            HID_EVT_KEYTYPED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("keyReleased"),           String(ModuleRes(RID_STR_EVT_KEYUP)),               HID_EVT_KEYUP),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseEntered"),          String(ModuleRes(RID_STR_EVT_MOUSEENTERED)),        HID_EVT_MOUSEENTERED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseDragged"),          String(ModuleRes(RID_STR_EVT_MOUSEDRAGGED)),        HID_EVT_MOUSEDRAGGED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseMoved"),            String(ModuleRes(RID_STR_EVT_MOUSEMOVED)),          HID_EVT_MOUSEMOVED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mousePressed"),          String(ModuleRes(RID_STR_EVT_MOUSEPRESSED)),        HID_EVT_MOUSEPRESSED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseReleased"),         String(ModuleRes(RID_STR_EVT_MOUSERELEASED)),       HID_EVT_MOUSERELEASED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("mouseExited"),           String(ModuleRes(RID_STR_EVT_MOUSEEXITED)),         HID_EVT_MOUSEEXITED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveReset"),          String(ModuleRes(RID_STR_EVT_APPROVERESETTED)),     HID_EVT_APPROVERESETTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("resetted"),              String(ModuleRes(RID_STR_EVT_RESETTED)),            HID_EVT_RESETTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveSubmit"),         String(ModuleRes(RID_STR_EVT_SUBMITTED)),           HID_EVT_SUBMITTED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveUpdate"),         String(ModuleRes(RID_STR_EVT_BEFOREUPDATE)),        HID_EVT_BEFOREUPDATE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("updated"),               String(ModuleRes(RID_STR_EVT_AFTERUPDATE)),         HID_EVT_AFTERUPDATE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("loaded"),                String(ModuleRes(RID_STR_EVT_LOADED)),              HID_EVT_LOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("reloading"),             String(ModuleRes(RID_STR_EVT_RELOADING)),           HID_EVT_RELOADING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("reloaded"),              String(ModuleRes(RID_STR_EVT_RELOADED)),            HID_EVT_RELOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("unloading"),             String(ModuleRes(RID_STR_EVT_UNLOADING)),           HID_EVT_UNLOADING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("unloaded"),              String(ModuleRes(RID_STR_EVT_UNLOADED)),            HID_EVT_UNLOADED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("confirmDelete"),         String(ModuleRes(RID_STR_EVT_CONFIRMDELETE)),       HID_EVT_CONFIRMDELETE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveRowChange"),      String(ModuleRes(RID_STR_EVT_APPROVEROWCHANGE)),    HID_EVT_APPROVEROWCHANGE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("rowChanged"),            String(ModuleRes(RID_STR_EVT_ROWCHANGE)),           HID_EVT_ROWCHANGE),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveCursorMove"),     String(ModuleRes(RID_STR_EVT_POSITIONING)),         HID_EVT_POSITIONING),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("cursorMoved"),           String(ModuleRes(RID_STR_EVT_POSITIONED)),          HID_EVT_POSITIONED),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("approveParameter"),      String(ModuleRes(RID_STR_EVT_APPROVEPARAMETER)),    HID_EVT_APPROVEPARAMETER),
                EventDisplayDescription(nCount++,::rtl::OUString::createFromAscii("errorOccured"),          String(ModuleRes(RID_STR_EVT_ERROROCCURED)),        HID_EVT_ERROROCCURED)
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2001/02/06 10:19:29  fs
 *  #83479# reintroduced the HasNavigationBar property
 *
 *  Revision 1.3  2001/02/05 09:33:13  fs
 *  #83424# +RID_STR_ENUM_SUBMIT_TARGET
 *
 *  Revision 1.2  2001/01/17 08:41:48  fs
 *  #82726# renamed the display text for the navigation bar
 *
 *  Revision 1.1  2001/01/12 11:28:26  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 09.01.01 15:35:18  fs
 ************************************************************************/

