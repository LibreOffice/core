/*************************************************************************
 *
 *  $RCSfile: shapeimport.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:25:22 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#include <list>

#ifndef _XMLOFF_SHAPEIMPORT_HXX
#include "shapeimport.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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

#ifndef _XIMP3DSCENE_HXX
#include "ximp3dscene.hxx"
#endif

#ifndef _XIMP3DOBJECT_HXX
#include "ximp3dobject.hxx"
#endif

#ifndef _XIMPGROUP_HXX
#include "ximpgrp.hxx"
#endif

#include <map>
#include <vector>

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

struct ltint32
{
  bool operator()(const sal_Int32 p, sal_Int32 q) const
  {
    return p < q;
  }
};

typedef std::map<sal_Int32,com::sun::star::uno::Reference< com::sun::star::drawing::XShape >,ltint32> IdShapeMap;

struct ConnectionHint
{
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxConnector;
    sal_Bool  bStart;
    sal_Int32 nDestShapeId;
    sal_Int32 nDestGlueId;
};

struct XShapeCompareHelper
{
  bool operator()(com::sun::star::uno::Reference < com::sun::star::drawing::XShape > x1,
                  com::sun::star::uno::Reference < com::sun::star::drawing::XShape > x2 ) const
  {
    return x1.get() < x2.get();
  }
};

/** this map store all glue point id mappings for shapes that had user defined glue points. This
    is needed because on insertion the glue points will get a new and unique id */
typedef std::map<sal_Int32,sal_Int32,ltint32> GluePointIdMap;
typedef std::map< com::sun::star::uno::Reference < com::sun::star::drawing::XShape >, GluePointIdMap, XShapeCompareHelper > ShapeGluePointsMap;

/** this struct is created for each startPage() call and stores information that is needed during
    import of shapes for one page. Since pages could be nested ( notes pages inside impress ) there
    is a pointer so one can build up a stack of this structs */
struct XMLShapeImportPageContextImpl
{
    ShapeGluePointsMap      maShapeGluePointsMap;

    uno::Reference < drawing::XShapes > mxShapes;

    struct XMLShapeImportPageContextImpl* mpNext;
};

/** this class is to enable adding members to the XMLShapeImportHelper without getting incompatible */
struct XMLShapeImportHelperImpl
{
    // context for sorting shapes
    ShapeSortContext*           mpSortContext;

    IdShapeMap                  maShapeIds;

    std::vector<ConnectionHint> maConnections;

    // #88546# possibility to swich progress bar handling on/off
    sal_Bool                    mbHandleProgressBar;

    // stores the capability of the current model to create presentation shapes
    sal_Bool                    mbIsPresentationShapesSupported;
};

//////////////////////////////////////////////////////////////////////////////

XMLShapeImportHelper::XMLShapeImportHelper(
        SvXMLImport& rImporter,
        const uno::Reference< frame::XModel>& rModel,
        SvXMLImportPropertyMapper *pExtMapper )
:   mxModel(rModel),
    mrImporter( rImporter ),
    mpPageContext(NULL),

    mpPropertySetMapper(0L),
    mpPresPagePropsMapper(0L),
    mpStylesContext(0L),
    mpAutoStylesContext(0L),
    mpGroupShapeElemTokenMap(0L),
    mpFrameShapeElemTokenMap(0L),
    mp3DSceneShapeElemTokenMap(0L),
    mp3DObjectAttrTokenMap(0L),
    mp3DPolygonBasedAttrTokenMap(0L),
    mp3DCubeObjectAttrTokenMap(0L),
    mp3DSphereObjectAttrTokenMap(0L),
    mp3DSceneShapeAttrTokenMap(0L),
    mp3DLightAttrTokenMap(0L),
    mpPathShapeAttrTokenMap(0L),
    mpPolygonShapeAttrTokenMap(0L),
/*
    mpShapeAttrTokenMap(0L),
    mpRectShapeAttrTokenMap(0L),
    mpLineShapeAttrTokenMap(0L),
    mpEllipseShapeAttrTokenMap(0L),
    mpTextBoxShapeAttrTokenMap(0L),
    mpControlShapeAttrTokenMap(0L),
    mpPageShapeAttrTokenMap(0L),
    mpGraphicObjectShapeAttrTokenMap(0L),
*/
    msStartShape(RTL_CONSTASCII_USTRINGPARAM("StartShape")),
    msEndShape(RTL_CONSTASCII_USTRINGPARAM("EndShape")),
    msStartGluePointIndex(RTL_CONSTASCII_USTRINGPARAM("StartGluePointIndex")),
    msEndGluePointIndex(RTL_CONSTASCII_USTRINGPARAM("EndGluePointIndex"))
{
    mpImpl = new XMLShapeImportHelperImpl();
    mpImpl->mpSortContext = 0;

    // #88546# init to FALSE
    mpImpl->mbHandleProgressBar = sal_False;

    mpSdPropHdlFactory = new XMLSdPropHdlFactory( rModel, NULL );

    // set lock to avoid deletion
    mpSdPropHdlFactory->acquire();

    // construct PropertySetMapper
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper(mpSdPropHdlFactory);
    mpPropertySetMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );
    // set lock to avoid deletion
    mpPropertySetMapper->acquire();

    if( pExtMapper )
    {
        UniReference < SvXMLImportPropertyMapper > xExtMapper( pExtMapper );
        mpPropertySetMapper->ChainImportMapper( xExtMapper );
    }

    // chain text attributes
    mpPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(rImporter));

