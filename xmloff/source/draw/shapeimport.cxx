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

#include <tools/debug.hxx>

#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <osl/diagnose.h>

#include <list>

#include <xmloff/shapeimport.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include "ximpstyl.hxx"
#include "ximpshap.hxx"
#include "sdpropls.hxx"
#include <xmloff/xmlprmap.hxx>
#include "ximp3dscene.hxx"
#include "ximp3dobject.hxx"
#include "ximpgrp.hxx"
#include "ximplink.hxx"

#include <map>
#include <vector>

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

struct ltint32
{
  bool operator()(const sal_Int32 p, sal_Int32 q) const
  {
    return p < q;
  }
};

struct ConnectionHint
{
    css::uno::Reference< css::drawing::XShape > mxConnector;
    bool      bStart;
    OUString  aDestShapeId;
    sal_Int32 nDestGlueId;
};

struct XShapeCompareHelper
{
  bool operator()(css::uno::Reference < css::drawing::XShape > x1,
                  css::uno::Reference < css::drawing::XShape > x2 ) const
  {
    return x1.get() < x2.get();
  }
};

/** this map store all glue point id mappings for shapes that had user defined glue points. This
    is needed because on insertion the glue points will get a new and unique id */
typedef std::map<sal_Int32,sal_Int32,ltint32> GluePointIdMap;
typedef std::map< css::uno::Reference < css::drawing::XShape >, GluePointIdMap, XShapeCompareHelper > ShapeGluePointsMap;

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

    std::vector<ConnectionHint> maConnections;

    // #88546# possibility to switch progress bar handling on/off
    bool                        mbHandleProgressBar;

    // stores the capability of the current model to create presentation shapes
    bool                        mbIsPresentationShapesSupported;
};

XMLShapeImportHelper::XMLShapeImportHelper(
        SvXMLImport& rImporter,
        const uno::Reference< frame::XModel>& rModel,
        SvXMLImportPropertyMapper *pExtMapper )
:   mpImpl( new XMLShapeImportHelperImpl() ),
    mpPageContext(nullptr),

    mpPropertySetMapper(nullptr),
    mpPresPagePropsMapper(nullptr),
    mpStylesContext(nullptr),
    mpAutoStylesContext(nullptr),
    mpGroupShapeElemTokenMap(nullptr),
    mpFrameShapeElemTokenMap(nullptr),
    mp3DSceneShapeElemTokenMap(nullptr),
    mp3DObjectAttrTokenMap(nullptr),
    mp3DPolygonBasedAttrTokenMap(nullptr),
    mp3DCubeObjectAttrTokenMap(nullptr),
    mp3DSphereObjectAttrTokenMap(nullptr),
    mp3DSceneShapeAttrTokenMap(nullptr),
    mp3DLightAttrTokenMap(nullptr),
    mpPathShapeAttrTokenMap(nullptr),
    mpPolygonShapeAttrTokenMap(nullptr),
    msStartShape("StartShape"),
    msEndShape("EndShape"),
    msStartGluePointIndex("StartGluePointIndex"),
    msEndGluePointIndex("EndGluePointIndex"),

    mrImporter( rImporter )
{
    mpImpl->mpSortContext = nullptr;

    // #88546# init to sal_False
    mpImpl->mbHandleProgressBar = false;

    mpSdPropHdlFactory = new XMLSdPropHdlFactory( rModel, rImporter );

    // set lock to avoid deletion
    mpSdPropHdlFactory->acquire();

    // construct PropertySetMapper
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper(mpSdPropHdlFactory, false);
    mpPropertySetMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );
    // set lock to avoid deletion
    mpPropertySetMapper->acquire();

    if( pExtMapper )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xExtMapper( pExtMapper );
        mpPropertySetMapper->ChainImportMapper( xExtMapper );
    }

    // chain text attributes
    mpPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(rImporter));
    mpPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaDefaultExtPropMapper(rImporter));

    // construct PresPagePropsMapper
    xMapper = new XMLPropertySetMapper(aXMLSDPresPageProps, mpSdPropHdlFactory, false);
    mpPresPagePropsMapper = new SvXMLImportPropertyMapper( xMapper, rImporter );
    // set lock to avoid deletion
    mpPresPagePropsMapper->acquire();

    uno::Reference< lang::XServiceInfo > xInfo( rImporter.GetModel(), uno::UNO_QUERY );
    const OUString aSName( "com.sun.star.presentation.PresentationDocument" );
    mpImpl->mbIsPresentationShapesSupported = xInfo.is() && xInfo->supportsService( aSName );
}

