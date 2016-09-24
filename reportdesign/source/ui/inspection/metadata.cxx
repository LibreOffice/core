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
        PropUIFlags     nUIFlags;

        OPropertyInfoImpl(
                        const OUString&        rName,
                        sal_Int32              _nId,
                        const OUString&        aTranslation,
                        const OString&         _sHelpId,
                        PropUIFlags            _nUIFlags);
    };


    OPropertyInfoImpl::OPropertyInfoImpl(const OUString& _rName, sal_Int32 _nId,
                                   const OUString& aString, const OString& sHid, PropUIFlags _nUIFlags)
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
    DEF_INFO( ident, uinameres, helpid, PropUIFlags::flag1 )

#define DEF_INFO_2( ident, uinameres, helpid, flag1, flag2 ) \
    DEF_INFO( ident, uinameres, helpid, PropUIFlags::flag1 | PropUIFlags::flag2 )

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
             DEF_INFO_1( FORCENEWPAGE,                  FORCENEWPAGE,               FORCENEWPAGE,               Composeable )
            ,DEF_INFO_1( NEWROWORCOL,                   NEWROWORCOL,                NEWROWORCOL,                Composeable )
            ,DEF_INFO_1( KEEPTOGETHER,                  KEEPTOGETHER,               KEEPTOGETHER,               Composeable )
            ,DEF_INFO_1( CANGROW,                       CANGROW,                    CANGROW,                    Composeable )
            ,DEF_INFO_1( CANSHRINK,                     CANSHRINK,                  CANSHRINK,                  Composeable )
            ,DEF_INFO_1( REPEATSECTION,                 REPEATSECTION,              REPEATSECTION,              Composeable )
            ,DEF_INFO_1( PRINTREPEATEDVALUES,           PRINTREPEATEDVALUES,        PRINTREPEATEDVALUES,        Composeable )
            ,DEF_INFO_1( CONDITIONALPRINTEXPRESSION,    CONDITIONALPRINTEXPRESSION, CONDITIONALPRINTEXPRESSION, Composeable )
            ,DEF_INFO_1( STARTNEWCOLUMN,                STARTNEWCOLUMN,             STARTNEWCOLUMN,             Composeable )
            ,DEF_INFO_1( RESETPAGENUMBER,               RESETPAGENUMBER,            RESETPAGENUMBER,            Composeable )
            ,DEF_INFO_1( PRINTWHENGROUPCHANGE,          PRINTWHENGROUPCHANGE,       PRINTWHENGROUPCHANGE,       Composeable )
            ,DEF_INFO_1( VISIBLE,                       VISIBLE,                    VISIBLE,                    Composeable )
            ,DEF_INFO_1( GROUPKEEPTOGETHER,             GROUPKEEPTOGETHER,          GROUPKEEPTOGETHER,          Composeable )
            ,DEF_INFO_1( PAGEHEADEROPTION,              PAGEHEADEROPTION,           PAGEHEADEROPTION,           Composeable )
            ,DEF_INFO_1( PAGEFOOTEROPTION,              PAGEFOOTEROPTION,           PAGEFOOTEROPTION,           Composeable )
            ,DEF_INFO_1( POSITIONX,                     POSITIONX,                  RPT_POSITIONX,              Composeable )
            ,DEF_INFO_1( POSITIONY,                     POSITIONY,                  RPT_POSITIONY,              Composeable )
            ,DEF_INFO_1( WIDTH,                         WIDTH,                      RPT_WIDTH,                  Composeable )
            ,DEF_INFO_1( HEIGHT,                        HEIGHT,                     RPT_HEIGHT,                 Composeable )
            ,DEF_INFO_1( FONT,                          FONT,                       RPT_FONT,                   Composeable )
            ,DEF_INFO_1( PREEVALUATED,                  PREEVALUATED,               PREEVALUATED,               Composeable )
            ,DEF_INFO_1( DEEPTRAVERSING,                DEEPTRAVERSING,             DEEPTRAVERSING,             Composeable )
            ,DEF_INFO_1( FORMULA,                       FORMULA,                    FORMULA,                    Composeable )
            ,DEF_INFO_1( INITIALFORMULA,                INITIALFORMULA,             INITIALFORMULA,             Composeable )
            ,DEF_INFO_2( TYPE,                          TYPE,                       TYPE,                       Composeable,DataProperty )
            ,DEF_INFO_2( DATAFIELD,                     DATAFIELD,                  DATAFIELD,                  Composeable,DataProperty )
            ,DEF_INFO_2( FORMULALIST,                   FORMULALIST,                FORMULALIST,                Composeable,DataProperty )
            ,DEF_INFO_2( SCOPE,                         SCOPE,                      SCOPE,                      Composeable,DataProperty )
            ,DEF_INFO_1( PRESERVEIRI,                   PRESERVEIRI,                PRESERVEIRI,                Composeable )
            ,DEF_INFO_1( BACKCOLOR,                     BACKCOLOR,                  BACKCOLOR,                  Composeable )
            ,DEF_INFO_1( CONTROLBACKGROUND,             BACKCOLOR,                  BACKCOLOR,                  Composeable )
            ,DEF_INFO_1( BACKTRANSPARENT,               BACKTRANSPARENT,            BACKTRANSPARENT,            Composeable )
            ,DEF_INFO_1( CONTROLBACKGROUNDTRANSPARENT,  CONTROLBACKGROUNDTRANSPARENT,
                                                                                    CONTROLBACKGROUNDTRANSPARENT, Composeable )
            ,DEF_INFO_1( CHARTTYPE,                     CHARTTYPE,                  CHARTTYPE,                  Composeable )
            ,DEF_INFO_1( PREVIEW_COUNT,                 PREVIEW_COUNT,              PREVIEW_COUNT,              Composeable )
            ,DEF_INFO_2( MASTERFIELDS,                  MASTERFIELDS,               MASTERFIELDS,               Composeable,DataProperty )
            ,DEF_INFO_2( DETAILFIELDS,                  DETAILFIELDS,               DETAILFIELDS,               Composeable,DataProperty)
            ,DEF_INFO_1( AREA,                          AREA,                       AREA,                       Composeable )
            ,DEF_INFO_2( MIMETYPE,                      MIMETYPE,                   MIMETYPE,                   Composeable,DataProperty )
            ,DEF_INFO_1( PARAADJUST,                    PARAADJUST,                 PARAADJUST,                 Composeable )
            ,DEF_INFO_1( VERTICALALIGN,                 VERTICALALIGN,              VERTICALALIGN,              Composeable )
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


    PropUIFlags OPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return (pInfo) ? pInfo->nUIFlags : PropUIFlags::NONE;
    }


    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(const OUString& _rName)
    {
        // intialisierung
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0L, OUString(), "", PropUIFlags::NONE);

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
            PropUIFlags nFlags = getPropertyUIFlags( nId );
            return bool( nFlags & PropUIFlags::Composeable );
        }

        return _rxFormComponentHandler->isComposable( _rPropertyName );
    }


    void OPropertyInfoService::getExcludeProperties(::std::vector< beans::Property >& _rExcludeProperties,const css::uno::Reference< css::inspection::XPropertyHandler >& _xFormComponentHandler)
    {
        uno::Sequence< beans::Property > aProps = _xFormComponentHandler->getSupportedProperties();
        static const OUStringLiteral pExcludeProperties[] =
        {
                OUStringLiteral("Enabled"),
                OUStringLiteral("Printable"),
                OUStringLiteral("WordBreak"),
                OUStringLiteral("MultiLine"),
                OUStringLiteral("Tag"),
                OUStringLiteral("HelpText"),
                OUStringLiteral("HelpURL"),
                OUStringLiteral("MaxTextLen"),
                OUStringLiteral("ReadOnly"),
                OUStringLiteral("Tabstop"),
                OUStringLiteral("TabIndex"),
                OUStringLiteral("ValueMin"),
                OUStringLiteral("ValueMax"),
                OUStringLiteral("Spin"),
                OUStringLiteral("SpinValue"),
                OUStringLiteral("SpinValueMin"),
                OUStringLiteral("SpinValueMax"),
                OUStringLiteral("DefaultSpinValue"),
                OUStringLiteral("SpinIncrement"),
                OUStringLiteral("Repeat"),
                OUStringLiteral("RepeatDelay"),
                OUStringLiteral("ControlLabel"), /// TODO: has to be checked
                OUStringLiteral("LabelControl"),
                OUStringLiteral("Title"), // comment this out if you want to have title feature for charts
                OUStringLiteral(PROPERTY_MAXTEXTLEN),
                OUStringLiteral(PROPERTY_EFFECTIVEDEFAULT),
                OUStringLiteral(PROPERTY_EFFECTIVEMAX),
                OUStringLiteral(PROPERTY_EFFECTIVEMIN),
                OUStringLiteral("HideInactiveSelection"),
                OUStringLiteral("SubmitAction"),
                OUStringLiteral("InputRequired"),
                OUStringLiteral("VerticalAlign"),
                OUStringLiteral(PROPERTY_ALIGN),
                OUStringLiteral(PROPERTY_EMPTY_IS_NULL),
                OUStringLiteral(PROPERTY_FILTERPROPOSAL)
                ,OUStringLiteral(PROPERTY_POSITIONX)
                ,OUStringLiteral(PROPERTY_POSITIONY)
                ,OUStringLiteral(PROPERTY_WIDTH)
                ,OUStringLiteral(PROPERTY_HEIGHT)
                ,OUStringLiteral(PROPERTY_FONT)
                ,OUStringLiteral(PROPERTY_LABEL)
                ,OUStringLiteral(PROPERTY_LINECOLOR)
                ,OUStringLiteral(PROPERTY_BORDER)
                ,OUStringLiteral(PROPERTY_BORDERCOLOR)
                ,OUStringLiteral(PROPERTY_BACKTRANSPARENT)
                ,OUStringLiteral(PROPERTY_CONTROLBACKGROUND)
                ,OUStringLiteral(PROPERTY_BACKGROUNDCOLOR)
                ,OUStringLiteral(PROPERTY_CONTROLBACKGROUNDTRANSPARENT)
                ,OUStringLiteral(PROPERTY_FORMULALIST)
                ,OUStringLiteral(PROPERTY_SCOPE)
                ,OUStringLiteral(PROPERTY_TYPE)
                ,OUStringLiteral(PROPERTY_DATASOURCENAME)
                ,OUStringLiteral(PROPERTY_VERTICALALIGN)
        };

        beans::Property* pPropsIter = aProps.getArray();
        beans::Property* pPropsEnd = pPropsIter + aProps.getLength();
        for (; pPropsIter != pPropsEnd; ++pPropsIter)
        {
            size_t nPos = 0;
            for (; nPos < SAL_N_ELEMENTS(pExcludeProperties) && pExcludeProperties[nPos] != pPropsIter->Name; ++nPos )
                ;
            if ( nPos == SAL_N_ELEMENTS(pExcludeProperties) )
                _rExcludeProperties.push_back(*pPropsIter);
        }
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
