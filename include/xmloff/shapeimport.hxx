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

#ifndef INCLUDED_XMLOFF_SHAPEIMPORT_HXX
#define INCLUDED_XMLOFF_SHAPEIMPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <salhelper/simplereferenceobject.hxx>
#include <xmloff/xmlictxt.hxx>
#include <sax/fastattribs.hxx>

#include <basegfx/vector/b3dvector.hxx>
#include <vector>
#include <memory>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::drawing { class XShapes; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::xml::sax { class XAttributeList; }

class SvXMLImport;
class SvXMLTokenMap;
class SvXMLStylesContext;
class XMLSdPropHdlFactory;
class SvXMLImportPropertyMapper;
class XMLTableImport;

// dr3d:3dlight context

class SdXML3DLightContext final : public SvXMLImportContext
{
    // local parameters which need to be read
    sal_Int32                   maDiffuseColor;
    ::basegfx::B3DVector        maDirection;
    bool                        mbEnabled;
    bool                        mbSpecular;

public:
    SdXML3DLightContext(
        SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList);
    virtual ~SdXML3DLightContext() override;

    sal_Int32 GetDiffuseColor() const { return maDiffuseColor; }
    const ::basegfx::B3DVector& GetDirection() const { return maDirection; }
    bool GetEnabled() const { return mbEnabled; }
};


class SdXML3DSceneAttributesHelper
{
protected:
    SvXMLImport& mrImport;

    // list for local light contexts
    ::std::vector< rtl::Reference< SdXML3DLightContext > >
                                maList;

    // local parameters which need to be read
    css::drawing::HomogenMatrix mxHomMat;
    bool                        mbSetTransform;

    css::drawing::ProjectionMode mxPrjMode;
    sal_Int32                   mnDistance;
    sal_Int32                   mnFocalLength;
    sal_Int32                   mnShadowSlant;
    css::drawing::ShadeMode     mxShadeMode;
    sal_Int32                   maAmbientColor;
    bool                        mbLightingMode;

    ::basegfx::B3DVector        maVRP;
    ::basegfx::B3DVector        maVPN;
    ::basegfx::B3DVector        maVUP;
    bool                        mbVRPUsed;

public:
    SdXML3DSceneAttributesHelper( SvXMLImport& rImporter );

    /** creates a 3d light context and adds it to the internal list for later processing */
    SvXMLImportContext * create3DLightContext( const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList);

    /** this should be called for each scene attribute */
    void processSceneAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter );

    /** this sets the scene attributes at this propertyset */
    void setSceneAttributes( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
};


class SvXMLShapeContext : public SvXMLImportContext
{
protected:
    css::uno::Reference< css::drawing::XShape >   mxShape;
    bool                                          mbTemporaryShape;
    OUString                                      msHyperlink;

public:
    SvXMLShapeContext( SvXMLImport& rImp, bool bTemporaryShape ) : SvXMLImportContext( rImp ), mbTemporaryShape(bTemporaryShape) {}


    const css::uno::Reference< css::drawing::XShape >& getShape() const { return mxShape; }

    void setHyperlink( const OUString& rHyperlink );
};


struct XMLShapeImportHelperImpl;
struct XMLShapeImportPageContextImpl;
struct SdXMLEventContextData;

class XMLOFF_DLLPUBLIC XMLShapeImportHelper : public salhelper::SimpleReferenceObject
{
    std::unique_ptr<XMLShapeImportHelperImpl> mpImpl;

    std::shared_ptr<XMLShapeImportPageContextImpl> mpPageContext;

    // PropertySetMappers and factory
    rtl::Reference<XMLSdPropHdlFactory>       mpSdPropHdlFactory;
    rtl::Reference<SvXMLImportPropertyMapper> mpPropertySetMapper;
    rtl::Reference<SvXMLImportPropertyMapper> mpPresPagePropsMapper;

    // contexts for Style and AutoStyle import
    rtl::Reference<SvXMLStylesContext> mxStylesContext;
    rtl::Reference<SvXMLStylesContext> mxAutoStylesContext;

    rtl::Reference< XMLTableImport > mxShapeTableImport;

protected:
    SvXMLImport& mrImporter;

public:
    XMLShapeImportHelper( SvXMLImport& rImporter,
        const css::uno::Reference< css::frame::XModel>& rModel,
    SvXMLImportPropertyMapper *pExtMapper=nullptr );