XMLShapeImportHelper::~XMLShapeImportHelper()
{
    DBG_ASSERT( mpImpl->maConnections.empty(), "XMLShapeImportHelper::restoreConnections() was not called!" );

    // cleanup factory, decrease refcount. Should lead to destruction.
    if(mpSdPropHdlFactory)
    {
        mpSdPropHdlFactory->release();
        mpSdPropHdlFactory = nullptr;
    }

    // cleanup mapper, decrease refcount. Should lead to destruction.
    if(mpPropertySetMapper)
    {
        mpPropertySetMapper->release();
        mpPropertySetMapper = nullptr;
    }

    // cleanup presPage mapper, decrease refcount. Should lead to destruction.
    if(mpPresPagePropsMapper)
    {
        mpPresPagePropsMapper->release();
        mpPresPagePropsMapper = nullptr;
    }

    delete mpGroupShapeElemTokenMap;
    delete mpFrameShapeElemTokenMap;

    delete mpPolygonShapeAttrTokenMap;
    delete mpPathShapeAttrTokenMap;
    delete mp3DSceneShapeElemTokenMap;
    delete mp3DObjectAttrTokenMap;
    delete mp3DPolygonBasedAttrTokenMap;
    delete mp3DCubeObjectAttrTokenMap;
    delete mp3DSphereObjectAttrTokenMap;
    delete mp3DSceneShapeAttrTokenMap;
    delete mp3DLightAttrTokenMap;

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
}

