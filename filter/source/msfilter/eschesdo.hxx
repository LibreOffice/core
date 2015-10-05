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
#include <svx/unopage.hxx>
#include <vcl/mapmod.hxx>


// fractions of Draw PPTWriter etc.

enum ImplEESdrPageType { NORMAL = 0, MASTER = 1, NOTICE = 2, UNDEFINED = 3 };

class ImplEESdrWriter;
class ImplEscherExSdr;

class ImplEESdrObject
{
    css::uno::Reference< css::drawing::XShape >           mXShape;
//  XTextRef            mXText; // TextRef des globalen Text
    css::uno::Any       mAny;
    Rectangle           maRect;
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

    ImplEESdrObject( ImplEscherExSdr& rEx, const SdrObject& rObj, bool bOOXML = false );
    ImplEESdrObject( ImplEESdrWriter& rEx, const css::uno::Reference< css::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    bool ImplGetPropertyValue( const sal_Unicode* pString );
    bool ImplGetPropertyValue( const OUString& rString ) { return ImplGetPropertyValue(rString.getStr()); }

    sal_Int32 ImplGetInt32PropertyValue( const sal_Unicode* pStr, sal_uInt32 nDef = 0 )
    { return ImplGetPropertyValue( pStr ) ? *static_cast<sal_Int32 const *>(mAny.getValue()) : nDef; }
    sal_Int32 ImplGetInt32PropertyValue( const OUString& rStr, sal_uInt32 nDef = 0 )
    { return ImplGetInt32PropertyValue(rStr.getStr(), nDef); }

    const css::uno::Reference< css::drawing::XShape >&    GetShapeRef() const     { return mXShape; }
    const css::uno::Any&       GetUsrAny() const       { return mAny; }
    const OUString&     GetType() const         { return mType; }
    void                SetType( const OUString& rS ) { mType = rS; }

    const Rectangle&    GetRect() const         { return maRect; }
    void                SetRect( const Point& rPos, const Size& rSz );
    void                SetRect( const Rectangle& rRect )
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
namespace com { namespace sun { namespace star {
    namespace drawing {
        class XDrawPage;
        class XShape;
    }
    namespace task {
        class XStatusIndicator;
    }
}}}
class EscherExHostAppData;

class ImplEESdrWriter
{
protected:
    EscherEx*           mpEscherEx;
    MapMode             maMapModeSrc;
    MapMode             maMapModeDest;

    css::uno::Reference< css::task::XStatusIndicator >    mXStatusIndicator;
    css::uno::Reference< css::drawing::XDrawPage >        mXDrawPage;
    css::uno::Reference< css::drawing::XShapes >          mXShapes;

    SvStream*           mpPicStrm;

    // own extensions

    EscherExHostAppData*    mpHostAppData;

    sal_uInt32              mnPagesWritten;

    sal_uInt32              mnShapeMasterTitle;
    sal_uInt32              mnShapeMasterBody;

    // per page values
    sal_uInt32              mnIndices;
    sal_uInt32              mnOutlinerCount;
    sal_uInt32              mnPrevTextStyle;
    sal_uInt32              mnStatMaxValue;

    sal_uInt16              mnEffectCount;

    bool                    mbIsTitlePossible;
    bool                    mbStatusIndicator;
    bool                    mbStatus;


    explicit ImplEESdrWriter( EscherEx& rEx );

    bool                ImplInitPageValues();

    void                ImplWritePage(
                            EscherSolverContainer& rSolver,
                            ImplEESdrPageType ePageType,
                            bool bBackGround = false );

    sal_uInt32          ImplWriteShape( ImplEESdrObject& rObj,
                            EscherSolverContainer& rSolver,
                            ImplEESdrPageType ePageType, const bool bOOxmlExport = false );  // returns ShapeID

    static void         ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
    void                ImplWriteAdditionalText(
                                        ImplEESdrObject& rObj,
                                        const Point& rTextRefPoint );
    sal_uInt32          ImplEnterAdditionalTextGroup(
                                const css::uno::Reference< css::drawing::XShape >& rShape,
                                const Rectangle* pBoundRect = NULL );


public:
    Point               ImplMapPoint( const Point& rPoint );
    Size                ImplMapSize( const Size& rSize );
    EscherExHostAppData* ImplGetHostData() { return mpHostAppData; }
};

class SdrObject;
class SdrPage;

class ImplEscherExSdr : public ImplEESdrWriter
{
private:
    const SdrPage*          mpSdrPage;
    EscherSolverContainer*  mpSolverContainer;

public:
    explicit            ImplEscherExSdr( EscherEx& rEx );
    virtual             ~ImplEscherExSdr();

    bool                ImplInitPage( const SdrPage& rPage );
    bool                ImplInitUnoShapes( const css::uno::Reference< css::drawing::XShapes >& rxShapes );
    void                ImplWriteCurrentPage();

    sal_uInt32          ImplWriteTheShape( ImplEESdrObject& rObj, bool ooxmlExport = false );

    void                ImplExitPage();
    void                ImplFlushSolverContainer();
};



#endif // INCLUDED_FILTER_SOURCE_MSFILTER_ESCHESDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