/*
    // chain form attributes
    const UniReference< SvXMLImportPropertyMapper> xFormMapper( rImporter.GetFormImport()->getStylePropertyMapper().getBodyPtr() );
    mpPropertySetMapper->ChainImportMapper(xFormMapper);
*/

    // construct PresPagePropsMapper
    xMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLSDPresPageProps, mpSdPropHdlFactory);
    mpPresPagePropsMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );
    if(mpPresPagePropsMapper)
    {
        // set lock to avoid deletion
        mpPresPagePropsMapper->acquire();
    }

    uno::Reference< lang::XServiceInfo > xInfo( rImporter.GetModel(), uno::UNO_QUERY );
    const OUString aSName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument") );
    mpImpl->mbIsPresentationShapesSupported = xInfo.is() && xInfo->supportsService( aSName );
}

//////////////////////////////////////////////////////////////////////////////

XMLShapeImportHelper::~XMLShapeImportHelper()
{
    DBG_ASSERT( mpImpl->maConnections.empty(), "XMLShapeImportHelper::restoreConnections() was not called!" );

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
    if(mpFrameShapeElemTokenMap) delete mpFrameShapeElemTokenMap;
/*
    if(mpShapeAttrTokenMap) delete mpShapeAttrTokenMap;
    if(mpRectShapeAttrTokenMap) delete mpRectShapeAttrTokenMap;
    if(mpLineShapeAttrTokenMap) delete mpLineShapeAttrTokenMap;
    if(mpEllipseShapeAttrTokenMap) delete mpEllipseShapeAttrTokenMap;
    if(mpTextBoxShapeAttrTokenMap) delete mpTextBoxShapeAttrTokenMap;
    if(mpControlShapeAttrTokenMap) delete mpControlShapeAttrTokenMap;
    if(mpPageShapeAttrTokenMap) delete mpPageShapeAttrTokenMap;
    if(mpGraphicObjectShapeAttrTokenMap) delete mpGraphicObjectShapeAttrTokenMap;
*/
    if(mpPolygonShapeAttrTokenMap) delete mpPolygonShapeAttrTokenMap;
    if(mpPathShapeAttrTokenMap) delete mpPathShapeAttrTokenMap;
    if(mp3DSceneShapeElemTokenMap) delete mp3DSceneShapeElemTokenMap;
    if(mp3DObjectAttrTokenMap) delete mp3DObjectAttrTokenMap;
    if(mp3DPolygonBasedAttrTokenMap) delete mp3DPolygonBasedAttrTokenMap;
    if(mp3DCubeObjectAttrTokenMap) delete mp3DCubeObjectAttrTokenMap;
    if(mp3DSphereObjectAttrTokenMap) delete mp3DSphereObjectAttrTokenMap;
    if(mp3DSceneShapeAttrTokenMap) delete mp3DSceneShapeAttrTokenMap;
    if(mp3DLightAttrTokenMap) delete mp3DLightAttrTokenMap;

    // Styles or AutoStyles context?
    if(mpStylesContext)
    {
        mpStylesContext->Clear();
        mpStylesContext->ReleaseRef();
    }

    if(mpAutoStylesContext)
    {
        mpAutoStylesContext->Clear();
        mpAutoStylesContext->ReleaseRef();
    }

    delete mpImpl;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aGroupShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_G,              XML_TOK_GROUP_GROUP         },
    { XML_NAMESPACE_DRAW,           XML_RECT,           XML_TOK_GROUP_RECT          },
    { XML_NAMESPACE_DRAW,           XML_LINE,           XML_TOK_GROUP_LINE          },
    { XML_NAMESPACE_DRAW,           XML_CIRCLE,         XML_TOK_GROUP_CIRCLE        },
    { XML_NAMESPACE_DRAW,           XML_ELLIPSE,        XML_TOK_GROUP_ELLIPSE       },
    { XML_NAMESPACE_DRAW,           XML_POLYGON,        XML_TOK_GROUP_POLYGON       },
    { XML_NAMESPACE_DRAW,           XML_POLYLINE,       XML_TOK_GROUP_POLYLINE      },
    { XML_NAMESPACE_DRAW,           XML_PATH,           XML_TOK_GROUP_PATH          },

    { XML_NAMESPACE_DRAW,           XML_CONTROL,        XML_TOK_GROUP_CONTROL       },
    { XML_NAMESPACE_DRAW,           XML_CONNECTOR,      XML_TOK_GROUP_CONNECTOR     },
    { XML_NAMESPACE_DRAW,           XML_MEASURE,        XML_TOK_GROUP_MEASURE       },
    { XML_NAMESPACE_DRAW,           XML_PAGE_THUMBNAIL, XML_TOK_GROUP_PAGE          },
    { XML_NAMESPACE_DRAW,           XML_CAPTION,        XML_TOK_GROUP_CAPTION       },

    { XML_NAMESPACE_CHART,          XML_CHART,          XML_TOK_GROUP_CHART         },
    { XML_NAMESPACE_DR3D,           XML_SCENE,          XML_TOK_GROUP_3DSCENE       },

    { XML_NAMESPACE_DRAW,           XML_FRAME,          XML_TOK_GROUP_FRAME         },

    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aFrameShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_TEXT_BOX,       XML_TOK_FRAME_TEXT_BOX      },
    { XML_NAMESPACE_DRAW,           XML_IMAGE,          XML_TOK_FRAME_IMAGE         },
    { XML_NAMESPACE_DRAW,           XML_OBJECT,         XML_TOK_FRAME_OBJECT        },
    { XML_NAMESPACE_DRAW,           XML_OBJECT_OLE,     XML_TOK_FRAME_OBJECT_OLE    },
    { XML_NAMESPACE_DRAW,           XML_PLUGIN,         XML_TOK_FRAME_PLUGIN        },
    { XML_NAMESPACE_DRAW,           XML_FLOATING_FRAME, XML_TOK_FRAME_FLOATING_FRAME},
    { XML_NAMESPACE_DRAW,           XML_APPLET,         XML_TOK_FRAME_APPLET        },

    { XML_NAMESPACE_DRAW,           XML_CUSTOM_SHAPE,       XML_TOK_GROUP_CUSTOM_SHAPE  },

    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetGroupShapeElemTokenMap()
{
    if(!mpGroupShapeElemTokenMap)
        mpGroupShapeElemTokenMap = new SvXMLTokenMap(aGroupShapeElemTokenMap);
    return *mpGroupShapeElemTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::GetFrameShapeElemTokenMap()
{
    if(!mpFrameShapeElemTokenMap)
        mpFrameShapeElemTokenMap = new SvXMLTokenMap(aFrameShapeElemTokenMap);
    return *mpFrameShapeElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DSceneShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_SCENE,      XML_TOK_3DSCENE_3DSCENE     },
    { XML_NAMESPACE_DR3D,           XML_CUBE,       XML_TOK_3DSCENE_3DCUBE      },
    { XML_NAMESPACE_DR3D,           XML_SPHERE,     XML_TOK_3DSCENE_3DSPHERE    },
    { XML_NAMESPACE_DR3D,           XML_ROTATE,     XML_TOK_3DSCENE_3DLATHE     },
    { XML_NAMESPACE_DR3D,           XML_EXTRUDE,    XML_TOK_3DSCENE_3DEXTRUDE   },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DSceneShapeElemTokenMap()
{
    if(!mp3DSceneShapeElemTokenMap)
        mp3DSceneShapeElemTokenMap = new SvXMLTokenMap(a3DSceneShapeElemTokenMap);
    return *mp3DSceneShapeElemTokenMap;
}

//////////////////////////////////////////////////////////////////////////////
/*
static __FAR_DATA SvXMLTokenMapEntry aShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_NAME,               XML_TOK_SHAPE_NAME                          },
    { XML_NAMESPACE_DRAW,           XML_STYLE_NAME,         XML_TOK_SHAPE_DRAWSTYLE_NAME_GRAPHICS       },
    { XML_NAMESPACE_PRESENTATION,   XML_CLASS,              XML_TOK_SHAPE_PRESENTATION_CLASS            },
    { XML_NAMESPACE_PRESENTATION,   XML_STYLE_NAME,         XML_TOK_SHAPE_DRAWSTYLE_NAME_PRESENTATION   },
    { XML_NAMESPACE_SVG,            XML_TRANSFORM,          XML_TOK_SHAPE_TRANSFORM                     },
    { XML_NAMESPACE_PRESENTATION,   XML_PLACEHOLDER,        XML_TOK_SHAPE_IS_PLACEHOLDER                },
    { XML_NAMESPACE_PRESENTATION,   XML_USER_TRANSFORMED,   XML_TOK_SHAPE_IS_USER_TRANSFORMED           },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetShapeAttrTokenMap()
{
    if(!mpShapeAttrTokenMap)
        mpShapeAttrTokenMap = new SvXMLTokenMap(aShapeAttrTokenMap);
    return *mpShapeAttrTokenMap;
}
*/
//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_STYLE_NAME,         XML_TOK_3DOBJECT_DRAWSTYLE_NAME     },
    { XML_NAMESPACE_DR3D,           XML_TRANSFORM,          XML_TOK_3DOBJECT_TRANSFORM          },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DObjectAttrTokenMap()
{
    if(!mp3DObjectAttrTokenMap)
        mp3DObjectAttrTokenMap = new SvXMLTokenMap(a3DObjectAttrTokenMap);
    return *mp3DObjectAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DPolygonBasedAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,            XML_VIEWBOX,            XML_TOK_3DPOLYGONBASED_VIEWBOX      },
    { XML_NAMESPACE_SVG,            XML_D,                  XML_TOK_3DPOLYGONBASED_D            },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DPolygonBasedAttrTokenMap()
{
    if(!mp3DPolygonBasedAttrTokenMap)
        mp3DPolygonBasedAttrTokenMap = new SvXMLTokenMap(a3DPolygonBasedAttrTokenMap);
    return *mp3DPolygonBasedAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DCubeObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_MIN_EDGE,           XML_TOK_3DCUBEOBJ_MINEDGE   },
    { XML_NAMESPACE_DR3D,           XML_MAX_EDGE,           XML_TOK_3DCUBEOBJ_MAXEDGE   },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DCubeObjectAttrTokenMap()
{
    if(!mp3DCubeObjectAttrTokenMap)
        mp3DCubeObjectAttrTokenMap = new SvXMLTokenMap(a3DCubeObjectAttrTokenMap);
    return *mp3DCubeObjectAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DSphereObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_CENTER,             XML_TOK_3DSPHEREOBJ_CENTER  },
    { XML_NAMESPACE_DR3D,           XML_SIZE,               XML_TOK_3DSPHEREOBJ_SIZE    },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DSphereObjectAttrTokenMap()
{
    if(!mp3DSphereObjectAttrTokenMap)
        mp3DSphereObjectAttrTokenMap = new SvXMLTokenMap(a3DSphereObjectAttrTokenMap);
    return *mp3DSphereObjectAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////
/*
static __FAR_DATA SvXMLTokenMapEntry aRectShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    XML_X,                  XML_TOK_RECTSHAPE_X                 },
    { XML_NAMESPACE_SVG,    XML_Y,                  XML_TOK_RECTSHAPE_Y                 },
    { XML_NAMESPACE_SVG,    XML_WIDTH,              XML_TOK_RECTSHAPE_WIDTH             },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,             XML_TOK_RECTSHAPE_HEIGHT            },
    { XML_NAMESPACE_DRAW,   XML_CORNER_RADIUS,      XML_TOK_RECTSHAPE_CORNER_RADIUS     },
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
    { XML_NAMESPACE_SVG,    XML_X1,             XML_TOK_LINESHAPE_X1                },
    { XML_NAMESPACE_SVG,    XML_Y1,             XML_TOK_LINESHAPE_Y1                },
    { XML_NAMESPACE_SVG,    XML_X2,             XML_TOK_LINESHAPE_X2                },
    { XML_NAMESPACE_SVG,    XML_Y2,             XML_TOK_LINESHAPE_Y2                },
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
    { XML_NAMESPACE_SVG,    XML_RX,             XML_TOK_ELLIPSESHAPE_RX             },
    { XML_NAMESPACE_SVG,    XML_RY,             XML_TOK_ELLIPSESHAPE_RY             },
    { XML_NAMESPACE_SVG,    XML_CX,             XML_TOK_ELLIPSESHAPE_CX             },
    { XML_NAMESPACE_SVG,    XML_CY,             XML_TOK_ELLIPSESHAPE_CY             },
    { XML_NAMESPACE_SVG,    XML_R,              XML_TOK_ELLIPSESHAPE_R              },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetEllipseShapeAttrTokenMap()
{
    if(!mpEllipseShapeAttrTokenMap)
        mpEllipseShapeAttrTokenMap = new SvXMLTokenMap(aEllipseShapeAttrTokenMap);
    return *mpEllipseShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////
*/
static __FAR_DATA SvXMLTokenMapEntry aPolygonShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    XML_VIEWBOX,            XML_TOK_POLYGONSHAPE_VIEWBOX        },
    { XML_NAMESPACE_DRAW,   XML_POINTS,             XML_TOK_POLYGONSHAPE_POINTS         },
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
    { XML_NAMESPACE_SVG,    XML_VIEWBOX,            XML_TOK_PATHSHAPE_VIEWBOX           },
    { XML_NAMESPACE_SVG,    XML_D,                  XML_TOK_PATHSHAPE_D                 },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetPathShapeAttrTokenMap()
{
    if(!mpPathShapeAttrTokenMap)
        mpPathShapeAttrTokenMap = new SvXMLTokenMap(aPathShapeAttrTokenMap);
    return *mpPathShapeAttrTokenMap;
}
/*
//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry aTextBoxShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    XML_X,                  XML_TOK_TEXTBOXSHAPE_X              },
    { XML_NAMESPACE_SVG,    XML_Y,                  XML_TOK_TEXTBOXSHAPE_Y              },
    { XML_NAMESPACE_SVG,    XML_WIDTH,              XML_TOK_TEXTBOXSHAPE_WIDTH          },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,             XML_TOK_TEXTBOXSHAPE_HEIGHT         },
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
    { XML_NAMESPACE_SVG,    XML_X,                  XML_TOK_CONTROLSHAPE_X              },
    { XML_NAMESPACE_SVG,    XML_Y,                  XML_TOK_CONTROLSHAPE_Y              },
    { XML_NAMESPACE_SVG,    XML_WIDTH,              XML_TOK_CONTROLSHAPE_WIDTH          },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,             XML_TOK_CONTROLSHAPE_HEIGHT         },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetControlShapeAttrTokenMap()
{
    if(!mpControlShapeAttrTokenMap)
        mpControlShapeAttrTokenMap = new SvXMLTokenMap(aControlShapeAttrTokenMap);
    return *mpControlShapeAttrTokenMap;
}
*/
//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DSceneShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,   XML_TRANSFORM,          XML_TOK_3DSCENESHAPE_TRANSFORM      },
    { XML_NAMESPACE_DR3D,   XML_VRP,                XML_TOK_3DSCENESHAPE_VRP            },
    { XML_NAMESPACE_DR3D,   XML_VPN,                XML_TOK_3DSCENESHAPE_VPN            },
    { XML_NAMESPACE_DR3D,   XML_VUP,                XML_TOK_3DSCENESHAPE_VUP            },
    { XML_NAMESPACE_DR3D,   XML_PROJECTION,         XML_TOK_3DSCENESHAPE_PROJECTION     },
    { XML_NAMESPACE_DR3D,   XML_DISTANCE,           XML_TOK_3DSCENESHAPE_DISTANCE       },
    { XML_NAMESPACE_DR3D,   XML_FOCAL_LENGTH,       XML_TOK_3DSCENESHAPE_FOCAL_LENGTH   },
    { XML_NAMESPACE_DR3D,   XML_SHADOW_SLANT,       XML_TOK_3DSCENESHAPE_SHADOW_SLANT   },
    { XML_NAMESPACE_DR3D,   XML_SHADE_MODE,         XML_TOK_3DSCENESHAPE_SHADE_MODE     },
    { XML_NAMESPACE_DR3D,   XML_AMBIENT_COLOR,      XML_TOK_3DSCENESHAPE_AMBIENT_COLOR  },
    { XML_NAMESPACE_DR3D,   XML_LIGHTING_MODE,      XML_TOK_3DSCENESHAPE_LIGHTING_MODE  },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DSceneShapeAttrTokenMap()
{
    if(!mp3DSceneShapeAttrTokenMap)
        mp3DSceneShapeAttrTokenMap = new SvXMLTokenMap(a3DSceneShapeAttrTokenMap);
    return *mp3DSceneShapeAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////

static __FAR_DATA SvXMLTokenMapEntry a3DLightAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,   XML_DIFFUSE_COLOR,      XML_TOK_3DLIGHT_DIFFUSE_COLOR       },
    { XML_NAMESPACE_DR3D,   XML_DIRECTION,          XML_TOK_3DLIGHT_DIRECTION           },
    { XML_NAMESPACE_DR3D,   XML_ENABLED,            XML_TOK_3DLIGHT_ENABLED             },
    { XML_NAMESPACE_DR3D,   XML_SPECULAR,           XML_TOK_3DLIGHT_SPECULAR            },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::Get3DLightAttrTokenMap()
{
    if(!mp3DLightAttrTokenMap)
        mp3DLightAttrTokenMap = new SvXMLTokenMap(a3DLightAttrTokenMap);
    return *mp3DLightAttrTokenMap;
}

//////////////////////////////////////////////////////////////////////////////
/*
static __FAR_DATA SvXMLTokenMapEntry aPageShapeAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    XML_X,                  XML_TOK_PAGESHAPE_X             },
    { XML_NAMESPACE_SVG,    XML_Y,                  XML_TOK_PAGESHAPE_Y             },
    { XML_NAMESPACE_SVG,    XML_WIDTH,              XML_TOK_PAGESHAPE_WIDTH         },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,             XML_TOK_PAGESHAPE_HEIGHT        },
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
    { XML_NAMESPACE_SVG,    XML_X,                  XML_TOK_GOSHAPE_X                   },
    { XML_NAMESPACE_SVG,    XML_Y,                  XML_TOK_GOSHAPE_Y                   },
    { XML_NAMESPACE_SVG,    XML_WIDTH,              XML_TOK_GOSHAPE_WIDTH               },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,             XML_TOK_GOSHAPE_HEIGHT              },
    { XML_NAMESPACE_XLINK,  XML_HREF,               XML_TOK_GOSHAPE_URL                 },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLShapeImportHelper::GetGraphicObjectShapeAttrTokenMap()
{
    if(!mpGraphicObjectShapeAttrTokenMap)
        mpGraphicObjectShapeAttrTokenMap = new SvXMLTokenMap(aGraphicObjectShapeAttrTokenMap);
    return *mpGraphicObjectShapeAttrTokenMap;
}
*/
//////////////////////////////////////////////////////////////////////////////

SvXMLShapeContext* XMLShapeImportHelper::Create3DSceneChildContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
{
    SdXMLShapeContext *pContext = 0L;

    if(rShapes.is())
    {
        const SvXMLTokenMap& rTokenMap = Get3DSceneShapeElemTokenMap();
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

        for(sal_Int16 a(0); a < nAttrCount; a++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex(a);
            OUString aLocalName;
            sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
        }

        switch(rTokenMap.Get(nPrefix, rLocalName))
        {
            case XML_TOK_3DSCENE_3DSCENE:
            {
                // dr3d:3dscene inside dr3d:3dscene context
                pContext = new SdXML3DSceneShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
                break;
            }
            case XML_TOK_3DSCENE_3DCUBE:
            {
                // dr3d:3dcube inside dr3d:3dscene context
                pContext = new SdXML3DCubeObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
                break;
            }
            case XML_TOK_3DSCENE_3DSPHERE:
            {
                // dr3d:3dsphere inside dr3d:3dscene context
                pContext = new SdXML3DSphereObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
                break;
            }
            case XML_TOK_3DSCENE_3DLATHE:
            {
                // dr3d:3dlathe inside dr3d:3dscene context
                pContext = new SdXML3DLatheObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
                break;
            }
            case XML_TOK_3DSCENE_3DEXTRUDE:
            {
                // dr3d:3dextrude inside dr3d:3dscene context
                pContext = new SdXML3DExtrudeObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
                break;
            }
        }
    }

    // now parse the attribute list and call the child context for each unknown attribute
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 a(0); a < nAttrCount; a++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex(a);
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
        const OUString aValue( xAttrList->getValueByIndex(a) );

        pContext->processAttribute( nPrefix, aLocalName, aValue );
    }

    return pContext;
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

SvXMLShapeContext* XMLShapeImportHelper::CreateGroupChildContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
{
    SdXMLShapeContext *pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetGroupShapeElemTokenMap();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_GROUP_GROUP:
        {
            // draw:g inside group context (RECURSIVE)
            pContext = new SdXMLGroupShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
            break;
        }
        case XML_TOK_GROUP_3DSCENE:
        {
            // dr3d:3dscene inside group context
            pContext = new SdXML3DSceneShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes);
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
        case XML_TOK_GROUP_FRAME:
        {
            // text:text-box inside group context
            pContext = new SdXMLFrameShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
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
        // add other shapes here...
        default:
            return new SvXMLImportContext( rImport, nPrefix, rLocalName );
    }

    // now parse the attribute list and call the child context for each unknown attribute
    for(sal_Int16 a(0); a < nAttrCount; a++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex(a);
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
        const OUString aValue( xAttrList->getValueByIndex(a) );

        pContext->processAttribute( nPrefix, aLocalName, aValue );
    }

    return pContext;
}

// This method is called from SdXMLFrameContext to create children of drawe:frame
SvXMLShapeContext* XMLShapeImportHelper::CreateFrameChildContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& rAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    const uno::Reference< xml::sax::XAttributeList>& rFrameAttrList)
{
    SdXMLShapeContext *pContext = 0L;

    const SvXMLTokenMap& rTokenMap = GetFrameShapeElemTokenMap();

    SvXMLAttributeList *pAttrList = new SvXMLAttributeList( rAttrList );
    pAttrList->AppendAttributeList( rFrameAttrList );
    uno::Reference < xml::sax::XAttributeList > xAttrList = pAttrList;


    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_FRAME_TEXT_BOX:
        {
            // text:text-box inside group context
            pContext = new SdXMLTextBoxShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_FRAME_IMAGE:
        {
            // office:image inside group context
            pContext = new SdXMLGraphicObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_FRAME_OBJECT:
        case XML_TOK_FRAME_OBJECT_OLE:
        {
            // draw:object or draw:object_ole
            pContext = new SdXMLObjectShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_FRAME_PLUGIN:
        {
            // draw:plugin
            pContext = new SdXMLPluginShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_FRAME_FLOATING_FRAME:
        {
            // draw:floating-frame
            pContext = new SdXMLFloatingFrameShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_FRAME_APPLET:
        {
            // draw:applet
            pContext = new SdXMLAppletShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        case XML_TOK_GROUP_CUSTOM_SHAPE:
        {
            // draw:customshape
            pContext = new SdXMLCustomShapeContext( rImport, nPrefix, rLocalName, xAttrList, rShapes );
            break;
        }
        // add other shapes here...
        default:
            break;
    }

    if( pContext )
    {
        // now parse the attribute list and call the child context for each unknown attribute
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 a(0); a < nAttrCount; a++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex(a);
            OUString aLocalName;
            sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
            const OUString aValue( xAttrList->getValueByIndex(a) );

            pContext->processAttribute( nPrefix, aLocalName, aValue );
        }
    }

    return pContext;
}

SvXMLImportContext *XMLShapeImportHelper::CreateFrameChildContext(
    SvXMLImportContext *pThisContext,
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    SdXMLFrameShapeContext *pFrameContext = PTR_CAST( SdXMLFrameShapeContext, pThisContext );
    if( pFrameContext )
        pContext = pFrameContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}


/** this function is called whenever the implementation classes like to add this new
    shape to the given XShapes.
*/
void XMLShapeImportHelper::addShape( uno::Reference< drawing::XShape >& rShape,
                                     const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                     uno::Reference< drawing::XShapes >& rShapes)
{
    if( rShape.is() && rShapes.is() )
    {
        // add new shape to parent
        rShapes->add( rShape );
    }
}

/** this function is called whenever the implementation classes have finished importing
    a shape to the given XShapes. The shape is already inserted into its XShapes and
    all properties and styles are set.
*/
void XMLShapeImportHelper::finishShape(
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
{
}

// helper functions for z-order sorting
struct ZOrderHint
{
    sal_Int32 nIs;
    sal_Int32 nShould;

    int operator<(const ZOrderHint& rComp) const { return nShould < rComp.nShould; }
};

class ShapeSortContext
{
public:
    uno::Reference< drawing::XShapes > mxShapes;
    list<ZOrderHint>              maZOrderList;
    list<ZOrderHint>              maUnsortedList;

    sal_Int32                     mnCurrentZ;
    ShapeSortContext*             mpParentContext;
    const OUString                msZOrder;

    ShapeSortContext( uno::Reference< drawing::XShapes >& rShapes, ShapeSortContext* pParentContext = NULL );

    void moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos );
};

ShapeSortContext::ShapeSortContext( uno::Reference< drawing::XShapes >& rShapes, ShapeSortContext* pParentContext )
:   mxShapes( rShapes ), mnCurrentZ( 0 ), mpParentContext( pParentContext ),
    msZOrder(RTL_CONSTASCII_USTRINGPARAM("ZOrder"))
{
}

void ShapeSortContext::moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos )
{
    uno::Any aAny( mxShapes->getByIndex( nSourcePos ) );
    uno::Reference< beans::XPropertySet > xPropSet;
    aAny >>= xPropSet;

    if( xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName( msZOrder ) )
    {
        aAny <<= nDestPos;
        xPropSet->setPropertyValue( msZOrder, aAny );

        list<ZOrderHint>::iterator aIter = maZOrderList.begin();
        list<ZOrderHint>::iterator aEnd = maZOrderList.end();

        while( aIter != aEnd )
        {
            if( (*aIter).nIs < nSourcePos )
            {
                DBG_ASSERT( (*aIter).nIs >= nDestPos, "Shape sorting failed" );
                (*aIter).nIs++;
            }
            aIter++;
        }

        aIter = maUnsortedList.begin();
        aEnd = maUnsortedList.end();

        while( aIter != aEnd )
        {
            if( (*aIter).nIs < nSourcePos )
            {
                DBG_ASSERT( (*aIter).nIs >= nDestPos, "shape sorting failed" );
                (*aIter).nIs++;
            }
            aIter++;
        }
    }
}

void XMLShapeImportHelper::pushGroupForSorting( uno::Reference< drawing::XShapes >& rShapes )
{
    mpImpl->mpSortContext = new ShapeSortContext( rShapes, mpImpl->mpSortContext );
}

void XMLShapeImportHelper::popGroupAndSort()
{
    DBG_ASSERT( mpImpl->mpSortContext, "No context to sort!" );
    if( mpImpl->mpSortContext == NULL )
        return;

    try
    {
        list<ZOrderHint>& rZList = mpImpl->mpSortContext->maZOrderList;
        list<ZOrderHint>& rUnsortedList = mpImpl->mpSortContext->maUnsortedList;

        // sort shapes
        if( !rZList.empty() )
        {
            // only do something if we have shapes to sort

            // check if there are more shapes than inserted with ::shapeWithZIndexAdded()
            // This can happen if there where already shapes on the page before import
            // Since the writer may delete some of this shapes during import, we need
            // to do this here and not in our c'tor anymore

            // check if we have more shapes than we know of
            sal_Int32 nCount = mpImpl->mpSortContext->mxShapes->getCount();

            nCount -= rZList.size();
            nCount -= rUnsortedList.size();


            if( nCount > 0 )
            {
                // first update offsets of added shapes
                list<ZOrderHint>::iterator aIter( rZList.begin() );
                while( aIter != rZList.end() )
                    (*aIter++).nIs += nCount;

                aIter = rUnsortedList.begin();
                while( aIter != rUnsortedList.end() )
                    (*aIter++).nIs += nCount;

                // second add the already existing shapes in the unsorted list
                ZOrderHint aNewHint;

                do
                {
                    nCount--;

                    aNewHint.nIs = nCount;
                    aNewHint.nShould = -1;

                    rUnsortedList.insert(rUnsortedList.begin(), aNewHint);
                }
                while( nCount );
            }

            // sort z ordered shapes
            rZList.sort();

            // this is the current index, all shapes before that
            // index are finished
            sal_Int32 nIndex = 0;
            while( !rZList.empty() )
            {
                list<ZOrderHint>::iterator aIter( rZList.begin() );

                while( nIndex < (*aIter).nShould && !rUnsortedList.empty() )
                {
                    ZOrderHint aGapHint( *rUnsortedList.begin() );
                    rUnsortedList.pop_front();

                    mpImpl->mpSortContext->moveShape( aGapHint.nIs, nIndex++ );
                }

                if( (*aIter).nIs != nIndex )
                    mpImpl->mpSortContext->moveShape( (*aIter).nIs, nIndex );

                rZList.pop_front();
                nIndex++;
            }
        }
    }
    catch( uno::Exception& )
    {
        DBG_ERROR("exception while sorting shapes, sorting failed!");
    }

    // put parent on top and delete current context, were done
    ShapeSortContext* pContext = mpImpl->mpSortContext;
    mpImpl->mpSortContext = pContext->mpParentContext;
    delete pContext;
}

void XMLShapeImportHelper::shapeWithZIndexAdded( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape, sal_Int32 nZIndex )
{
    if( mpImpl->mpSortContext )
    {
        ZOrderHint aNewHint;
        aNewHint.nIs = mpImpl->mpSortContext->mnCurrentZ++;
        aNewHint.nShould = nZIndex;

        sal_Int32 nInsertIndex = 0;

        if( nZIndex == -1 )
        {
            // don't care, so add to unsorted list
            mpImpl->mpSortContext->maUnsortedList.push_back(aNewHint);
        }
        else
        {
            // insert into sort list
            mpImpl->mpSortContext->maZOrderList.push_back(aNewHint);
        }
    }
}

void XMLShapeImportHelper::createShapeId( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, sal_Int32 nId )
{
    DBG_ASSERT( mpImpl->maShapeIds.find(nId) == mpImpl->maShapeIds.end(), "draw:id imported twice!" );
    mpImpl->maShapeIds[nId] = xShape;
}

uno::Reference< drawing::XShape > XMLShapeImportHelper::getShapeFromId( sal_Int32 nId )
{
    IdShapeMap::iterator aShapeIter( mpImpl->maShapeIds.find( nId ) );
    if( aShapeIter != mpImpl->maShapeIds.end() )
    {
        return (*aShapeIter).second;
    }
    else
    {
        DBG_ERROR( "unknown draw:id found!" );
        uno::Reference< drawing::XShape > xShape;
        return xShape;
    }
}

void XMLShapeImportHelper::addShapeConnection( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rConnectorShape,
                         sal_Bool bStart,
                         sal_Int32 nDestShapeId,
                         sal_Int32 nDestGlueId )
{
    ConnectionHint aHint;
    aHint.mxConnector = rConnectorShape;
    aHint.bStart = bStart;
    aHint.nDestShapeId = nDestShapeId;
    aHint.nDestGlueId = nDestGlueId;

    mpImpl->maConnections.push_back( aHint );
}

void XMLShapeImportHelper::restoreConnections()
{
    if( !mpImpl->maConnections.empty() )
    {
        uno::Any aAny;

        const vector<ConnectionHint>::size_type nCount = mpImpl->maConnections.size();
        for( vector<ConnectionHint>::size_type i = 0; i < nCount; i++ )
        {
            ConnectionHint& rHint = mpImpl->maConnections[i];
            uno::Reference< beans::XPropertySet > xConnector( rHint.mxConnector, uno::UNO_QUERY );
            if( xConnector.is() )
            {
                // #86637# remember line deltas
                uno::Any aLine1Delta;
                uno::Any aLine2Delta;
                uno::Any aLine3Delta;
                OUString aStr1(RTL_CONSTASCII_USTRINGPARAM("EdgeLine1Delta"));
                OUString aStr2(RTL_CONSTASCII_USTRINGPARAM("EdgeLine2Delta"));
                OUString aStr3(RTL_CONSTASCII_USTRINGPARAM("EdgeLine3Delta"));
                aLine1Delta = xConnector->getPropertyValue(aStr1);
                aLine2Delta = xConnector->getPropertyValue(aStr2);
                aLine3Delta = xConnector->getPropertyValue(aStr3);

                // #86637# simply setting these values WILL force the connector to do
                // an new layout promptly. So the line delta values have to be rescued
                // and restored around connector changes.
                uno::Reference< drawing::XShape > xShape( getShapeFromId( rHint.nDestShapeId ) );
                if( xShape.is() )
                {
                    aAny <<= xShape;
                    xConnector->setPropertyValue( rHint.bStart ? msStartShape : msEndShape, aAny );

                    sal_Int32 nGlueId = rHint.nDestGlueId < 4 ? rHint.nDestGlueId : getGluePointId( xShape, rHint.nDestGlueId );
                    aAny <<= nGlueId;
                    xConnector->setPropertyValue( rHint.bStart ? msStartGluePointIndex : msEndGluePointIndex, aAny );
                }

                // #86637# restore line deltas
                xConnector->setPropertyValue(aStr1, aLine1Delta );
                xConnector->setPropertyValue(aStr2, aLine2Delta );
                xConnector->setPropertyValue(aStr3, aLine3Delta );
            }
        }
        mpImpl->maConnections.clear();
    }
}

SvXMLImportPropertyMapper* XMLShapeImportHelper::CreateShapePropMapper( const uno::Reference< frame::XModel>& rModel, SvXMLImport& rImport )
{
    UniReference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rModel, NULL );
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory );
    SvXMLImportPropertyMapper* pResult = new SvXMLImportPropertyMapper( xMapper, rImport );

    // chain text attributes
    pResult->ChainImportMapper( XMLTextImportHelper::CreateParaExtPropMapper( rImport ) );
    return pResult;
}

/** creates a shape property set mapper that can be used for non shape elements.
    Only current feature is that the ShapeUserDefinedAttributes property is not included in this one. */
SvXMLImportPropertyMapper* XMLShapeImportHelper::CreateExternalShapePropMapper( const uno::Reference< frame::XModel>& rModel, SvXMLImport& rImport )
{
    UniReference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rModel, NULL );
    UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory, 1 );
    SvXMLImportPropertyMapper* pResult = new SvXMLImportPropertyMapper( xMapper, rImport );

    // chain text attributes
    pResult->ChainImportMapper( XMLTextImportHelper::CreateParaExtPropMapper( rImport ) );
    return pResult;
}