const SvXMLTokenMap& XMLShapeImportHelper::GetGroupShapeElemTokenMap()
{
    if(!mpGroupShapeElemTokenMap)
    {
        static const SvXMLTokenMapEntry aGroupShapeElemTokenMap[] =
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
    { XML_NAMESPACE_DRAW,           XML_CUSTOM_SHAPE,   XML_TOK_GROUP_CUSTOM_SHAPE  },

    { XML_NAMESPACE_DRAW,           XML_CUSTOM_SHAPE,   XML_TOK_GROUP_CUSTOM_SHAPE  },
    { XML_NAMESPACE_OFFICE,         XML_ANNOTATION,     XML_TOK_GROUP_ANNOTATION    },
    { XML_NAMESPACE_DRAW,           XML_A,              XML_TOK_GROUP_A             },

    XML_TOKEN_MAP_END
};

        mpGroupShapeElemTokenMap = new SvXMLTokenMap(aGroupShapeElemTokenMap);
    } // if(!mpGroupShapeElemTokenMap)

    return *mpGroupShapeElemTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::GetFrameShapeElemTokenMap()
{
    if(!mpFrameShapeElemTokenMap)
    {
        static const SvXMLTokenMapEntry aFrameShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_TEXT_BOX,       XML_TOK_FRAME_TEXT_BOX      },
    { XML_NAMESPACE_DRAW,           XML_IMAGE,          XML_TOK_FRAME_IMAGE         },
    { XML_NAMESPACE_DRAW,           XML_OBJECT,         XML_TOK_FRAME_OBJECT        },
    { XML_NAMESPACE_DRAW,           XML_OBJECT_OLE,     XML_TOK_FRAME_OBJECT_OLE    },
    { XML_NAMESPACE_DRAW,           XML_PLUGIN,         XML_TOK_FRAME_PLUGIN        },
    { XML_NAMESPACE_DRAW,           XML_FLOATING_FRAME, XML_TOK_FRAME_FLOATING_FRAME},
    { XML_NAMESPACE_DRAW,           XML_APPLET,         XML_TOK_FRAME_APPLET        },
    { XML_NAMESPACE_TABLE,          XML_TABLE,          XML_TOK_FRAME_TABLE         },
    XML_TOKEN_MAP_END
};

        mpFrameShapeElemTokenMap = new SvXMLTokenMap(aFrameShapeElemTokenMap);
    } // if(!mpFrameShapeElemTokenMap)

    return *mpFrameShapeElemTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DSceneShapeElemTokenMap()
{
    if(!mp3DSceneShapeElemTokenMap)
    {
        static const SvXMLTokenMapEntry a3DSceneShapeElemTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_SCENE,      XML_TOK_3DSCENE_3DSCENE     },
    { XML_NAMESPACE_DR3D,           XML_CUBE,       XML_TOK_3DSCENE_3DCUBE      },
    { XML_NAMESPACE_DR3D,           XML_SPHERE,     XML_TOK_3DSCENE_3DSPHERE    },
    { XML_NAMESPACE_DR3D,           XML_ROTATE,     XML_TOK_3DSCENE_3DLATHE     },
    { XML_NAMESPACE_DR3D,           XML_EXTRUDE,    XML_TOK_3DSCENE_3DEXTRUDE   },
    XML_TOKEN_MAP_END
};

        mp3DSceneShapeElemTokenMap = new SvXMLTokenMap(a3DSceneShapeElemTokenMap);
    } // if(!mp3DSceneShapeElemTokenMap)

    return *mp3DSceneShapeElemTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DObjectAttrTokenMap()
{
    if(!mp3DObjectAttrTokenMap)
    {
        static const SvXMLTokenMapEntry a3DObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW,           XML_STYLE_NAME,         XML_TOK_3DOBJECT_DRAWSTYLE_NAME     },
    { XML_NAMESPACE_DR3D,           XML_TRANSFORM,          XML_TOK_3DOBJECT_TRANSFORM          },
    XML_TOKEN_MAP_END
};

        mp3DObjectAttrTokenMap = new SvXMLTokenMap(a3DObjectAttrTokenMap);
    } // if(!mp3DObjectAttrTokenMap)

    return *mp3DObjectAttrTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DPolygonBasedAttrTokenMap()
{
    if(!mp3DPolygonBasedAttrTokenMap)
    {
        static const SvXMLTokenMapEntry a3DPolygonBasedAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,            XML_VIEWBOX,            XML_TOK_3DPOLYGONBASED_VIEWBOX      },
    { XML_NAMESPACE_SVG,            XML_D,                  XML_TOK_3DPOLYGONBASED_D            },
    XML_TOKEN_MAP_END
};

        mp3DPolygonBasedAttrTokenMap = new SvXMLTokenMap(a3DPolygonBasedAttrTokenMap);
    } // if(!mp3DPolygonBasedAttrTokenMap)

    return *mp3DPolygonBasedAttrTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DCubeObjectAttrTokenMap()
{
    if(!mp3DCubeObjectAttrTokenMap)
    {
        static const SvXMLTokenMapEntry a3DCubeObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_MIN_EDGE,           XML_TOK_3DCUBEOBJ_MINEDGE   },
    { XML_NAMESPACE_DR3D,           XML_MAX_EDGE,           XML_TOK_3DCUBEOBJ_MAXEDGE   },
    XML_TOKEN_MAP_END
};

        mp3DCubeObjectAttrTokenMap = new SvXMLTokenMap(a3DCubeObjectAttrTokenMap);
    } // if(!mp3DCubeObjectAttrTokenMap)

    return *mp3DCubeObjectAttrTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DSphereObjectAttrTokenMap()
{
    if(!mp3DSphereObjectAttrTokenMap)
    {
        static const SvXMLTokenMapEntry a3DSphereObjectAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,           XML_CENTER,             XML_TOK_3DSPHEREOBJ_CENTER  },
    { XML_NAMESPACE_DR3D,           XML_SIZE,               XML_TOK_3DSPHEREOBJ_SIZE    },
    XML_TOKEN_MAP_END
};

        mp3DSphereObjectAttrTokenMap = new SvXMLTokenMap(a3DSphereObjectAttrTokenMap);
    } // if(!mp3DSphereObjectAttrTokenMap)

    return *mp3DSphereObjectAttrTokenMap;
}

