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
#ifndef RPTUI_METADATA_HXX_
#define RPTUI_METADATA_HXX_

#include "ModuleHelper.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <tools/string.hxx>

//............................................................................
namespace rptui
{
//............................................................................

    struct OPropertyInfoImpl;

    //========================================================================
    //= OPropertyInfoService
    //========================================================================
    class OPropertyInfoService
        :public OModuleClient
    {
        OPropertyInfoService(const OPropertyInfoService&);
        void operator =(const OPropertyInfoService&);
    protected:
        static sal_uInt16               s_nCount;
        static OPropertyInfoImpl*       s_pPropertyInfos;
        // TODO: a real structure which allows quick access by name as well as by id

    public:
        OPropertyInfoService(){}
        virtual ~OPropertyInfoService(){}
        // IPropertyInfoService
        sal_Int32                           getPropertyId(const String& _rName) const;
        String                              getPropertyTranslation(sal_Int32 _nId) const;
        OString                        getPropertyHelpId(sal_Int32 _nId) const;
        sal_uInt32                          getPropertyUIFlags(sal_Int32 _nId) const;
        static void                         getExcludeProperties(::std::vector< com::sun::star::beans::Property >& _rExcludeProperties,const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _xFormComponentHandler);

        bool                                isComposable(
                                                const OUString& _rPropertyName,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _xFormComponentHandler
                                            );

    protected:
        static const OPropertyInfoImpl* getPropertyInfo();

        static const OPropertyInfoImpl* getPropertyInfo(const String& _rName);
        static const OPropertyInfoImpl* getPropertyInfo(sal_Int32 _nId);
    };

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static OUString getHelpURL( const OString& _sHelpId );
    };

    //========================================================================
    //= UI flags (for all browseable properties)
    //========================================================================

#define PROP_FLAG_NONE              0x00000001  // no special flag
#define PROP_FLAG_ENUM              0x00000002  // the property is some kind of enum property, i.e. its
                                                // value is chosen from a fixed list of possible values
#define PROP_FLAG_ENUM_ONE          0x00000004  // the property is an enum property starting with 1
                                                //  (note that this includes PROP_FLAG_ENUM)
#define PROP_FLAG_COMPOSEABLE       0x00000008  // the property is "composeable", i.e. an intersection of property
                                                //  sets should expose it, if all elements do
#define PROP_FLAG_EXPERIMENTAL      0x00000010  // the property is experimental, i.e. should not appear in the
                                                // UI, unless experimental properties are enabled by a configuraiton
                                                // option
#define PROP_FLAG_DATA_PROPERTY     0x00000020  // the property is to appear on the "Data" page

    //========================================================================
    //= property ids (for all browseable properties)
    //= The ID is used for the view order in the property browser.
    //========================================================================

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


//............................................................................
} // namespace rptui
//............................................................................

#endif // RPTUI_METADATA_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