/** adds a mapping for a glue point identifier from an xml file to the identifier created after inserting
    the new glue point into the core. The saved mappings can be retrieved by getGluePointId() */
void XMLShapeImportHelper::addGluePointMapping( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape,
                          sal_Int32 nSourceId, sal_Int32 nDestinnationId )
{
    if( mpPageContext )
        mpPageContext->maShapeGluePointsMap[xShape][nSourceId] = nDestinnationId;
}

/** retrieves a mapping for a glue point identifier from the current xml file to the identifier created after
    inserting the new glue point into the core. The mapping must be initialized first with addGluePointMapping() */
sal_Int32 XMLShapeImportHelper::getGluePointId( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape, sal_Int32 nSourceId )
{
    if( mpPageContext )
    {
        ShapeGluePointsMap::iterator aShapeIter( mpPageContext->maShapeGluePointsMap.find( xShape ) );
        if( aShapeIter != mpPageContext->maShapeGluePointsMap.end() )
        {
            GluePointIdMap::iterator aIdIter = (*aShapeIter).second.find(nSourceId);
            if( aIdIter != (*aShapeIter).second.end() )
                return (*aIdIter).second;
        }
    }

    return -1;
}

/** this method must be calling before the first shape is imported for the given page */
void XMLShapeImportHelper::startPage( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes )
{
    XMLShapeImportPageContextImpl* pOldContext = mpPageContext;
    mpPageContext = new XMLShapeImportPageContextImpl();
    mpPageContext->mpNext = pOldContext;
    mpPageContext->mxShapes = rShapes;
}

/** this method must be calling after the last shape is imported for the given page */
void XMLShapeImportHelper::endPage( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes )
{
    DBG_ASSERT( mpPageContext && (mpPageContext->mxShapes == rShapes), "wrong call to endPage(), no startPage called or wrong page" );
    if( NULL == mpPageContext )
        return;

    restoreConnections();

    XMLShapeImportPageContextImpl* pNextContext = mpPageContext->mpNext;
    delete mpPageContext;
    mpPageContext = pNextContext;
}

// #88546#
/** defines if the import should increment the progress bar or not */
void XMLShapeImportHelper::enableHandleProgressBar( sal_Bool bEnable )
{
    mpImpl->mbHandleProgressBar = bEnable;
}

sal_Bool XMLShapeImportHelper::IsHandleProgressBarEnabled() const
{
    return mpImpl->mbHandleProgressBar;
}

/** queries the capability of the current model to create presentation shapes */
sal_Bool XMLShapeImportHelper::IsPresentationShapesSupported()
{
    return mpImpl->mbIsPresentationShapesSupported;
}

