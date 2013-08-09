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

#ifndef _SDPROPLS_HXX
#define _SDPROPLS_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/xmlnume.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>

// entry list for graphic properties

extern const XMLPropertyMapEntry aXMLSDProperties[];

// entry list for presentation page properties

extern const XMLPropertyMapEntry aXMLSDPresPageProps[];
extern const XMLPropertyMapEntry aXMLSDPresPageProps_onlyHeadersFooter[];

// types of own properties

#define XML_SD_TYPE_STROKE                          (XML_SD_TYPES_START +  0)
#define XML_SD_TYPE_PRESPAGE_TYPE                   (XML_SD_TYPES_START +  1)
#define XML_SD_TYPE_PRESPAGE_STYLE                  (XML_SD_TYPES_START +  2)
#define XML_SD_TYPE_PRESPAGE_SPEED                  (XML_SD_TYPES_START +  3)
#define XML_SD_TYPE_PRESPAGE_DURATION               (XML_SD_TYPES_START +  4)
#define XML_SD_TYPE_PRESPAGE_VISIBILITY             (XML_SD_TYPES_START +  5)
#define XML_SD_TYPE_MARKER                          (XML_SD_TYPES_START +  6 )
#define XML_SD_TYPE_OPACITY                         (XML_SD_TYPES_START +  7 )
#define XML_SD_TYPE_LINEJOIN                        (XML_SD_TYPES_START +  8 )
#define XML_SD_TYPE_FILLSTYLE                       (XML_SD_TYPES_START +  9 )
#define XML_SD_TYPE_GRADIENT                        (XML_SD_TYPES_START + 10 )
#define XML_SD_TYPE_GRADIENT_STEPCOUNT              (XML_SD_TYPES_START + 11 )
#define XML_SD_TYPE_SHADOW                          (XML_SD_TYPES_START + 12 )
#define XML_SD_TYPE_TEXT_CROSSEDOUT                 (XML_SD_TYPES_START + 13 )
#define XML_SD_TYPE_NUMBULLET                       (XML_SD_TYPES_START + 14 )
#define XML_SD_TYPE_WRITINGMODE                     (XML_SD_TYPES_START + 15 )
#define XML_SD_TYPE_BITMAP_MODE                     (XML_SD_TYPES_START + 16 )
#define XML_SD_TYPE_BITMAPREPOFFSETX                (XML_SD_TYPES_START + 17 )
#define XML_SD_TYPE_BITMAPREPOFFSETY                (XML_SD_TYPES_START + 18 )
#define XML_SD_TYPE_FILLBITMAPSIZE                  (XML_SD_TYPES_START + 19 )
#define XML_SD_TYPE_LOGICAL_SIZE                    (XML_SD_TYPES_START + 20 )
#define XML_SD_TYPE_BITMAP_REFPOINT                 (XML_SD_TYPES_START + 21 )
#define XML_SD_TYPE_PRESPAGE_BACKSIZE               (XML_SD_TYPES_START + 22 )
#define XML_TYPE_TEXT_ANIMATION_BLINKING            (XML_SD_TYPES_START + 23 )
#define XML_TYPE_TEXT_ANIMATION_STEPS               (XML_SD_TYPES_START + 24 )
#define XML_SD_TYPE_TEXT_ALIGN                      (XML_SD_TYPES_START + 25 )
#define XML_SD_TYPE_VERTICAL_ALIGN                  (XML_SD_TYPES_START + 26 )
#define XML_SD_TYPE_FITTOSIZE                       (XML_SD_TYPES_START + 27 )
#define XML_SD_TYPE_MEASURE_HALIGN                  (XML_SD_TYPES_START + 28 )
#define XML_SD_TYPE_MEASURE_VALIGN                  (XML_SD_TYPES_START + 29 )
#define XML_SD_TYPE_MEASURE_UNIT                    (XML_SD_TYPES_START + 30 )
#define XML_SD_TYPE_MEASURE_PLACING                 (XML_SD_TYPES_START + 31 )
#define XML_SD_TYPE_CONTROL_BORDER                  (XML_SD_TYPES_START + 32 )
#define XML_SD_TYPE_CONTROL_BORDER_COLOR            (XML_SD_TYPES_START + 33 )
#define XML_SD_TYPE_IMAGE_SCALE_MODE                (XML_SD_TYPES_START + 34 )
#define XML_SD_TYPE_LINECAP                         (XML_SD_TYPES_START + 35 )

