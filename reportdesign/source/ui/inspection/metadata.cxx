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
#include "metadata.hxx"
#include <svtools/localresaccess.hxx>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <comphelper/extract.hxx>
#include "helpids.hrc"
#include "RptResId.hrc"
#include "uistrings.hrc"

#include <functional>
#include <algorithm>


namespace rptui
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star;


    //= OPropertyInfoImpl

    struct OPropertyInfoImpl
    {
        OUString        sName;
        OUString        sTranslation;
        OString         sHelpId;
        sal_Int32       nId;
        sal_uInt32      nUIFlags;

        OPropertyInfoImpl(
                        const OUString&        rName,
                        sal_Int32              _nId,
                        const OUString&        aTranslation,
                        const OString&         _sHelpId,
                        sal_uInt32             _nUIFlags);
    };


    OPropertyInfoImpl::OPropertyInfoImpl(const OUString& _rName, sal_Int32 _nId,
                                   const OUString& aString, const OString& sHid, sal_uInt32 _nUIFlags)
       :sName(_rName)
       ,sTranslation(aString)
       ,sHelpId(sHid)
       ,nId(_nId)
       ,nUIFlags(_nUIFlags)
    {
    }


    // Vergleichen von PropertyInfo
    struct PropertyInfoLessByName : public ::std::binary_function< OPropertyInfoImpl, OPropertyInfoImpl, bool >
    {
        bool operator()( const OPropertyInfoImpl& _lhs, const OPropertyInfoImpl& _rhs )
        {
            return _lhs.sName < _rhs.sName;
        }
    };


    //= OPropertyInfoService