const SvXMLTokenMap& XMLShapeImportHelper::Get3DLightAttrTokenMap()
{
    if(!mp3DLightAttrTokenMap)
    {
        static const SvXMLTokenMapEntry a3DLightAttrTokenMap[] =
{
    { XML_NAMESPACE_DR3D,   XML_DIFFUSE_COLOR,      XML_TOK_3DLIGHT_DIFFUSE_COLOR       },
    { XML_NAMESPACE_DR3D,   XML_DIRECTION,          XML_TOK_3DLIGHT_DIRECTION           },
    { XML_NAMESPACE_DR3D,   XML_ENABLED,            XML_TOK_3DLIGHT_ENABLED             },
    { XML_NAMESPACE_DR3D,   XML_SPECULAR,           XML_TOK_3DLIGHT_SPECULAR            },
    XML_TOKEN_MAP_END
};

        mp3DLightAttrTokenMap = new SvXMLTokenMap(a3DLightAttrTokenMap);
    } // if(!mp3DLightAttrTokenMap)

    return *mp3DLightAttrTokenMap;
}

SvXMLShapeContext* XMLShapeImportHelper::Create3DSceneChildContext(
    SvXMLImport& rImport,
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
{
    SdXMLShapeContext *pContext = nullptr;

    if(rShapes.is())
    {
        const SvXMLTokenMap& rTokenMap = Get3DSceneShapeElemTokenMap();
        switch(rTokenMap.Get(p_nPrefix, rLocalName))
        {
            case XML_TOK_3DSCENE_3DSCENE:
            {
                // dr3d:3dscene inside dr3d:3dscene context
                pContext = new SdXML3DSceneShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false);
                break;
            }
            case XML_TOK_3DSCENE_3DCUBE:
            {
                // dr3d:3dcube inside dr3d:3dscene context
                pContext = new SdXML3DCubeObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false);
                break;
            }
            case XML_TOK_3DSCENE_3DSPHERE:
            {
                // dr3d:3dsphere inside dr3d:3dscene context
                pContext = new SdXML3DSphereObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false);
                break;
            }
            case XML_TOK_3DSCENE_3DLATHE:
            {
                // dr3d:3dlathe inside dr3d:3dscene context
                pContext = new SdXML3DLatheObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false);
                break;
            }
            case XML_TOK_3DSCENE_3DEXTRUDE:
            {
                // dr3d:3dextrude inside dr3d:3dscene context
                pContext = new SdXML3DExtrudeObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false);
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

void XMLShapeImportHelper::SetStylesContext(SvXMLStylesContext* pNew)
{
    mpStylesContext = pNew;
    if (mpStylesContext)
        mpStylesContext->AddFirstRef();
}

void XMLShapeImportHelper::SetAutoStylesContext(SvXMLStylesContext* pNew)
{
    mpAutoStylesContext = pNew;
    if (mpAutoStylesContext)
        mpAutoStylesContext->AddFirstRef();
}

