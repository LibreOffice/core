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
#ifndef INCLUDED_FILTER_SOURCE_MSFILTER_ESCHESDO_HXX
#define INCLUDED_FILTER_SOURCE_MSFILTER_ESCHESDO_HXX
#include <filter/msfilter/escherex.hxx>
#include <o3tl/any.hxx>
#include <vcl/mapmod.hxx>


// fractions of Draw PPTWriter etc.

class ImplEESdrWriter;
class SdrObject;
class SdrPage;

class ImplEESdrObject
{
    css::uno::Reference< css::drawing::XShape >           mXShape;
    css::uno::Any       mAny;
    tools::Rectangle           maRect;
    OUString            mType;
    sal_uInt32          mnShapeId;
    sal_uInt32          mnTextSize;
    sal_Int32           mnAngle;
    bool                mbValid : 1;
    bool                mbPresObj : 1;
    bool                mbEmptyPresObj : 1;
    bool                mbOOXML;

    void Init( ImplEESdrWriter& rEx );
public:
    css::uno::Reference< css::beans::XPropertySet >   mXPropSet;

    ImplEESdrObject( ImplEESdrWriter& rEx, const SdrObject& rObj, bool bOOXML );
    ImplEESdrObject( ImplEESdrWriter& rEx, const css::uno::Reference< css::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    bool ImplGetPropertyValue( const OUString& rString );

    sal_Int32 ImplGetInt32PropertyValue( const OUString& rStr )
    { return ImplGetPropertyValue( rStr ) ? *o3tl::doAccess<sal_Int32>(mAny) : 0; }

    const css::uno::Reference< css::drawing::XShape >&    GetShapeRef() const     { return mXShape; }
    const css::uno::Any&       GetUsrAny() const       { return mAny; }
    const OUString&     GetType() const         { return mType; }
    void                SetType( const OUString& rS ) { mType = rS; }

    const tools::Rectangle&    GetRect() const         { return maRect; }
    void                SetRect( const Point& rPos, const Size& rSz );
    void                SetRect( const tools::Rectangle& rRect )
                            { maRect = rRect; }

    sal_Int32           GetAngle() const        { return mnAngle; }
    void                SetAngle( sal_Int32 nVal )  { mnAngle = nVal; }

    bool                IsValid() const         { return mbValid; }

    bool                IsEmptyPresObj() const  { return mbEmptyPresObj; }
    sal_uInt32          GetShapeId() const      { return mnShapeId; }
    void                SetShapeId( sal_uInt32 nVal ) { mnShapeId = nVal; }

    const SdrObject*    GetSdrObject() const;

    sal_uInt32          ImplGetText();
    bool                ImplHasText() const;
    bool                GetOOXML() const { return mbOOXML;}
    void                SetOOXML(bool bOOXML);
};


// fractions of the Draw PPTWriter

class EscherEx;
namespace com::sun::star {
    namespace drawing {
        class XDrawPage;
        class XShape;
    }
    namespace task {
        class XStatusIndicator;
    }
}
class EscherExHostAppData;

class ImplEESdrWriter
{
    EscherEx*           mpEscherEx;
    MapMode             maMapModeSrc;
    MapMode             maMapModeDest;
    css::uno::Reference< css::drawing::XDrawPage >        mXDrawPage;
    css::uno::Reference< css::drawing::XShapes >          mXShapes;
    SvStream*           mpPicStrm;
    // own extensions
    EscherExHostAppData*    mpHostAppData;
    bool                    mbIsTitlePossible;
    const SdrPage*          mpSdrPage;
    std::unique_ptr<EscherSolverContainer> mpSolverContainer;

    void                ImplInitPageValues();
    void                ImplWritePage( EscherSolverContainer& rSolver );
    sal_uInt32          ImplWriteShape( ImplEESdrObject& rObj,
                            EscherSolverContainer& rSolver,
                            const bool bOOxmlExport = false );  // returns ShapeID
    static void         ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
    void                ImplWriteAdditionalText(
                            ImplEESdrObject& rObj );
    sal_uInt32          ImplEnterAdditionalTextGroup(
                            const css::uno::Reference< css::drawing::XShape >& rShape,
                            const tools::Rectangle* pBoundRect );
    void                ImplFlushSolverContainer();

public:
    explicit            ImplEESdrWriter( EscherEx& rEx );
                        ~ImplEESdrWriter();
    Point               ImplMapPoint( const Point& rPoint );
    Size                ImplMapSize( const Size& rSize );
    EscherExHostAppData* ImplGetHostData() { return mpHostAppData; }
    bool                ImplInitPage( const SdrPage& rPage );
    bool                ImplInitUnoShapes( const css::uno::Reference< css::drawing::XShapes >& rxShapes );
    void                ImplWriteCurrentPage();
    sal_uInt32          ImplWriteTheShape( ImplEESdrObject& rObj, bool ooxmlExport );
    void                ImplExitPage();
};


#endif // INCLUDED_FILTER_SOURCE_MSFILTER_ESCHESDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
