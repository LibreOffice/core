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
#include <metadata.hxx>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <core_resource.hxx>
#include <helpids.h>
#include <strings.hrc>
#include <strings.hxx>

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

    namespace {

    // compare PropertyInfo
    struct PropertyInfoLessByName
    {
        bool operator()( const OPropertyInfoImpl& _lhs, const OPropertyInfoImpl& _rhs )
        {
            return _lhs.sName < _rhs.sName;
        }
    };

    }

    //= OPropertyInfoService

#define DEF_INFO( ident, uinameres, helpid, flags )   \
    OPropertyInfoImpl( PROPERTY_##ident, PROPERTY_ID_##ident, \
                       RptResId( RID_STR_##uinameres ), HID_RPT_PROP_##helpid, flags )

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

        static OPropertyInfoImpl aPropertyInfos[] =
        {
        /*
        DEF_INFO_?( propname and id,   resource id,         help id,           flags ),
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
            ,DEF_INFO_1( AUTOGROW,                      AUTOGROW,                   RPT_AUTOGROW,               Composeable )
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
        return pInfo ? pInfo->sTranslation : OUString();
    }


    OString OPropertyInfoService::getPropertyHelpId(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return pInfo ? pInfo->sHelpId : OString();
    }


    PropUIFlags OPropertyInfoService::getPropertyUIFlags(sal_Int32 _nId)
    {
        const OPropertyInfoImpl* pInfo = getPropertyInfo(_nId);
        return pInfo ? pInfo->nUIFlags : PropUIFlags::NONE;
    }


    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(const OUString& _rName)
    {
        // initialization
        if(!s_pPropertyInfos)
            getPropertyInfo();
        OPropertyInfoImpl  aSearch(_rName, 0, OUString(), "", PropUIFlags::NONE);

        const OPropertyInfoImpl* pPropInfo = ::std::lower_bound(
            s_pPropertyInfos, s_pPropertyInfos + s_nCount, aSearch, PropertyInfoLessByName() );

        if ( ( pPropInfo < s_pPropertyInfos + s_nCount ) && pPropInfo->sName == _rName )
            return pPropInfo;

        return nullptr;
    }


    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
    {
        // initialization
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
        const uno::Sequence< beans::Property > aProps = _xFormComponentHandler->getSupportedProperties();
        static const OUStringLiteral pExcludeProperties[] =
        {
                u"Enabled",
                u"Printable",
                u"WordBreak",
                u"MultiLine",
                u"Tag",
                u"HelpText",
                u"HelpURL",
                u"MaxTextLen",
                u"ReadOnly",
                u"Tabstop",
                u"TabIndex",
                u"ValueMin",
                u"ValueMax",
                u"Spin",
                u"SpinValue",
                u"SpinValueMin",
                u"SpinValueMax",
                u"DefaultSpinValue",
                u"SpinIncrement",
                u"Repeat",
                u"RepeatDelay",
                u"ControlLabel", /// TODO: has to be checked
                u"LabelControl",
                u"Title", // comment this out if you want to have title feature for charts
                u"" PROPERTY_MAXTEXTLEN,
                u"" PROPERTY_EFFECTIVEDEFAULT,
                u"" PROPERTY_EFFECTIVEMAX,
                u"" PROPERTY_EFFECTIVEMIN,
                u"HideInactiveSelection",
                u"SubmitAction",
                u"InputRequired",
                u"VerticalAlign",
                u"" PROPERTY_ALIGN,
                u"" PROPERTY_EMPTY_IS_NULL,
                u"" PROPERTY_FILTERPROPOSAL
                ,u"" PROPERTY_POSITIONX
                ,u"" PROPERTY_POSITIONY
                ,u"" PROPERTY_WIDTH
                ,u"" PROPERTY_HEIGHT
                ,u"" PROPERTY_AUTOGROW
                ,u"" PROPERTY_FONT
                ,u"" PROPERTY_LABEL
                ,u"" PROPERTY_LINECOLOR
                ,u"" PROPERTY_BORDER
                ,u"" PROPERTY_BORDERCOLOR
                ,u"" PROPERTY_BACKTRANSPARENT
                ,u"" PROPERTY_CONTROLBACKGROUND
                ,u"" PROPERTY_BACKGROUNDCOLOR
                ,u"" PROPERTY_CONTROLBACKGROUNDTRANSPARENT
                ,u"" PROPERTY_FORMULALIST
                ,u"" PROPERTY_SCOPE
                ,u"" PROPERTY_TYPE
                ,u"" PROPERTY_DATASOURCENAME
                ,u"" PROPERTY_VERTICALALIGN
        };

        for (beans::Property const & prop : aProps)
        {
            size_t nPos = 0;
            for (; nPos < SAL_N_ELEMENTS(pExcludeProperties) && pExcludeProperties[nPos] != prop.Name; ++nPos )
                ;
            if ( nPos == SAL_N_ELEMENTS(pExcludeProperties) )
                _rExcludeProperties.push_back(prop);
        }
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