SvXMLShapeContext* XMLShapeImportHelper::CreateGroupChildContext(
    SvXMLImport& rImport,
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
{
    SdXMLShapeContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetGroupShapeElemTokenMap();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    switch(rTokenMap.Get(p_nPrefix, rLocalName))
    {
        case XML_TOK_GROUP_GROUP:
        {
            // draw:g inside group context (RECURSIVE)
            pContext = new SdXMLGroupShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape);
            break;
        }
        case XML_TOK_GROUP_3DSCENE:
        {
            // dr3d:3dscene inside group context
            pContext = new SdXML3DSceneShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape);
            break;
        }
        case XML_TOK_GROUP_RECT:
        {
            // draw:rect inside group context
            pContext = new SdXMLRectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_LINE:
        {
            // draw:line inside group context
            pContext = new SdXMLLineShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CIRCLE:
        case XML_TOK_GROUP_ELLIPSE:
        {
            // draw:circle or draw:ellipse inside group context
            pContext = new SdXMLEllipseShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_POLYGON:
        case XML_TOK_GROUP_POLYLINE:
        {
            // draw:polygon or draw:polyline inside group context
            pContext = new SdXMLPolygonShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes,
                rTokenMap.Get(p_nPrefix, rLocalName) == XML_TOK_GROUP_POLYGON, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_PATH:
        {
            // draw:path inside group context
            pContext = new SdXMLPathShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape);
            break;
        }
        case XML_TOK_GROUP_FRAME:
        {
            // text:text-box inside group context
            pContext = new SdXMLFrameShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CONTROL:
        {
            // draw:control inside group context
            pContext = new SdXMLControlShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CONNECTOR:
        {
            // draw:connector inside group context
            pContext = new SdXMLConnectorShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_MEASURE:
        {
            // draw:measure inside group context
            pContext = new SdXMLMeasureShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_PAGE:
        {
            // draw:page inside group context
            pContext = new SdXMLPageShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CAPTION:
        case XML_TOK_GROUP_ANNOTATION:
        {
            // draw:caption inside group context
            pContext = new SdXMLCaptionShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CHART:
        {
            // chart:chart inside group context
            pContext = new SdXMLChartShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, bTemporaryShape );
            break;
        }
        case XML_TOK_GROUP_CUSTOM_SHAPE:
        {
            // draw:customshape
            pContext = new SdXMLCustomShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
         case XML_TOK_GROUP_A:
         {
             return new SdXMLShapeLinkContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes );
         }
        // add other shapes here...
        default:
            return new SvXMLShapeContext( rImport, p_nPrefix, rLocalName, bTemporaryShape );
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

// This method is called from SdXMLFrameShapeContext to create children of draw:frame
SvXMLShapeContext* XMLShapeImportHelper::CreateFrameChildContext(
    SvXMLImport& rImport,
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& rAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    const uno::Reference< xml::sax::XAttributeList>& rFrameAttrList)
{
    SdXMLShapeContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetFrameShapeElemTokenMap();

    SvXMLAttributeList *pAttrList = new SvXMLAttributeList( rAttrList );
    if( rFrameAttrList.is() )
        pAttrList->AppendAttributeList( rFrameAttrList );
    uno::Reference < xml::sax::XAttributeList > xAttrList = pAttrList;

    switch(rTokenMap.Get(p_nPrefix, rLocalName))
    {
        case XML_TOK_FRAME_TEXT_BOX:
        {
            // text:text-box inside group context
            pContext = new SdXMLTextBoxShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
        case XML_TOK_FRAME_IMAGE:
        {
            // office:image inside group context
            pContext = new SdXMLGraphicObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
        case XML_TOK_FRAME_OBJECT:
        case XML_TOK_FRAME_OBJECT_OLE:
        {
            // draw:object or draw:object_ole
            pContext = new SdXMLObjectShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
        case XML_TOK_FRAME_TABLE:
        {
            // draw:object or draw:object_ole
            if( rImport.IsTableShapeSupported() )
                pContext = new SdXMLTableShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes );
            break;

        }
        case XML_TOK_FRAME_PLUGIN:
        {
            // draw:plugin
            pContext = new SdXMLPluginShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
        case XML_TOK_FRAME_FLOATING_FRAME:
        {
            // draw:floating-frame
            pContext = new SdXMLFloatingFrameShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
            break;
        }
        case XML_TOK_FRAME_APPLET:
        {
            // draw:applet
            pContext = new SdXMLAppletShapeContext( rImport, p_nPrefix, rLocalName, xAttrList, rShapes, false );
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
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = nullptr;

    SdXMLFrameShapeContext *pFrameContext = dynamic_cast<SdXMLFrameShapeContext*>( pThisContext  );
    if( pFrameContext )
        pContext = pFrameContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

/** this function is called whenever the implementation classes like to add this new
    shape to the given XShapes.
*/
void XMLShapeImportHelper::addShape( uno::Reference< drawing::XShape >& rShape,
                                     const uno::Reference< xml::sax::XAttributeList >&,
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
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XAttributeList >&,
        css::uno::Reference< css::drawing::XShapes >&)
{
    /* Set property <PositionLayoutDir>
       to <PositionInHoriL2R>, if it exists and the import states that
       the shape positioning attributes are in horizontal left-to-right
       layout. This is the case for the OpenOffice.org file format.
       This setting is done for Writer documents, because the property
       only exists at service css::text::Shape - the Writer
       UNO service for shapes.
       The value indicates that the positioning attributes are given
       in horizontal left-to-right layout. The property is evaluated
       during the first positioning of the shape in order to convert
       the shape position given in the OpenOffice.org file format to
       the one for the OASIS Open Office file format. (#i28749#, #i36248#)
    */
    uno::Reference< beans::XPropertySet > xPropSet(rShape, uno::UNO_QUERY);
    if ( xPropSet.is() )
    {
        if ( mrImporter.IsShapePositionInHoriL2R() &&
             xPropSet->getPropertySetInfo()->hasPropertyByName(
                "PositionLayoutDir") )
        {
            uno::Any aPosLayoutDir;
            aPosLayoutDir <<= text::PositionLayoutDir::PositionInHoriL2R;
            xPropSet->setPropertyValue( "PositionLayoutDir", aPosLayoutDir );
        }
    }
}

// helper functions for z-order sorting
struct ZOrderHint
{
    sal_Int32 nIs;
    sal_Int32 nShould;

    bool operator<(const ZOrderHint& rComp) const { return nShould < rComp.nShould; }
};

class ShapeSortContext
{
public:
    uno::Reference< drawing::XShapes > mxShapes;
    vector<ZOrderHint>              maZOrderList;
    vector<ZOrderHint>              maUnsortedList;

    sal_Int32                       mnCurrentZ;
    ShapeSortContext*               mpParentContext;

    ShapeSortContext( uno::Reference< drawing::XShapes >& rShapes, ShapeSortContext* pParentContext = nullptr );

    void popGroupAndSort();
private:
    void moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos );
};

ShapeSortContext::ShapeSortContext( uno::Reference< drawing::XShapes >& rShapes, ShapeSortContext* pParentContext )
:   mxShapes( rShapes ), mnCurrentZ( 0 ), mpParentContext( pParentContext )
{
}

void ShapeSortContext::moveShape( sal_Int32 nSourcePos, sal_Int32 nDestPos )
{
    uno::Any aAny( mxShapes->getByIndex( nSourcePos ) );
    uno::Reference< beans::XPropertySet > xPropSet;
    aAny >>= xPropSet;

    if( xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName( "ZOrder" ) )
    {
        aAny <<= nDestPos;
        xPropSet->setPropertyValue( "ZOrder", aAny );

        for( ZOrderHint& rHint : maZOrderList )
        {
            if( rHint.nIs < nSourcePos )
            {
                DBG_ASSERT(rHint.nIs >= nDestPos, "Shape sorting failed" );
                rHint.nIs++;
            }
        }

        for( ZOrderHint& rHint : maUnsortedList )
        {
            if( rHint.nIs < nSourcePos )
            {
                DBG_ASSERT( rHint.nIs >= nDestPos, "shape sorting failed" );
                rHint.nIs++;
            }
        }
    }
}

// sort shapes
void ShapeSortContext::popGroupAndSort()
{
    // only do something if we have shapes to sort
    if( maZOrderList.empty() )
        return;

    // check if there are more shapes than inserted with ::shapeWithZIndexAdded()
    // This can happen if there where already shapes on the page before import
    // Since the writer may delete some of this shapes during import, we need
    // to do this here and not in our c'tor anymore

    // check if we have more shapes than we know of
    sal_Int32 nCount = mxShapes->getCount();

    nCount -= maZOrderList.size();
    nCount -= maUnsortedList.size();

    if( nCount > 0 )
    {
        // first update offsets of added shapes
        for (ZOrderHint& rHint : maZOrderList)
            rHint.nIs += nCount;
        for (ZOrderHint& rHint : maUnsortedList)
            rHint.nIs += nCount;

        // second add the already existing shapes in the unsorted list
        ZOrderHint aNewHint;
        do
        {
            nCount--;

            aNewHint.nIs = nCount;
            aNewHint.nShould = -1;

            maUnsortedList.insert(maUnsortedList.begin(), aNewHint);
        }
        while( nCount );
    }

    // sort z-ordered shapes by nShould field
    std::sort(maZOrderList.begin(), maZOrderList.end());

    // this is the current index, all shapes before that
    // index are finished
    sal_Int32 nIndex = 0;
    for (ZOrderHint& rHint : maZOrderList)
    {
        while( nIndex < rHint.nShould && !maUnsortedList.empty() )
        {
            auto it = maUnsortedList.begin();

            ZOrderHint aGapHint(*it);
            maUnsortedList.erase(it);

            moveShape( aGapHint.nIs, nIndex++ );
        }

        if(rHint.nIs != nIndex )
            moveShape( rHint.nIs, nIndex );

        nIndex++;
    }
    maZOrderList.clear();
}

void XMLShapeImportHelper::pushGroupForSorting( uno::Reference< drawing::XShapes >& rShapes )
{
    mpImpl->mpSortContext = new ShapeSortContext( rShapes, mpImpl->mpSortContext );
}

void XMLShapeImportHelper::popGroupAndSort()
{
    DBG_ASSERT( mpImpl->mpSortContext, "No context to sort!" );
    if( mpImpl->mpSortContext == nullptr )
        return;

    try
    {
        mpImpl->mpSortContext->popGroupAndSort();
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("exception while sorting shapes, sorting failed!");
    }

    // put parent on top and delete current context, we are done
    ShapeSortContext* pContext = mpImpl->mpSortContext;
    mpImpl->mpSortContext = pContext->mpParentContext;
    delete pContext;
}

void XMLShapeImportHelper::shapeWithZIndexAdded( css::uno::Reference< css::drawing::XShape >&, sal_Int32 nZIndex )
{
    if( mpImpl->mpSortContext)
    {
        ZOrderHint aNewHint;
        aNewHint.nIs = mpImpl->mpSortContext->mnCurrentZ++;
        aNewHint.nShould = nZIndex;

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

void XMLShapeImportHelper::addShapeConnection( css::uno::Reference< css::drawing::XShape >& rConnectorShape,
                         bool bStart,
                         const OUString& rDestShapeId,
                         sal_Int32 nDestGlueId )
{
    ConnectionHint aHint;
    aHint.mxConnector = rConnectorShape;
    aHint.bStart = bStart;
    aHint.aDestShapeId = rDestShapeId;
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
                OUString aStr1("EdgeLine1Delta");
                OUString aStr2("EdgeLine2Delta");
                OUString aStr3("EdgeLine3Delta");
                aLine1Delta = xConnector->getPropertyValue(aStr1);
                aLine2Delta = xConnector->getPropertyValue(aStr2);
                aLine3Delta = xConnector->getPropertyValue(aStr3);

                // #86637# simply setting these values WILL force the connector to do
                // an new layout promptly. So the line delta values have to be rescued
                // and restored around connector changes.
                uno::Reference< drawing::XShape > xShape(
                    mrImporter.getInterfaceToIdentifierMapper().getReference( rHint.aDestShapeId ), uno::UNO_QUERY );
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
    rtl::Reference< XMLPropertyHandlerFactory > xFactory = new XMLSdPropHdlFactory( rModel, rImport );
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( xFactory, false );
    SvXMLImportPropertyMapper* pResult = new SvXMLImportPropertyMapper( xMapper, rImport );

    // chain text attributes
    pResult->ChainImportMapper( XMLTextImportHelper::CreateParaExtPropMapper( rImport ) );
    return pResult;
}

/** adds a mapping for a glue point identifier from an xml file to the identifier created after inserting
    the new glue point into the core. The saved mappings can be retrieved by getGluePointId() */
void XMLShapeImportHelper::addGluePointMapping( css::uno::Reference< css::drawing::XShape >& xShape,
                          sal_Int32 nSourceId, sal_Int32 nDestinnationId )
{
    if( mpPageContext )
        mpPageContext->maShapeGluePointsMap[xShape][nSourceId] = nDestinnationId;
}

/** find mapping for given DestinationID. This allows to extract the original draw:id imported with a draw:glue-point */
sal_Int32 XMLShapeImportHelper::findGluePointMapping(
    const css::uno::Reference< css::drawing::XShape >& xShape,
    sal_Int32 nDestinnationId ) const
{
    if( mpPageContext )
    {
        ShapeGluePointsMap::iterator aShapeIter( mpPageContext->maShapeGluePointsMap.find( xShape ) );

        if( aShapeIter != mpPageContext->maShapeGluePointsMap.end() )
        {
            GluePointIdMap::iterator aShapeIdIter = (*aShapeIter).second.begin();
            GluePointIdMap::iterator aShapeIdEnd  = (*aShapeIter).second.end();

            while ( aShapeIdIter != aShapeIdEnd )
            {
                if ( (*aShapeIdIter).second == nDestinnationId )
                {
                    return (*aShapeIdIter).first;
                }

                ++aShapeIdIter;
            }
        }
    }

    return -1;
}

/** moves all current DestinationId's by n */
void XMLShapeImportHelper::moveGluePointMapping( const css::uno::Reference< css::drawing::XShape >& xShape, const sal_Int32 n )
{
    if( mpPageContext )
    {
        ShapeGluePointsMap::iterator aShapeIter( mpPageContext->maShapeGluePointsMap.find( xShape ) );
        if( aShapeIter != mpPageContext->maShapeGluePointsMap.end() )
        {
            GluePointIdMap::iterator aShapeIdIter = (*aShapeIter).second.begin();
            GluePointIdMap::iterator aShapeIdEnd  = (*aShapeIter).second.end();
            while ( aShapeIdIter != aShapeIdEnd )
            {
                if ( (*aShapeIdIter).second != -1 )
                    (*aShapeIdIter).second += n;
                ++aShapeIdIter;
            }
        }
    }
}

/** retrieves a mapping for a glue point identifier from the current xml file to the identifier created after
    inserting the new glue point into the core. The mapping must be initialized first with addGluePointMapping() */
sal_Int32 XMLShapeImportHelper::getGluePointId( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nSourceId )
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
void XMLShapeImportHelper::startPage( css::uno::Reference< css::drawing::XShapes >& rShapes )
{
    XMLShapeImportPageContextImpl* pOldContext = mpPageContext;
    mpPageContext = new XMLShapeImportPageContextImpl();
    mpPageContext->mpNext = pOldContext;
    mpPageContext->mxShapes = rShapes;
}

/** this method must be calling after the last shape is imported for the given page */
void XMLShapeImportHelper::endPage( css::uno::Reference< css::drawing::XShapes >& rShapes )
{
    DBG_ASSERT( mpPageContext && (mpPageContext->mxShapes == rShapes), "wrong call to endPage(), no startPage called or wrong page" );
    if( nullptr == mpPageContext )
        return;

    restoreConnections();

    XMLShapeImportPageContextImpl* pNextContext = mpPageContext->mpNext;
    delete mpPageContext;
    mpPageContext = pNextContext;
}

/** defines if the import should increment the progress bar or not */
void XMLShapeImportHelper::enableHandleProgressBar( bool bEnable )
{
    mpImpl->mbHandleProgressBar = bEnable;
}

bool XMLShapeImportHelper::IsHandleProgressBarEnabled() const
{
    return mpImpl->mbHandleProgressBar;
}

/** queries the capability of the current model to create presentation shapes */
bool XMLShapeImportHelper::IsPresentationShapesSupported()
{
    return mpImpl->mbIsPresentationShapesSupported;
}

const rtl::Reference< XMLTableImport >& XMLShapeImportHelper::GetShapeTableImport()
{
    if( !mxShapeTableImport.is() )
    {
        rtl::Reference< XMLPropertyHandlerFactory > xFactory( new XMLSdPropHdlFactory( mrImporter.GetModel(), mrImporter ) );
        rtl::Reference< XMLPropertySetMapper > xPropertySetMapper( new XMLShapePropertySetMapper( xFactory.get(), false ) );
        mxShapeTableImport = new XMLTableImport( mrImporter, xPropertySetMapper, xFactory );
    }

    return mxShapeTableImport;
}

void SvXMLShapeContext::setHyperlink( const OUString& rHyperlink )
{
    msHyperlink = rHyperlink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
