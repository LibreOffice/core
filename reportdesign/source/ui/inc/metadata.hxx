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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_METADATA_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_METADATA_HXX

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <vector>

    //= UI flags (for all browseable properties)
enum class PropUIFlags {
    NONE          = 0x0001,  // no special flag
    Composeable   = 0x0002,  // the property is "composeable", i.e. an intersection of property
                             //  sets should expose it, if all elements do
    DataProperty  = 0x0004   // the property is to appear on the "Data" page
};
namespace o3tl {
    template<> struct typed_flags<PropUIFlags> : is_typed_flags<PropUIFlags, 0x0007> {};
}


namespace rptui
{


    struct OPropertyInfoImpl;


    //= OPropertyInfoService

    class OPropertyInfoService
    {
        OPropertyInfoService(const OPropertyInfoService&) = delete;
        void operator =(const OPropertyInfoService&) = delete;
        OPropertyInfoService() = delete;
    protected:
        static sal_uInt16               s_nCount;
        static OPropertyInfoImpl*       s_pPropertyInfos;
        // TODO: a real structure which allows quick access by name as well as by id

    public:
        // IPropertyInfoService
        static sal_Int32                    getPropertyId(const OUString& _rName);
        static OUString                     getPropertyTranslation(sal_Int32 _nId);
        static OString                      getPropertyHelpId(sal_Int32 _nId);
        static PropUIFlags                  getPropertyUIFlags(sal_Int32 _nId);
        static void                         getExcludeProperties(::std::vector< css::beans::Property >& _rExcludeProperties,const css::uno::Reference< css::inspection::XPropertyHandler >& _xFormComponentHandler);

        static bool                         isComposable(
                                                const OUString& _rPropertyName,
                                                const css::uno::Reference< css::inspection::XPropertyHandler >& _xFormComponentHandler
                                            );

    protected:
        static const OPropertyInfoImpl* getPropertyInfo();

        static const OPropertyInfoImpl* getPropertyInfo(const OUString& _rName);
        static const OPropertyInfoImpl* getPropertyInfo(sal_Int32 _nId);
    };


    //= HelpIdUrl

    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static OUString getHelpURL( const OString& _sHelpId );
    };


    //= property ids (for all browseable properties)
    //= The ID is used for the view order in the property browser.


    #define PROPERTY_ID_FORCENEWPAGE                    1
    #define PROPERTY_ID_NEWROWORCOL                     2
    #define PROPERTY_ID_KEEPTOGETHER                    3
    #define PROPERTY_ID_CANGROW                         4
    #define PROPERTY_ID_CANSHRINK                       5
    #define PROPERTY_ID_REPEATSECTION                   6
    #define PROPERTY_ID_PRESERVEIRI                     7
    #define PROPERTY_ID_VISIBLE                         8
    #define PROPERTY_ID_GROUPKEEPTOGETHER               9
    #define PROPERTY_ID_PAGEHEADEROPTION                10
    #define PROPERTY_ID_PAGEFOOTEROPTION                11
    #define PROPERTY_ID_POSITIONX                       12
    #define PROPERTY_ID_POSITIONY                       13
    #define PROPERTY_ID_WIDTH                           14
    #define PROPERTY_ID_HEIGHT                          15
    #define PROPERTY_ID_FORMULA                         16
    #define PROPERTY_ID_CONDITIONALPRINTEXPRESSION      17
    #define PROPERTY_ID_PRINTREPEATEDVALUES             18
    #define PROPERTY_ID_PRINTWHENGROUPCHANGE            19
    #define PROPERTY_ID_INITIALFORMULA                  20
    #define PROPERTY_ID_STARTNEWCOLUMN                  21
    #define PROPERTY_ID_TYPE                            22
    #define PROPERTY_ID_DATAFIELD                       23
    #define PROPERTY_ID_CHARFONTNAME                    24
    #define PROPERTY_ID_DEEPTRAVERSING                  25
    #define PROPERTY_ID_PREEVALUATED                    26

    #define PROPERTY_ID_BACKTRANSPARENT                 27
    #define PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT    28
    #define PROPERTY_ID_BACKCOLOR                       29
    #define PROPERTY_ID_CONTROLBACKGROUND               30

    #define PROPERTY_ID_FORMULALIST                     31
    #define PROPERTY_ID_SCOPE                           32
    #define PROPERTY_ID_RESETPAGENUMBER                 33
    #define PROPERTY_ID_CHARTTYPE                       34
    #define PROPERTY_ID_MASTERFIELDS                    35
    #define PROPERTY_ID_DETAILFIELDS                    36
    #define PROPERTY_ID_PREVIEW_COUNT                   37
    #define PROPERTY_ID_AREA                            38
    #define PROPERTY_ID_MIMETYPE                        39

    #define PROPERTY_ID_FONT                            40
    #define PROPERTY_ID_PARAADJUST                      41
    #define PROPERTY_ID_VERTICALALIGN                   42


} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_METADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
