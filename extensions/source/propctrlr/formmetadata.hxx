/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#define _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_

#include "propertyinfo.hxx"
#include "modulepcr.hxx"
#include "enumrepresentation.hxx"

//............................................................................
namespace pcr
{
//............................................................................

    struct OPropertyInfoImpl;

    //========================================================================
    //= OPropertyInfoService
    //========================================================================
    class OPropertyInfoService
                :public IPropertyInfoService
                ,public PcrClient
    {
    protected:
        static sal_uInt16               s_nCount;
        static OPropertyInfoImpl*       s_pPropertyInfos;
        // TODO: a real structure which allows quick access by name as well as by id

    public:
        // IPropertyInfoService
        virtual sal_Int32                           getPropertyId(const String& _rName) const;
        virtual String                              getPropertyTranslation(sal_Int32 _nId) const;
        virtual rtl::OString                        getPropertyHelpId(sal_Int32 _nId) const;
        virtual sal_Int16                           getPropertyPos(sal_Int32 _nId) const;
        virtual sal_uInt32                          getPropertyUIFlags(sal_Int32 _nId) const;
        virtual ::std::vector< ::rtl::OUString >    getPropertyEnumRepresentations(sal_Int32 _nId) const;
        virtual String                              getPropertyName( sal_Int32 _nPropId );

        virtual sal_Bool                isComposeable( const ::rtl::OUString& _rPropertyName ) const;

    protected:
        static const OPropertyInfoImpl* getPropertyInfo();

        static const OPropertyInfoImpl* getPropertyInfo(const String& _rName);
        static const OPropertyInfoImpl* getPropertyInfo(sal_Int32 _nId);
    };

    //========================================================================
    //= DefaultEnumRepresentation
    //========================================================================
    /** an implementation of the IPropertyEnumRepresentation

        To be used with properties which, in formmetadata.cxx, are declared as ENUM.
    */
    class DefaultEnumRepresentation : public IPropertyEnumRepresentation
    {
    private:
        oslInterlockedCount         m_refCount;
        const IPropertyInfoService& m_rMetaData;
        ::com::sun::star::uno::Type m_aType;
        const sal_Int32             m_nPropertyId;

    public:
        /** constructs an instance

            @param _rInfo
                An instance implementing IPropertyInfoService. Must live at least as
                long as the DefaultEnumRepresentation should live.
        */
        DefaultEnumRepresentation( const IPropertyInfoService& _rInfo, const ::com::sun::star::uno::Type& _rType, sal_Int32 _nPropertyId );

    protected:
        ~DefaultEnumRepresentation();

    protected:
        // IPropertyEnumRepresentation implementqation
        virtual ::std::vector< ::rtl::OUString >
                                    SAL_CALL getDescriptions() const;
        virtual void                SAL_CALL getValueFromDescription( const ::rtl::OUString& _rDescription, ::com::sun::star::uno::Any& _out_rValue ) const;
        virtual ::rtl::OUString     SAL_CALL getDescriptionForValue( const ::com::sun::star::uno::Any& _rEnumValue ) const;

        // IReference implementqation
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        DefaultEnumRepresentation();                                                // never implemented
        DefaultEnumRepresentation( const DefaultEnumRepresentation& );              // never implemented
        DefaultEnumRepresentation& operator=( const DefaultEnumRepresentation& );   // never implemented
    };

    //========================================================================
    //= UI flags (for all browseable properties)
    //========================================================================

#define PROP_FLAG_NONE              0x00000000  // no special flag
#define PROP_FLAG_FORM_VISIBLE      0x00000001  // the property is visible when inspecting a form object
#define PROP_FLAG_DIALOG_VISIBLE    0x00000002  // the property is visible when inspecting a dialog object
#define PROP_FLAG_DATA_PROPERTY     0x00000004  // the property is to appear on the "Data" page
#define PROP_FLAG_ENUM              0x00000020  // the property is some kind of enum property, i.e. its
                                                // value is chosen from a fixed list of possible values
#define PROP_FLAG_ENUM_ONE          0x00000060  // the property is an enum property starting with 1
                                                //  (note that this includes PROP_FLAG_ENUM)
#define PROP_FLAG_COMPOSEABLE       0x00000080  // the property is "composeable", i.e. an intersection of property
                                                //  sets should expose it, if all elements do
#define PROP_FLAG_EXPERIMENTAL      0x00000100  // the property is experimental, i.e. should not appear in the
                                                // UI, unless experimental properties are enabled by a configuraiton
                                                // option