#define DEF_INFO( ident, uinameres, helpid, flags )   \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
            OUString( ModuleRes( RID_STR_##uinameres ) ), HID_RPT_PROP_##helpid, flags )

#define DEF_INFO_1( ident, uinameres, helpid, flag1 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 )

#define DEF_INFO_2( ident, uinameres, helpid, flag1, flag2 ) \
    DEF_INFO( ident, uinameres, helpid, PROP_FLAG_##flag1 | PROP_FLAG_##flag2 )

    sal_uInt16              OPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OPropertyInfoService::s_pPropertyInfos = nullptr;

    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        OModuleClient aResourceAccess;
        // this ensures that we have our resource file loaded

        static OPropertyInfoImpl aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resoure id,         help id,           flags ),
        */
             DEF_INFO_1( FORCENEWPAGE,                  FORCENEWPAGE,               FORCENEWPAGE,               COMPOSEABLE )
            ,DEF_INFO_1( NEWROWORCOL,                   NEWROWORCOL,                NEWROWORCOL,                COMPOSEABLE )
            ,DEF_INFO_1( KEEPTOGETHER,                  KEEPTOGETHER,               KEEPTOGETHER,               COMPOSEABLE )
            ,DEF_INFO_1( CANGROW,                       CANGROW,                    CANGROW,                    COMPOSEABLE )
            ,DEF_INFO_1( CANSHRINK,                     CANSHRINK,                  CANSHRINK,                  COMPOSEABLE )
            ,DEF_INFO_1( REPEATSECTION,                 REPEATSECTION,              REPEATSECTION,              COMPOSEABLE )
            ,DEF_INFO_1( PRINTREPEATEDVALUES,           PRINTREPEATEDVALUES,        PRINTREPEATEDVALUES,        COMPOSEABLE )
            ,DEF_INFO_1( CONDITIONALPRINTEXPRESSION,    CONDITIONALPRINTEXPRESSION, CONDITIONALPRINTEXPRESSION, COMPOSEABLE )
            ,DEF_INFO_1( STARTNEWCOLUMN,                STARTNEWCOLUMN,             STARTNEWCOLUMN,             COMPOSEABLE )
            ,DEF_INFO_1( RESETPAGENUMBER,               RESETPAGENUMBER,            RESETPAGENUMBER,            COMPOSEABLE )
            ,DEF_INFO_1( PRINTWHENGROUPCHANGE,          PRINTWHENGROUPCHANGE,       PRINTWHENGROUPCHANGE,       COMPOSEABLE )
            ,DEF_INFO_1( VISIBLE,                       VISIBLE,                    VISIBLE,                    COMPOSEABLE )
            ,DEF_INFO_1( GROUPKEEPTOGETHER,             GROUPKEEPTOGETHER,          GROUPKEEPTOGETHER,          COMPOSEABLE )
            ,DEF_INFO_1( PAGEHEADEROPTION,              PAGEHEADEROPTION,           PAGEHEADEROPTION,           COMPOSEABLE )
            ,DEF_INFO_1( PAGEFOOTEROPTION,              PAGEFOOTEROPTION,           PAGEFOOTEROPTION,           COMPOSEABLE )
            ,DEF_INFO_1( POSITIONX,                     POSITIONX,                  RPT_POSITIONX,              COMPOSEABLE )
            ,DEF_INFO_1( POSITIONY,                     POSITIONY,                  RPT_POSITIONY,              COMPOSEABLE )
            ,DEF_INFO_1( WIDTH,                         WIDTH,                      RPT_WIDTH,                  COMPOSEABLE )
            ,DEF_INFO_1( HEIGHT,                        HEIGHT,                     RPT_HEIGHT,                 COMPOSEABLE )
            ,DEF_INFO_1( FONT,                          FONT,                       RPT_FONT,                   COMPOSEABLE )
            ,DEF_INFO_1( PREEVALUATED,                  PREEVALUATED,               PREEVALUATED,               COMPOSEABLE )
            ,DEF_INFO_1( DEEPTRAVERSING,                DEEPTRAVERSING,             DEEPTRAVERSING,             COMPOSEABLE )
            ,DEF_INFO_1( FORMULA,                       FORMULA,                    FORMULA,                    COMPOSEABLE )
            ,DEF_INFO_1( INITIALFORMULA,                INITIALFORMULA,             INITIALFORMULA,             COMPOSEABLE )
            ,DEF_INFO_2( TYPE,                          TYPE,                       TYPE,                       COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( DATAFIELD,                     DATAFIELD,                  DATAFIELD,                  COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( FORMULALIST,                   FORMULALIST,                FORMULALIST,                COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( SCOPE,                         SCOPE,                      SCOPE,                      COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_1( PRESERVEIRI,                   PRESERVEIRI,                PRESERVEIRI,                COMPOSEABLE )
            ,DEF_INFO_1( BACKCOLOR,                     BACKCOLOR,                  BACKCOLOR,                  COMPOSEABLE )
            ,DEF_INFO_1( CONTROLBACKGROUND,             BACKCOLOR,                  BACKCOLOR,                  COMPOSEABLE )
            ,DEF_INFO_1( BACKTRANSPARENT,               BACKTRANSPARENT,            BACKTRANSPARENT,            COMPOSEABLE )
            ,DEF_INFO_1( CONTROLBACKGROUNDTRANSPARENT,  CONTROLBACKGROUNDTRANSPARENT,
                                                                                    CONTROLBACKGROUNDTRANSPARENT, COMPOSEABLE )
            ,DEF_INFO_1( CHARTTYPE,                     CHARTTYPE,                  CHARTTYPE,                  COMPOSEABLE )
            ,DEF_INFO_1( PREVIEW_COUNT,                 PREVIEW_COUNT,              PREVIEW_COUNT,              COMPOSEABLE )
            ,DEF_INFO_2( MASTERFIELDS,                  MASTERFIELDS,               MASTERFIELDS,               COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_2( DETAILFIELDS,                  DETAILFIELDS,               DETAILFIELDS,               COMPOSEABLE,DATA_PROPERTY)
            ,DEF_INFO_1( AREA,                          AREA,                       AREA,                       COMPOSEABLE )
            ,DEF_INFO_2( MIMETYPE,                      MIMETYPE,                   MIMETYPE,                   COMPOSEABLE,DATA_PROPERTY )
            ,DEF_INFO_1( PARAADJUST,                    PARAADJUST,                 PARAADJUST,                 COMPOSEABLE )
            ,DEF_INFO_1( VERTICALALIGN,                 VERTICALALIGN,              VERTICALALIGN,              COMPOSEABLE )
        };

        s_pPropertyInfos = aPropertyInfos;
        s_nCount = SAL_N_ELEMENTS(aPropertyInfos);
        ::std::sort( aPropertyInfos, aPropertyInfos + SAL_N_ELEMENTS(aPropertyInfos), PropertyInfoLessByName() );

        return s_pPropertyInfos;
    }


    sal_Int32 OPropertyInfoService::getPropertyId(const OUString& _rName)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_rName);
        return pInfo ? pInfo->nId : -1;
    }


    OUString OPropertyInfoService::getPropertyTranslation(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sTranslation : OUString();
    }


    OString OPropertyInfoService::getPropertyHelpId(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->sHelpId : OString();
    }


    sal_uInt32 OPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nUIFlags : 0;
    }


    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(const OUString& _rName)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0L, OUString(), "", 0);

        const OPropertyInfoImpl* pPropInfo = ::std::lower_bound(
            s_pPropertyInfos, s_pPropertyInfos + s_nCount, aSearch, PropertyInfoLessByName() );

        if ( ( pPropInfo < s_pPropertyInfos + s_nCount ) && pPropInfo->sName == _rName )
            return pPropInfo;

        return nullptr;
    }



    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();

        // TODO: a real structure which allows quick access by name as well as by id
        for (sal_uInt16 i = 0; i < s_nCount; i++)
            if (s_pPropertyInfos[i].nId == _nId)
                return &s_pPropertyInfos[i];

        return nullptr;
    }


    bool OPropertyInfoService::isComposable( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyHandler >& _rxFormComponentHandler )
    {
        sal_Int32 nId = getPropertyId( _rPropertyName );
        if ( nId != -1 )
        {
            sal_uInt32 nFlags = getPropertyUIFlags( nId );
            return ( nFlags & PROP_FLAG_COMPOSEABLE ) != 0;
        }

        return _rxFormComponentHandler->isComposable( _rPropertyName );
    }


    void OPropertyInfoService::getExcludeProperties(::std::vector< beans::Property >& _rExcludeProperties,const css::uno::Reference< css::inspection::XPropertyHandler >& _xFormComponentHandler)
    {
        uno::Sequence< beans::Property > aProps = _xFormComponentHandler->getSupportedProperties();
        static const OUString pExcludeProperties[] =
        {
                OUString("Enabled"),
                OUString("Printable"),
                OUString("WordBreak"),
                OUString("MultiLine"),
                OUString("Tag"),
                OUString("HelpText"),
                OUString("HelpURL"),
                OUString("MaxTextLen"),
                OUString("ReadOnly"),
                OUString("Tabstop"),
                OUString("TabIndex"),
                OUString("ValueMin"),
                OUString("ValueMax"),
                OUString("Spin"),
                OUString("SpinValue"),
                OUString("SpinValueMin"),
                OUString("SpinValueMax"),
                OUString("DefaultSpinValue"),
                OUString("SpinIncrement"),
                OUString("Repeat"),
                OUString("RepeatDelay"),
                OUString("ControlLabel"), /// TODO: has to be checked
                OUString("LabelControl"),
                OUString("Title"), // comment this out if you want to have title feature for charts
                OUString(PROPERTY_MAXTEXTLEN),
                OUString(PROPERTY_EFFECTIVEDEFAULT),
                OUString(PROPERTY_EFFECTIVEMAX),
                OUString(PROPERTY_EFFECTIVEMIN),
                OUString("HideInactiveSelection"),
                OUString("SubmitAction"),
                OUString("InputRequired"),
                OUString("VerticalAlign"),
                OUString(PROPERTY_ALIGN),
                OUString(PROPERTY_EMPTY_IS_NULL),
                OUString(PROPERTY_FILTERPROPOSAL)
                ,OUString(PROPERTY_POSITIONX)
                ,OUString(PROPERTY_POSITIONY)
                ,OUString(PROPERTY_WIDTH)
                ,OUString(PROPERTY_HEIGHT)
                ,OUString(PROPERTY_FONT)
                ,OUString(PROPERTY_LABEL)
                ,OUString(PROPERTY_LINECOLOR)
                ,OUString(PROPERTY_BORDER)
                ,OUString(PROPERTY_BORDERCOLOR)
                ,OUString(PROPERTY_BACKTRANSPARENT)
                ,OUString(PROPERTY_CONTROLBACKGROUND)
                ,OUString(PROPERTY_BACKGROUNDCOLOR)
                ,OUString(PROPERTY_CONTROLBACKGROUNDTRANSPARENT)
                ,OUString(PROPERTY_FORMULALIST)
                ,OUString(PROPERTY_SCOPE)
                ,OUString(PROPERTY_TYPE)
                ,OUString(PROPERTY_DATASOURCENAME)
                ,OUString(PROPERTY_VERTICALALIGN)
        };

        beans::Property* pPropsIter = aProps.getArray();
        beans::Property* pPropsEnd = pPropsIter + aProps.getLength();
        for (; pPropsIter != pPropsEnd; ++pPropsIter)
        {
            size_t nPos = 0;
            for (; nPos < sizeof(pExcludeProperties)/sizeof(pExcludeProperties[0]) && pExcludeProperties[nPos] != pPropsIter->Name;++nPos )
                ;
            if ( nPos == sizeof(pExcludeProperties)/sizeof(pExcludeProperties[0]) )
                _rExcludeProperties.push_back(*pPropsIter);
        }
    }



} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