// 3D property types
#define XML_SD_TYPE_BACKFACE_CULLING                (XML_SD_TYPES_START + 40 )
#define XML_SD_TYPE_NORMALS_KIND                    (XML_SD_TYPES_START + 41 )
#define XML_SD_TYPE_NORMALS_DIRECTION               (XML_SD_TYPES_START + 42 )
#define XML_SD_TYPE_TEX_GENERATION_MODE_X           (XML_SD_TYPES_START + 43 )
#define XML_SD_TYPE_TEX_GENERATION_MODE_Y           (XML_SD_TYPES_START + 44 )
#define XML_SD_TYPE_TEX_KIND                        (XML_SD_TYPES_START + 45 )
#define XML_SD_TYPE_TEX_MODE                        (XML_SD_TYPES_START + 46 )

// #FontWork# types
#define XML_SD_TYPE_FONTWORK_STYLE                  (XML_SD_TYPES_START + 47 )
#define XML_SD_TYPE_FONTWORK_ADJUST                 (XML_SD_TYPES_START + 48 )
#define XML_SD_TYPE_FONTWORK_SHADOW                 (XML_SD_TYPES_START + 49 )
#define XML_SD_TYPE_FONTWORK_FORM                   (XML_SD_TYPES_START + 50 )

// Caption types
#define XML_SD_TYPE_CAPTION_ANGLE_TYPE              (XML_SD_TYPES_START + 60 )
#define XML_SD_TYPE_CAPTION_IS_ESC_REL              (XML_SD_TYPES_START + 61 )
#define XML_SD_TYPE_CAPTION_ESC_REL                 (XML_SD_TYPES_START + 62 )
#define XML_SD_TYPE_CAPTION_ESC_ABS                 (XML_SD_TYPES_START + 63 )
#define XML_SD_TYPE_CAPTION_ESC_DIR                 (XML_SD_TYPES_START + 64 )
#define XML_SD_TYPE_CAPTION_TYPE                    (XML_SD_TYPES_START + 65 )

// header & footer types
#define XML_SD_TYPE_DATETIMEUPDATE                  (XML_SD_TYPES_START + 70 )
#define XML_SD_TYPE_DATETIME_FORMAT                 (XML_SD_TYPES_START + 71 )

// new types for merged style:protect attribute
#define XML_SD_TYPE_MOVE_PROTECT                    (XML_SD_TYPES_START + 72 )
#define XML_SD_TYPE_SIZE_PROTECT                    (XML_SD_TYPES_START + 73 )

// new type for style:mirror attribute
#define XML_TYPE_SD_MIRROR                          (XML_SD_TYPES_START + 74 )

// new smil transition types for pages
#define XML_SD_TYPE_TRANSITION_TYPE                 (XML_SD_TYPES_START + 75 )
#define XML_SD_TYPE_TRANSTIION_SUBTYPE              (XML_SD_TYPES_START + 76 )
#define XML_SD_TYPE_TRANSTIION_DIRECTION            (XML_SD_TYPES_START + 77 )

#define XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE   (XML_SD_TYPES_START + 78 )

#define CTF_NUMBERINGRULES          1000
#define CTF_CONTROLWRITINGMODE      1001
#define CTF_WRITINGMODE             1002
#define CTF_REPEAT_OFFSET_X         1003
#define CTF_REPEAT_OFFSET_Y         1004
#define CTF_PAGE_SOUND_URL          1005
#define CTF_PAGE_VISIBLE            1006
#define CTF_PAGE_TRANS_TYPE         1007
#define CTF_PAGE_TRANS_STYLE        1008
#define CTF_PAGE_TRANS_SPEED        1009
#define CTF_PAGE_TRANS_DURATION     1010
#define CTF_PAGE_BACKSIZE           1011
#define CTF_DASHNAME                1012
#define CTF_LINESTARTNAME           1013
#define CTF_LINEENDNAME             1014
#define CTF_FILLGRADIENTNAME        1015
#define CTF_FILLHATCHNAME           1016
#define CTF_FILLBITMAPNAME          1017
#define CTF_FILLTRANSNAME           1018
#define CTF_TEXTANIMATION_BLINKING  1019
#define CTF_TEXTANIMATION_KIND      1020

