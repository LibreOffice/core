/*************************************************************************
 *
 *  $RCSfile: shapeimport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-06 12:08:07 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _XMLOFF_SHAPEIMPORT_HXX
#include "shapeimport.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

XMLShapeImportHelper::XMLShapeImportHelper(const uno::Reference< frame::XModel>& rModel)
:   mxModel(rModel),
    mpSdPropHdlFactory(0L),
    mpPropertySetMapper(0L),
    mpPresPagePropsMapper(0L),
    mpStylesContext(0L),
    mpAutoStylesContext(0L),
    mpGroupShapeElemTokenMap(0L),
    mpShapeAttrTokenMap(0L),
    mpRectShapeAttrTokenMap(0L),
    mpLineShapeAttrTokenMap(0L),
    mpEllipseShapeAttrTokenMap(0L),
    mpPolygonShapeAttrTokenMap(0L),
    mpPathShapeAttrTokenMap(0L),
    mpTextBoxShapeAttrTokenMap(0L),
    mpControlShapeAttrTokenMap(0L),
    mpPageShapeAttrTokenMap(0L),
    mpGraphicObjectShapeAttrTokenMap(0L)
{
    // prepare factory parts
    mpSdPropHdlFactory = new XMLSdPropHdlFactory;
    if(mpSdPropHdlFactory)
    {
        // set lock to avoid deletion
        mpSdPropHdlFactory->acquire();

        // build one ref
        const UniReference< XMLPropertyHandlerFactory > aFactoryRef = mpSdPropHdlFactory;

        // construct PropertySetMapper
        mpPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLSDProperties, aFactoryRef);
        if(mpPropertySetMapper)
        {
            // set lock to avoid deletion
            mpPropertySetMapper->acquire();
        }

        // construct PresPagePropsMapper
        mpPresPagePropsMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLSDPresPageProps, aFactoryRef);
        if(mpPresPagePropsMapper)
        {
            // set lock to avoid deletion
            mpPresPagePropsMapper->acquire();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

XMLShapeImportHelper::~XMLShapeImportHelper()
{
    // cleanup factory, decrease refcount. Should lead to destruction.
    if(mpSdPropHdlFactory)
    {
        mpSdPropHdlFactory->release();
        mpSdPropHdlFactory = 0L;
    }

    // cleanup mapper, decrease refcount. Should lead to destruction.
    if(mpPropertySetMapper)
    {
        mpPropertySetMapper->release();
        mpPropertySetMapper = 0L;
    }

    // cleanup presPage mapper, decrease refcount. Should lead to destruction.
    if(mpPresPagePropsMapper)
    {
        mpPresPagePropsMapper->release();
        mpPresPagePropsMapper = 0L;
    }

    if(mpGroupShapeElemTokenMap) delete mpGroupShapeElemTokenMap;
    if(mpShapeAttrTokenMap) delete mpShapeAttrTokenMap;
    if(mpRectShapeAttrTokenMap) delete mpRectShapeAttrTokenMap;
    if(mpLineShapeAttrTokenMap) delete mpLineShapeAttrTokenMap;
    if(mpEllipseShapeAttrTokenMap) delete mpEllipseShapeAttrTokenMap;
    if(mpPolygonShapeAttrTokenMap) delete mpPolygonShapeAttrTokenMap;
    if(mpPathShapeAttrTokenMap) delete mpPathShapeAttrTokenMap;
    if(mpTextBoxShapeAttrTokenMap) delete mpTextBoxShapeAttrTokenMap;
    if(mpControlShapeAttrTokenMap) delete mpControlShapeAttrTokenMap;
    if(mpPageShapeAttrTokenMap) delete mpPageShapeAttrTokenMap;
    if(mpGraphicObjectShapeAttrTokenMap) delete mpGraphicObjectShapeAttrTokenMap;

    // Styles or AutoStyles context?
    if(mpStylesContext)
        mpStylesContext->ReleaseRef();
    if(mpAutoStylesContext)
        mpAutoStylesContext->ReleaseRef();
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aGroupShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           sXML_g,             XML_TOK_GROUP_GROUP         },
    { XML_NAMESPACE_DRAW,           sXML_rect,          XML_TOK_GROUP_RECT          },
    { XML_NAMESPACE_DRAW,           sXML_line,          XML_TOK_GROUP_LINE          },
    { XML_NAMESPACE_DRAW,           sXML_circle,        XML_TOK_GROUP_CIRCLE        },
    { XML_NAMESPACE_DRAW,           sXML_ellipse,       XML_TOK_GROUP_ELLIPSE       },
    { XML_NAMESPACE_DRAW,           sXML_polygon,       XML_TOK_GROUP_POLYGON       },
    { XML_NAMESPACE_DRAW,           sXML_polyline,      XML_TOK_GROUP_POLYLINE      },
    { XML_NAMESPACE_DRAW,           sXML_path,          XML_TOK_GROUP_PATH          },
    { XML_NAMESPACE_TEXT,           sXML_text_box,      XML_TOK_GROUP_TEXT_BOX      },

    { XML_NAMESPACE_DRAW,           sXML_control,       XML_TOK_GROUP_CONTROL       },
    { XML_NAMESPACE_DRAW,           sXML_connector,     XML_TOK_GROUP_CONNECTOR     },
    { XML_NAMESPACE_DRAW,           sXML_measure,       XML_TOK_GROUP_MEASURE       },
    { XML_NAMESPACE_DRAW,           sXML_page_thumbnail,XML_TOK_GROUP_PAGE          },
    { XML_NAMESPACE_DRAW,           sXML_caption,       XML_TOK_GROUP_CAPTION       },

    { XML_NAMESPACE_CHART,          sXML_chart,         XML_TOK_GROUP_CHART         },
    { XML_NAMESPACE_OFFICE,         sXML_image,         XML_TOK_GROUP_IMAGE         },
//  { XML_NAMESPACE_DRAW,           sXML_3DCube,        XML_TOK_GROUP_3D_CUBE       },
//  { XML_NAMESPACE_DRAW,           sXML_3DSphere,      XML_TOK_GROUP_3D_SPHERE     },
//  { XML_NAMESPACE_DRAW,           sXML_3DLathe,       XML_TOK_GROUP_3D_LATHE      },
//  { XML_NAMESPACE_DRAW,           sXML_3DExtrude,     XML_TOK_GROUP_3D_EXTRUDE    },
//  { XML_NAMESPACE_DRAW,           sXML_3DPolygon,     XML_TOK_GROUP_3D_POLYGON    },

    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetGroupShapeElemTokenMap()
{
    if(!mpGroupShapeElemTokenMap)
        mpGroupShapeElemTokenMap = new SvXMLTokenMap(aGroupShapeElemTokenMap);
    return *mpGroupShapeElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           sXML_style_name,        XML_TOK_SHAPE_DRAWSTYLE_NAME_GRAPHICS       },
    { XML_NAMESPACE_PRESENTATION,   sXML_class,             XML_TOK_SHAPE_PRESENTATION_CLASS            },
    { XML_NAMESPACE_PRESENTATION,   sXML_style_name,        XML_TOK_SHAPE_DRAWSTYLE_NAME_PRESENTATION   },
    { XML_NAMESPACE_SVG,            sXML_transform,         XML_TOK_SHAPE_TRANSFORM                     },
    { XML_NAMESPACE_PRESENTATION,   sXML_placeholder,       XML_TOK_SHAPE_IS_PLACEHOLDER                },
    { XML_NAMESPACE_PRESENTATION,   sXML_user_transformed,  XML_TOK_SHAPE_IS_USER_TRANSFORMED           },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetShapeAttrTokenMap()
{
    if(!mpShapeAttrTokenMap)
        mpShapeAttrTokenMap = new SvXMLTokenMap(aShapeAttrTokenMap);
    return *mpShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aRectShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_RECTSHAPE_X                 },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_RECTSHAPE_Y                 },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_RECTSHAPE_WIDTH             },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_RECTSHAPE_HEIGHT            },
    { XML_NAMESPACE_DRAW,   sXML_corner_radius,     XML_TOK_RECTSHAPE_CORNER_RADIUS     },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetRectShapeAttrTokenMap()
{
    if(!mpRectShapeAttrTokenMap)
        mpRectShapeAttrTokenMap = new SvXMLTokenMap(aRectShapeAttrTokenMap);
    return *mpRectShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aLineShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x1,                XML_TOK_LINESHAPE_X1                },
    { XML_NAMESPACE_SVG,    sXML_y1,                XML_TOK_LINESHAPE_Y1                },
    { XML_NAMESPACE_SVG,    sXML_x2,                XML_TOK_LINESHAPE_X2                },
    { XML_NAMESPACE_SVG,    sXML_y2,                XML_TOK_LINESHAPE_Y2                },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetLineShapeAttrTokenMap()
{
    if(!mpLineShapeAttrTokenMap)
        mpLineShapeAttrTokenMap = new SvXMLTokenMap(aLineShapeAttrTokenMap);
    return *mpLineShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aEllipseShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_rx,                XML_TOK_ELLIPSESHAPE_RX             },
    { XML_NAMESPACE_SVG,    sXML_ry,                XML_TOK_ELLIPSESHAPE_RY             },
    { XML_NAMESPACE_SVG,    sXML_cx,                XML_TOK_ELLIPSESHAPE_CX             },
    { XML_NAMESPACE_SVG,    sXML_cy,                XML_TOK_ELLIPSESHAPE_CY             },
    { XML_NAMESPACE_SVG,    sXML_r,                 XML_TOK_ELLIPSESHAPE_R              },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetEllipseShapeAttrTokenMap()
{
    if(!mpEllipseShapeAttrTokenMap)
        mpEllipseShapeAttrTokenMap = new SvXMLTokenMap(aEllipseShapeAttrTokenMap);
    return *mpEllipseShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aPolygonShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_POLYGONSHAPE_X              },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_POLYGONSHAPE_Y              },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_POLYGONSHAPE_WIDTH          },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_POLYGONSHAPE_HEIGHT         },
    { XML_NAMESPACE_SVG,    sXML_viewBox,           XML_TOK_POLYGONSHAPE_VIEWBOX        },
    { XML_NAMESPACE_SVG,    sXML_points,            XML_TOK_POLYGONSHAPE_POINTS         },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetPolygonShapeAttrTokenMap()
{
    if(!mpPolygonShapeAttrTokenMap)
        mpPolygonShapeAttrTokenMap = new SvXMLTokenMap(aPolygonShapeAttrTokenMap);
    return *mpPolygonShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aPathShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_PATHSHAPE_X                 },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_PATHSHAPE_Y                 },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_PATHSHAPE_WIDTH             },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_PATHSHAPE_HEIGHT            },
    { XML_NAMESPACE_SVG,    sXML_viewBox,           XML_TOK_PATHSHAPE_VIEWBOX           },
    { XML_NAMESPACE_SVG,    sXML_d,                 XML_TOK_PATHSHAPE_D                 },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetPathShapeAttrTokenMap()
{
    if(!mpPathShapeAttrTokenMap)
        mpPathShapeAttrTokenMap = new SvXMLTokenMap(aPathShapeAttrTokenMap);
    return *mpPathShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aTextBoxShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_TEXTBOXSHAPE_X              },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_TEXTBOXSHAPE_Y              },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_TEXTBOXSHAPE_WIDTH          },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_TEXTBOXSHAPE_HEIGHT         },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetTextBoxShapeAttrTokenMap()
{
    if(!mpTextBoxShapeAttrTokenMap)
        mpTextBoxShapeAttrTokenMap = new SvXMLTokenMap(aTextBoxShapeAttrTokenMap);
    return *mpTextBoxShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aControlShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_CONTROLSHAPE_X              },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_CONTROLSHAPE_Y              },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_CONTROLSHAPE_WIDTH          },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_CONTROLSHAPE_HEIGHT         },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetControlShapeAttrTokenMap()
{
    if(!mpControlShapeAttrTokenMap)
        mpControlShapeAttrTokenMap = new SvXMLTokenMap(aControlShapeAttrTokenMap);
    return *mpControlShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aPageShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_PAGESHAPE_X             },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_PAGESHAPE_Y             },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_PAGESHAPE_WIDTH         },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_PAGESHAPE_HEIGHT        },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetPageShapeAttrTokenMap()
{
    if(!mpPageShapeAttrTokenMap)
        mpPageShapeAttrTokenMap = new SvXMLTokenMap(aPageShapeAttrTokenMap);
    return *mpPageShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aGraphicObjectShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                 XML_TOK_GOSHAPE_X                   },
    { XML_NAMESPACE_SVG,    sXML_y,                 XML_TOK_GOSHAPE_Y                   },
    { XML_NAMESPACE_SVG,    sXML_width,             XML_TOK_GOSHAPE_WIDTH               },
    { XML_NAMESPACE_SVG,    sXML_height,            XML_TOK_GOSHAPE_HEIGHT              },
    { XML_NAMESPACE_XLINK,  sXML_href,              XML_TOK_GOSHAPE_URL                 },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetGraphicObjectShapeAttrTokenMap()
{
    if(!mpGraphicObjectShapeAttrTokenMap)
        mpGraphicObjectShapeAttrTokenMap = new SvXMLTokenMap(aGraphicObjectShapeAttrTokenMap);
    return *mpGraphicObjectShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeImportHelper::SetStylesContext(SvXMLStylesContext* pNew)
{
    mpStylesContext = pNew;
    mpStylesContext->AddRef();
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeImportHelper::SetAutoStylesContext(SvXMLStylesContext* pNew)
{
    mpAutoStylesContext = pNew;
    mpAutoStylesContext->AddRef();
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* XMLShapeImportHelper::CreateGroupChildContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
{
    SvXMLImportContext *pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetGroupShapeElemTokenMap();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    sal_Bool bIsPlaceholder(FALSE);
    OUString aPresentationObjectClass;

    for(sal_Int16 a(0); a < nAttrCount; a++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex(a);
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);

        if(XML_NAMESPACE_PRESENTATION == nPrefix)
        {
            if(aLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_placeholder))))
            {
                bIsPlaceholder = TRUE;
            }
            else if(aLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_class))))
            {
                aPresentationObjectClass = xAttrList->getValueByIndex(a);
            }
        }
    }

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_GROUP_GROUP:
        {
            // draw:g inside group context (RECURSIVE)
            // create new group shape and add it to rShapes, use it
            // as base for the new group import
            uno::Reference< lang::XMultiServiceFactory > xServiceFact(mxModel, uno::UNO_QUERY);
            if(xServiceFact.is())
            {
                uno::Reference< drawing::XShape > xShape(
                    xServiceFact->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GroupShape"))),
                    uno::UNO_QUERY);
                if(xShape.is())
                {
                    addShape( xShape, xAttrList, rShapes );

                    uno::Reference< drawing::XShapes > xNewShapes(xShape, uno::UNO_QUERY);
                    if(xNewShapes.is())
                    {
                        pContext = new SdXMLGroupShapeContext( rImport, nPrefix, rLocalName, xNewShapes);
                    }
                }
            }
            break;
        }
        case XML_TOK_GROUP_RECT:
        {
            // draw:rect inside group context
            pContext = new SdXMLRectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_LINE:
        {
            // draw:line inside group context
            pContext = new SdXMLLineShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CIRCLE:
        case XML_TOK_GROUP_ELLIPSE:
        {
            // draw:circle or draw:ellipse inside group context
            pContext = new SdXMLEllipseShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_POLYGON:
        case XML_TOK_GROUP_POLYLINE:
        {
            // draw:polygon or draw:polyline inside group context
            pContext = new SdXMLPolygonShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes,
                rTokenMap.Get(nPrefix, rLocalName) == XML_TOK_GROUP_POLYGON ? TRUE : FALSE );
            break;
        }
        case XML_TOK_GROUP_PATH:
        {
            // draw:path inside group context
            pContext = new SdXMLPathShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
            break;
        }
        case XML_TOK_GROUP_TEXT_BOX:
        {
            // text:text-box inside group context
            pContext = new SdXMLTextBoxShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CONTROL:
        {
            // draw:control inside group context
            pContext = new SdXMLControlShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CONNECTOR:
        {
            // draw:connector inside group context
            pContext = new SdXMLConnectorShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_MEASURE:
        {
            // draw:measure inside group context
            pContext = new SdXMLMeasureShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_PAGE:
        {
            // draw:page inside group context
            pContext = new SdXMLPageShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CAPTION:
        {
            // draw:caption inside group context
            pContext = new SdXMLCaptionShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CHART:
        {
            // chart:chart inside group context
            pContext = new SdXMLChartShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_IMAGE:
        {
            // office:image inside group context
            pContext = new SdXMLGraphicObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }

        // add other shapes here...
    }

    return pContext;
}


void XMLShapeImportHelper::addShape( uno::Reference< drawing::XShape >& rShape, const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Reference< drawing::XShapes >& rShapes)
    throw()
{
    if( rShape.is() && rShapes.is() )
    {
        // add new shape to parent
        rShapes->add( rShape );
    }
}