    virtual ~XMLShapeImportHelper() override;

    static SvXMLShapeContext* CreateGroupChildContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes,
        bool bTemporaryShape = false);

    static SvXMLShapeContext* CreateFrameChildContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xFrameAttrList);
    static css::uno::Reference< css::xml::sax::XFastContextHandler > CreateFrameChildContext(
        SvXMLImportContext *pThisContext, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );

    static SvXMLShapeContext* Create3DSceneChildContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes);

    // Styles and AutoStyles contexts
    SvXMLStylesContext* GetStylesContext() const { return mxStylesContext.get(); }
    void SetStylesContext(SvXMLStylesContext* pNew);
    SvXMLStylesContext* GetAutoStylesContext() const { return mxAutoStylesContext.get(); }
    void SetAutoStylesContext(SvXMLStylesContext* pNew);

    // get factories and mappers
    SvXMLImportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper.get(); }
    SvXMLImportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper.get(); }

    // this function is called whenever the implementation classes like to add this new
    // shape to the given XShapes.
    virtual void addShape(
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);

    // this function is called whenever the implementation classes have finished importing
    // a shape to the given XShapes. The shape is already inserted into its XShapes and
    // all properties and styles are set.
    virtual void finishShape(
        css::uno::Reference< css::drawing::XShape >& rShape,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);

    // tdf#127791 help function for group shape events
    void addShapeEvents(SdXMLEventContextData& rData);

    // helper functions processing groups after their component shapes are collected
    // e.g. for z-order sorting or adding events to the group
    void pushGroupForPostProcessing( css::uno::Reference< css::drawing::XShapes >& rShapes );
    void popGroupAndPostProcess();

    void shapeWithZIndexAdded( css::uno::Reference< css::drawing::XShape > const & rShape,
                               sal_Int32 nZIndex );
    /// Updates the z-order of other shapes to be consistent again, needed due
    /// to the removal of rShape.
    void shapeRemoved(const css::uno::Reference<css::drawing::XShape>& rShape);

    void addShapeConnection( css::uno::Reference< css::drawing::XShape > const & rConnectorShape,
                             bool bStart,
                             const OUString& rDestShapeId,
                             sal_Int32 nDestGlueId );

    /** adds a mapping for a glue point identifier from an xml file to the identifier created after inserting
        the new glue point into the core. The saved mappings can be retrieved by getGluePointId() */
    void addGluePointMapping( css::uno::Reference< css::drawing::XShape > const & xShape,
                              sal_Int32 nSourceId, sal_Int32 nDestinnationId );

    /** moves all current DestinationId's for rXShape by n */
    void moveGluePointMapping( const css::uno::Reference< css::drawing::XShape >& xShape, const sal_Int32 n );

    /** retrieves a mapping for a glue point identifier from the current xml file to the identifier created after
        inserting the new glue point into the core. The mapping must be initialized first with addGluePointMapping() */
    sal_Int32 getGluePointId( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nSourceId );

    /** this method must be calling before the first shape is imported for the given page.
        Calls to this method can be nested */
    void startPage( css::uno::Reference< css::drawing::XShapes > const & rShapes );

    /** this method must be calling after the last shape is imported for the given page
        Calls to this method can be nested */
    void endPage( css::uno::Reference< css::drawing::XShapes > const & rShapes );

    void restoreConnections();

    /** creates a property mapper for external chaining */
    static SvXMLImportPropertyMapper* CreateShapePropMapper(
        const css::uno::Reference< css::frame::XModel>& rModel, SvXMLImport& rImport );

    /** defines if the import should increment the progress bar or not */
    void enableHandleProgressBar();
    bool IsHandleProgressBarEnabled() const;

    /** queries the capability of the current model to create presentation shapes */
    bool IsPresentationShapesSupported() const;

    XMLSdPropHdlFactory* GetSdPropHdlFactory() const { return mpSdPropHdlFactory.get(); }

    const rtl::Reference< XMLTableImport >&     GetShapeTableImport();
};

#endif // INCLUDED_XMLOFF_SHAPEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