#define CTF_PAGE_TRANSITION_TYPE        1021
#define CTF_PAGE_TRANSITION_SUBTYPE     1022
#define CTF_PAGE_TRANSITION_DIRECTION   1023
#define CTF_PAGE_TRANSITION_FADECOLOR   1024

// #FontWork#
#define CTF_FONTWORK_STYLE              1021
#define CTF_FONTWORK_ADJUST             1022
#define CTF_FONTWORK_DISTANCE           1023
#define CTF_FONTWORK_START              1024
#define CTF_FONTWORK_MIRROR             1025
#define CTF_FONTWORK_OUTLINE            1026
#define CTF_FONTWORK_SHADOW             1027
#define CTF_FONTWORK_SHADOWCOLOR        1028
#define CTF_FONTWORK_SHADOWOFFSETX      1029
#define CTF_FONTWORK_SHADOWOFFSETY      1030
#define CTF_FONTWORK_FORM               1031
#define CTF_FONTWORK_HIDEFORM           1032
#define CTF_FONTWORK_SHADOWTRANSPARENCE 1033

// OLE part 1
#define CTF_SD_OLE_VIS_AREA_IMPORT_LEFT     1040
#define CTF_SD_OLE_VIS_AREA_IMPORT_TOP      1041
#define CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH    1042
#define CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT   1043
#define CTF_SD_OLE_ISINTERNAL               1044

#define CTF_SD_MOVE_PROTECT             1045
#define CTF_SD_SIZE_PROTECT             1046

// caption
#define CTF_CAPTION_ISESCREL            1047
#define CTF_CAPTION_ESCREL              1048
#define CTF_CAPTION_ESCABS              1049

// header&footer
#define CTF_HEADER_VISIBLE              1050
#define CTF_FOOTER_VISIBLE              1051
#define CTF_PAGE_NUMBER_VISIBLE         1052
#define CTF_DATE_TIME_VISIBLE           1053
#define CTF_HEADER_TEXT                 1054
#define CTF_FOOTER_TEXT                 1055
#define CTF_DATE_TIME_TEXT              1056
#define CTF_DATE_TIME_FORMAT            1057
#define CTF_DATE_TIME_UPDATE            1058

// OLE part 2
#define CTF_SD_OLE_ASPECT                   1059
#define CTF_SD_OLE_VIS_AREA_EXPORT_LEFT     1060
#define CTF_SD_OLE_VIS_AREA_EXPORT_TOP      1061
#define CTF_SD_OLE_VIS_AREA_EXPORT_WIDTH    1062
#define CTF_SD_OLE_VIS_AREA_EXPORT_HEIGHT   1063

// enum maps for attributes

extern SvXMLEnumMapEntry aXML_ConnectionKind_EnumMap[];
extern SvXMLEnumMapEntry aXML_CircleKind_EnumMap[];

/** contains the attribute to property mapping for a drawing layer table */
extern const XMLPropertyMapEntry aXMLTableShapeAttributes[];

// factory for own graphic properties

class SvXMLExport;
class SvXMLImport;

class XMLSdPropHdlFactory : public XMLPropertyHandlerFactory
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SvXMLExport* mpExport;
    SvXMLImport* mpImport;

public:
    XMLSdPropHdlFactory( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >, SvXMLExport& rExport );
    XMLSdPropHdlFactory( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >, SvXMLImport& rImport );
    virtual ~XMLSdPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

class XMLShapePropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLShapePropertySetMapper(const UniReference< XMLPropertyHandlerFactory >& rFactoryRef);
    ~XMLShapePropertySetMapper();
};

class XMLShapeExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    SvxXMLNumRuleExport maNumRuleExp;
    sal_Bool mbIsInAutoStyles;

    const OUString msCDATA;
    const OUString msTrue;
    const OUString msFalse;

protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    XMLShapeExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport );
    virtual ~XMLShapeExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;

    void SetAutoStyles( sal_Bool bIsInAutoStyles ) { mbIsInAutoStyles = bIsInAutoStyles; }

    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;
};

class XMLPageExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    SvXMLExport& mrExport;

    const OUString msCDATA;
    const OUString msTrue;
    const OUString msFalse;

protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    XMLPageExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport );
    virtual ~XMLPageExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;
};

#endif  //  _SDPROPLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