    //========================================================================
    //= property ids (for all browseable properties)
    //========================================================================

    #define PROPERTY_ID_NAME                  1
    #define PROPERTY_ID_LABEL                 2
    #define PROPERTY_ID_CONTROLLABEL          3
    #define PROPERTY_ID_MAXTEXTLEN            4
    #define PROPERTY_ID_EDITMASK              5
    #define PROPERTY_ID_LITERALMASK           6
    #define PROPERTY_ID_STRICTFORMAT          7
    #define PROPERTY_ID_ENABLED               8
    #define PROPERTY_ID_READONLY              9
    #define PROPERTY_ID_PRINTABLE            10
    #define PROPERTY_ID_CONTROLSOURCE        11
    #define PROPERTY_ID_TABSTOP              12
    #define PROPERTY_ID_TABINDEX             13
    #define PROPERTY_ID_DATASOURCE           14
    #define PROPERTY_ID_COMMAND              15
    #define PROPERTY_ID_COMMANDTYPE          16
    #define PROPERTY_ID_FILTER               17
    #define PROPERTY_ID_SORT                 18
    #define PROPERTY_ID_INSERTONLY           19
    #define PROPERTY_ID_ALLOWADDITIONS       20
    #define PROPERTY_ID_ALLOWEDITS           21
    #define PROPERTY_ID_ALLOWDELETIONS       22
    #define PROPERTY_ID_GROUP_NAME           23
    #define PROPERTY_ID_NAVIGATION           24
    #define PROPERTY_ID_CYCLE                25
    #define PROPERTY_ID_HIDDEN_VALUE         26
    #define PROPERTY_ID_VALUEMIN             27
    #define PROPERTY_ID_VALUEMAX             28
    #define PROPERTY_ID_VALUESTEP            29
    #define PROPERTY_ID_DEFAULT_VALUE        30
    #define PROPERTY_ID_DECIMAL_ACCURACY     31
    #define PROPERTY_ID_SHOWTHOUSANDSEP      32
    #define PROPERTY_ID_REFVALUE             33
    #define PROPERTY_ID_CURRENCYSYMBOL       34
    #define PROPERTY_ID_CURRSYM_POSITION     35
    #define PROPERTY_ID_DATEMIN              36
    #define PROPERTY_ID_DATEMAX              37
    #define PROPERTY_ID_DATEFORMAT           38
    #define PROPERTY_ID_SELECTEDITEMS        39
    #define PROPERTY_ID_DEFAULT_DATE         40
    #define PROPERTY_ID_TIMEMIN              41
    #define PROPERTY_ID_TIMEMAX              42
    #define PROPERTY_ID_TIMEFORMAT           43
    #define PROPERTY_ID_DEFAULT_TIME         44
    #define PROPERTY_ID_EFFECTIVE_MIN        45
    #define PROPERTY_ID_EFFECTIVE_MAX        46
    #define PROPERTY_ID_EFFECTIVE_DEFAULT    47
    #define PROPERTY_ID_FORMATKEY            48
    #define PROPERTY_ID_CLASSID              50
    #define PROPERTY_ID_HEIGHT               51
    #define PROPERTY_ID_WIDTH                52
    #define PROPERTY_ID_BOUNDCOLUMN          53
    #define PROPERTY_ID_LISTSOURCETYPE       54
    #define PROPERTY_ID_LISTSOURCE           55
    #define PROPERTY_ID_LISTINDEX            56
    #define PROPERTY_ID_STRINGITEMLIST       57
    #define PROPERTY_ID_DEFAULT_TEXT         58
    #define PROPERTY_ID_FONT                 59
    #define PROPERTY_ID_ALIGN                60
    #define PROPERTY_ID_ROWHEIGHT            61
    #define PROPERTY_ID_BACKGROUNDCOLOR      62
    #define PROPERTY_ID_FILLCOLOR            63
    #define PROPERTY_ID_ESCAPE_PROCESSING    64
    #define PROPERTY_ID_LINECOLOR            65
    #define PROPERTY_ID_BORDER               66
    #define PROPERTY_ID_DROPDOWN             67
    #define PROPERTY_ID_AUTOCOMPLETE         68
    #define PROPERTY_ID_LINECOUNT            69
    #define PROPERTY_ID_WORDBREAK            70
    #define PROPERTY_ID_MULTILINE            71
    #define PROPERTY_ID_MULTISELECTION       72
    #define PROPERTY_ID_AUTOLINEBREAK        73
    #define PROPERTY_ID_HSCROLL              74
    #define PROPERTY_ID_VSCROLL              75
    #define PROPERTY_ID_SPIN                 76
    #define PROPERTY_ID_BUTTONTYPE           77
    #define PROPERTY_ID_TARGET_URL           78
    #define PROPERTY_ID_TARGET_FRAME         79
    #define PROPERTY_ID_SUBMIT_ACTION        80
    #define PROPERTY_ID_SUBMIT_TARGET        81
    #define PROPERTY_ID_SUBMIT_METHOD        82
    #define PROPERTY_ID_SUBMIT_ENCODING      83
    #define PROPERTY_ID_DEFAULT_STATE        84
    #define PROPERTY_ID_DEFAULTBUTTON        85
    #define PROPERTY_ID_IMAGE_URL            86
    #define PROPERTY_ID_DEFAULT_SELECT_SEQ   87
    #define PROPERTY_ID_ECHO_CHAR            88
    #define PROPERTY_ID_EMPTY_IS_NULL        89
    #define PROPERTY_ID_TRISTATE             90
    #define PROPERTY_ID_MASTERFIELDS         91
    #define PROPERTY_ID_DETAILFIELDS         92
    #define PROPERTY_ID_RECORDMARKER         93
    #define PROPERTY_ID_FILTERPROPOSAL       94
    #define PROPERTY_ID_TAG                  95
    #define PROPERTY_ID_HELPTEXT             96
    #define PROPERTY_ID_HELPURL              97
    #define PROPERTY_ID_HASNAVIGATION        98
    #define PROPERTY_ID_POSITIONX            99
    #define PROPERTY_ID_POSITIONY            100
    #define PROPERTY_ID_TITLE                101
    #define PROPERTY_ID_STEP                 102
    #define PROPERTY_ID_PROGRESSVALUE        103
    #define PROPERTY_ID_PROGRESSVALUE_MIN    104
    #define PROPERTY_ID_PROGRESSVALUE_MAX    105
    #define PROPERTY_ID_SCROLLVALUE          106
    #define PROPERTY_ID_SCROLLVALUE_MAX      107
    #define PROPERTY_ID_LINEINCREMENT        108
    #define PROPERTY_ID_BLOCKINCREMENT       109
    #define PROPERTY_ID_VISIBLESIZE          110
    #define PROPERTY_ID_ORIENTATION          111
    #define PROPERTY_ID_IMAGEPOSITION        112
    #define PROPERTY_ID_DATE                 113
    #define PROPERTY_ID_STATE                114
    #define PROPERTY_ID_TIME                 115
    #define PROPERTY_ID_VALUE                116
    #define PROPERTY_ID_SCALEIMAGE           117
    #define PROPERTY_ID_PUSHBUTTONTYPE       118
    #define PROPERTY_ID_EFFECTIVE_VALUE      119
    #define PROPERTY_ID_TEXT                 120
    #define PROPERTY_ID_BOUND_CELL           121
    #define PROPERTY_ID_LIST_CELL_RANGE      122
    #define PROPERTY_ID_CELL_EXCHANGE_TYPE   123
    #define PROPERTY_ID_SCROLLVALUE_MIN      124
    #define PROPERTY_ID_DEFAULT_SCROLLVALUE  125
    #define PROPERTY_ID_REPEAT_DELAY         126
    #define PROPERTY_ID_SYMBOLCOLOR          127
    #define PROPERTY_ID_SPINVALUE            128
    #define PROPERTY_ID_SPINVALUE_MIN        129
    #define PROPERTY_ID_SPINVALUE_MAX        130
    #define PROPERTY_ID_DEFAULT_SPINVALUE    131
    #define PROPERTY_ID_SPININCREMENT        132
    #define PROPERTY_ID_REPEAT               133
    #define PROPERTY_ID_SHOW_SCROLLBARS      134
    #define PROPERTY_ID_ICONSIZE             135
    #define PROPERTY_ID_SHOW_POSITION        136
    #define PROPERTY_ID_SHOW_NAVIGATION      137
    #define PROPERTY_ID_SHOW_RECORDACTIONS   138
    #define PROPERTY_ID_SHOW_FILTERSORT      139
    #define PROPERTY_ID_TEXTTYPE             140
    #define PROPERTY_ID_LINEEND_FORMAT       141
    #define PROPERTY_ID_TOGGLE               142
    #define PROPERTY_ID_FOCUSONCLICK         143
    #define PROPERTY_ID_HIDEINACTIVESELECTION 144
    #define PROPERTY_ID_VISUALEFFECT         145
    #define PROPERTY_ID_BORDERCOLOR          146
    #define PROPERTY_ID_XML_DATA_MODEL       147
    #define PROPERTY_ID_BIND_EXPRESSION      148
    #define PROPERTY_ID_XSD_REQUIRED         149
    #define PROPERTY_ID_XSD_RELEVANT         150
    #define PROPERTY_ID_XSD_READONLY         151
    #define PROPERTY_ID_XSD_CONSTRAINT       152
    #define PROPERTY_ID_XSD_CALCULATION      153
    #define PROPERTY_ID_XSD_DATA_TYPE        154
    #define PROPERTY_ID_XSD_WHITESPACES      155
    #define PROPERTY_ID_XSD_PATTERN          156
    #define PROPERTY_ID_XSD_LENGTH           157
    #define PROPERTY_ID_XSD_MIN_LENGTH       158
    #define PROPERTY_ID_XSD_MAX_LENGTH       159
    #define PROPERTY_ID_XSD_TOTAL_DIGITS     160
    #define PROPERTY_ID_XSD_FRACTION_DIGITS  161
    #define PROPERTY_ID_XSD_MAX_INCLUSIVE_INT        162
    #define PROPERTY_ID_XSD_MAX_EXCLUSIVE_INT        163
    #define PROPERTY_ID_XSD_MIN_INCLUSIVE_INT        164
    #define PROPERTY_ID_XSD_MIN_EXCLUSIVE_INT        165
    #define PROPERTY_ID_XSD_MAX_INCLUSIVE_DOUBLE     166
    #define PROPERTY_ID_XSD_MAX_EXCLUSIVE_DOUBLE     167
    #define PROPERTY_ID_XSD_MIN_INCLUSIVE_DOUBLE     168
    #define PROPERTY_ID_XSD_MIN_EXCLUSIVE_DOUBLE     169
    #define PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE       170
    #define PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE       171
    #define PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE       172
    #define PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE       173
    #define PROPERTY_ID_XSD_MAX_INCLUSIVE_TIME       174
    #define PROPERTY_ID_XSD_MAX_EXCLUSIVE_TIME       175
    #define PROPERTY_ID_XSD_MIN_INCLUSIVE_TIME       176
    #define PROPERTY_ID_XSD_MIN_EXCLUSIVE_TIME       177
    #define PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE_TIME  178
    #define PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE_TIME  179
    #define PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE_TIME  180
    #define PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE_TIME  181
    #define PROPERTY_ID_UNCHECKEDREFVALUE           182
    #define PROPERTY_ID_SUBMISSION_ID               183
    #define PROPERTY_ID_XFORMS_BUTTONTYPE           184
    #define PROPERTY_ID_LIST_BINDING                185
    #define PROPERTY_ID_VERTICAL_ALIGN              186
    #define PROPERTY_ID_BINDING_NAME                187
    #define PROPERTY_ID_DECORATION                  188
    #define PROPERTY_ID_SELECTION_TYPE              189
    #define PROPERTY_ID_ROOT_DISPLAYED              190
    #define PROPERTY_ID_SHOWS_HANDLES               191
    #define PROPERTY_ID_SHOWS_ROOT_HANDLES          192
    #define PROPERTY_ID_EDITABLE                    193
    #define PROPERTY_ID_INVOKES_STOP_NOT_EDITING    194
    #define PROPERTY_ID_NOLABEL                     195
    #define PROPERTY_ID_SCALE_MODE                  196
    #define PROPERTY_ID_INPUT_REQUIRED              197
    #define PROPERTY_ID_WRITING_MODE                198
    #define PROPERTY_ID_ENABLE_VISIBLE              199
    #define PROPERTY_ID_WHEEL_BEHAVIOR              200
    #define PROPERTY_ID_TEXT_ANCHOR_TYPE            201
    #define PROPERTY_ID_SHEET_ANCHOR_TYPE           202
    #define PROPERTY_ID_SCROLL_WIDTH                203
    #define PROPERTY_ID_SCROLL_HEIGHT               204
    #define PROPERTY_ID_SCROLL_TOP                  205
    #define PROPERTY_ID_SCROLL_LEFT                 206

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
