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
#include <string_view>

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

    sal_uInt16              OPropertyInfoService::s_nCount = 0;
    OPropertyInfoImpl*      OPropertyInfoService::s_pPropertyInfos = nullptr;

    const OPropertyInfoImpl* OPropertyInfoService::getPropertyInfo()
    {
        if ( s_pPropertyInfos )
            return s_pPropertyInfos;

        static OPropertyInfoImpl aPropertyInfos[] =
        {
            OPropertyInfoImpl(PROPERTY_FORCENEWPAGE, PROPERTY_ID_FORCENEWPAGE, RptResId(RID_STR_FORCENEWPAGE), HID_RPT_PROP_FORCENEWPAGE, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_NEWROWORCOL, PROPERTY_ID_NEWROWORCOL, RptResId(RID_STR_NEWROWORCOL), HID_RPT_PROP_NEWROWORCOL, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_KEEPTOGETHER, PROPERTY_ID_KEEPTOGETHER, RptResId(RID_STR_KEEPTOGETHER), HID_RPT_PROP_KEEPTOGETHER, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CANGROW, PROPERTY_ID_CANGROW, RptResId(RID_STR_CANGROW), HID_RPT_PROP_CANGROW, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CANSHRINK, PROPERTY_ID_CANSHRINK, RptResId(RID_STR_CANSHRINK), HID_RPT_PROP_CANSHRINK, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_REPEATSECTION, PROPERTY_ID_REPEATSECTION, RptResId(RID_STR_REPEATSECTION), HID_RPT_PROP_REPEATSECTION, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PRINTREPEATEDVALUES, PROPERTY_ID_PRINTREPEATEDVALUES, RptResId(RID_STR_PRINTREPEATEDVALUES), HID_RPT_PROP_PRINTREPEATEDVALUES, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CONDITIONALPRINTEXPRESSION, PROPERTY_ID_CONDITIONALPRINTEXPRESSION, RptResId(RID_STR_CONDITIONALPRINTEXPRESSION), HID_RPT_PROP_CONDITIONALPRINTEXPRESSION,
                              PropUIFlags::Composeable ),
            OPropertyInfoImpl(PROPERTY_STARTNEWCOLUMN, PROPERTY_ID_STARTNEWCOLUMN, RptResId(RID_STR_STARTNEWCOLUMN), HID_RPT_PROP_STARTNEWCOLUMN, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_RESETPAGENUMBER, PROPERTY_ID_RESETPAGENUMBER, RptResId(RID_STR_RESETPAGENUMBER), HID_RPT_PROP_RESETPAGENUMBER, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PRINTWHENGROUPCHANGE, PROPERTY_ID_PRINTWHENGROUPCHANGE, RptResId(RID_STR_PRINTWHENGROUPCHANGE), HID_RPT_PROP_PRINTWHENGROUPCHANGE, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_VISIBLE, PROPERTY_ID_VISIBLE, RptResId(RID_STR_VISIBLE), HID_RPT_PROP_VISIBLE, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_GROUPKEEPTOGETHER, PROPERTY_ID_GROUPKEEPTOGETHER, RptResId(RID_STR_GROUPKEEPTOGETHER), HID_RPT_PROP_GROUPKEEPTOGETHER, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PAGEHEADEROPTION, PROPERTY_ID_PAGEHEADEROPTION, RptResId(RID_STR_PAGEHEADEROPTION), HID_RPT_PROP_PAGEHEADEROPTION, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PAGEFOOTEROPTION, PROPERTY_ID_PAGEFOOTEROPTION, RptResId(RID_STR_PAGEFOOTEROPTION), HID_RPT_PROP_PAGEFOOTEROPTION, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_POSITIONX, PROPERTY_ID_POSITIONX, RptResId(RID_STR_POSITIONX), HID_RPT_PROP_RPT_POSITIONX, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_POSITIONY, PROPERTY_ID_POSITIONY, RptResId(RID_STR_POSITIONY), HID_RPT_PROP_RPT_POSITIONY, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_WIDTH, PROPERTY_ID_WIDTH, RptResId(RID_STR_WIDTH), HID_RPT_PROP_RPT_WIDTH, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_HEIGHT, PROPERTY_ID_HEIGHT, RptResId(RID_STR_HEIGHT), HID_RPT_PROP_RPT_HEIGHT, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_AUTOGROW, PROPERTY_ID_AUTOGROW, RptResId(RID_STR_AUTOGROW), HID_RPT_PROP_RPT_AUTOGROW, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_FONT, PROPERTY_ID_FONT, RptResId(RID_STR_FONT), HID_RPT_PROP_RPT_FONT, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PREEVALUATED, PROPERTY_ID_PREEVALUATED, RptResId(RID_STR_PREEVALUATED), HID_RPT_PROP_PREEVALUATED, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_DEEPTRAVERSING, PROPERTY_ID_DEEPTRAVERSING, RptResId(RID_STR_DEEPTRAVERSING), HID_RPT_PROP_DEEPTRAVERSING, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_FORMULA, PROPERTY_ID_FORMULA, RptResId(RID_STR_FORMULA), HID_RPT_PROP_FORMULA, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_INITIALFORMULA, PROPERTY_ID_INITIALFORMULA, RptResId(RID_STR_INITIALFORMULA), HID_RPT_PROP_INITIALFORMULA, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_TYPE, PROPERTY_ID_TYPE, RptResId(RID_STR_TYPE), HID_RPT_PROP_TYPE, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_DATAFIELD,PROPERTY_ID_DATAFIELD, RptResId(RID_STR_DATAFIELD), HID_RPT_PROP_DATAFIELD, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_FORMULALIST, PROPERTY_ID_FORMULALIST, RptResId(RID_STR_FORMULALIST), HID_RPT_PROP_FORMULALIST, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_SCOPE, PROPERTY_ID_SCOPE, RptResId(RID_STR_SCOPE), HID_RPT_PROP_SCOPE, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_PRESERVEIRI, PROPERTY_ID_PRESERVEIRI, RptResId(RID_STR_PRESERVEIRI), HID_RPT_PROP_PRESERVEIRI, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_BACKCOLOR, PROPERTY_ID_BACKCOLOR, RptResId(RID_STR_BACKCOLOR), HID_RPT_PROP_BACKCOLOR, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CONTROLBACKGROUND, PROPERTY_ID_CONTROLBACKGROUND, RptResId(RID_STR_BACKCOLOR), HID_RPT_PROP_BACKCOLOR, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_BACKTRANSPARENT, PROPERTY_ID_BACKTRANSPARENT, RptResId(RID_STR_BACKTRANSPARENT), HID_RPT_PROP_BACKTRANSPARENT, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CONTROLBACKGROUNDTRANSPARENT, PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT, RptResId(RID_STR_CONTROLBACKGROUNDTRANSPARENT), HID_RPT_PROP_CONTROLBACKGROUNDTRANSPARENT,
                              PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_CHARTTYPE, PROPERTY_ID_CHARTTYPE, RptResId(RID_STR_CHARTTYPE), HID_RPT_PROP_CHARTTYPE, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_PREVIEW_COUNT, PROPERTY_ID_PREVIEW_COUNT, RptResId(RID_STR_PREVIEW_COUNT), HID_RPT_PROP_PREVIEW_COUNT, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_MASTERFIELDS, PROPERTY_ID_MASTERFIELDS, RptResId(RID_STR_MASTERFIELDS), HID_RPT_PROP_MASTERFIELDS, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_DETAILFIELDS, PROPERTY_ID_DETAILFIELDS, RptResId(RID_STR_DETAILFIELDS), HID_RPT_PROP_DETAILFIELDS, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_AREA, PROPERTY_ID_AREA, RptResId(RID_STR_AREA), HID_RPT_PROP_AREA, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_MIMETYPE, PROPERTY_ID_MIMETYPE, RptResId(RID_STR_MIMETYPE), HID_RPT_PROP_MIMETYPE, PropUIFlags::Composeable | PropUIFlags::DataProperty),
            OPropertyInfoImpl(PROPERTY_PARAADJUST, PROPERTY_ID_PARAADJUST, RptResId(RID_STR_PARAADJUST), HID_RPT_PROP_PARAADJUST, PropUIFlags::Composeable),
            OPropertyInfoImpl(PROPERTY_VERTICALALIGN, PROPERTY_ID_VERTICALALIGN, RptResId(RID_STR_VERTICALALIGN), HID_RPT_PROP_VERTICALALIGN, PropUIFlags::Composeable)
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
        static const std::u16string_view pExcludeProperties[] =
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
                 PROPERTY_ALIGN,
                 PROPERTY_EMPTY_IS_NULL,
                 PROPERTY_FILTERPROPOSAL
                ,PROPERTY_POSITIONX
                ,PROPERTY_POSITIONY
                ,PROPERTY_WIDTH
                ,PROPERTY_HEIGHT
                ,PROPERTY_AUTOGROW
                ,PROPERTY_FONT
                ,PROPERTY_LABEL
                ,PROPERTY_LINECOLOR
                ,PROPERTY_BORDER
                ,PROPERTY_BORDERCOLOR
                ,PROPERTY_BACKTRANSPARENT
                ,PROPERTY_CONTROLBACKGROUND
                ,PROPERTY_BACKGROUNDCOLOR
                ,PROPERTY_CONTROLBACKGROUNDTRANSPARENT
                ,PROPERTY_FORMULALIST
                ,PROPERTY_SCOPE
                ,PROPERTY_TYPE
                ,PROPERTY_DATASOURCENAME
                ,PROPERTY_VERTICALALIGN
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
