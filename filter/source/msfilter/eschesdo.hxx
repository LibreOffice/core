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
#ifndef _ESCHESDO_HXX
#define _ESCHESDO_HXX
#include <filter/msfilter/escherex.hxx>
#include <svx/unopage.hxx>
#include <vcl/mapmod.hxx>

// ===================================================================
// fractions of Draw PPTWriter etc.

enum ImplEESdrPageType { NORMAL = 0, MASTER = 1, NOTICE = 2, UNDEFINED = 3 };

class ImplEESdrWriter;
class ImplEscherExSdr;

class ImplEESdrObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >           mXShape;
//  XTextRef            mXText; // TextRef des globalen Text
    ::com::sun::star::uno::Any              mAny;
    Rectangle           maRect;
    String              mType;
    sal_uInt32              mnShapeId;
    sal_uInt32              mnTextSize;
    sal_Int32               mnAngle;
    sal_Bool                mbValid : 1;
    sal_Bool                mbPresObj : 1;
    sal_Bool                mbEmptyPresObj : 1;

    void Init( ImplEESdrWriter& rEx );
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   mXPropSet;

    ImplEESdrObject( ImplEscherExSdr& rEx, const SdrObject& rObj );
    ImplEESdrObject( ImplEESdrWriter& rEx, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    sal_Bool ImplGetPropertyValue( const sal_Unicode* pString );
    sal_Bool ImplGetPropertyValue( const rtl::OUString& rString ) { return ImplGetPropertyValue(rString.getStr()); }

    sal_Int32 ImplGetInt32PropertyValue( const sal_Unicode* pStr, sal_uInt32 nDef = 0 )
    { return ImplGetPropertyValue( pStr ) ? *(sal_Int32*)mAny.getValue() : nDef; }
    sal_Int32 ImplGetInt32PropertyValue( const rtl::OUString& rStr, sal_uInt32 nDef = 0 )
    { return ImplGetInt32PropertyValue(rStr.getStr(), nDef); }

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&    GetShapeRef() const     { return mXShape; }
    const ::com::sun::star::uno::Any&       GetUsrAny() const       { return mAny; }
    const String&       GetType() const         { return mType; }
    void                SetType( const String& rS ) { mType = rS; }

    const Rectangle&    GetRect() const         { return maRect; }
    void                SetRect( const Point& rPos, const Size& rSz );
    void                SetRect( const Rectangle& rRect )
                            { maRect = rRect; }

    sal_Int32               GetAngle() const        { return mnAngle; }
    void                SetAngle( sal_Int32 nVal )  { mnAngle = nVal; }

    sal_uInt32              GetTextSize() const     { return mnTextSize; }

    sal_Bool                IsValid() const         { return mbValid; }
    sal_Bool                IsPresObj() const       { return mbPresObj; }
    sal_Bool                IsEmptyPresObj() const  { return mbEmptyPresObj; }
    sal_uInt32              GetShapeId() const      { return mnShapeId; }
    void                SetShapeId( sal_uInt32 nVal ) { mnShapeId = nVal; }

    const SdrObject*    GetSdrObject() const;

    sal_uInt32              ImplGetText();
    sal_Bool                ImplHasText() const;
};



// -------------------------------------------------------------------
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

        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >    mXStatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >        mXDrawPage;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >          mXShapes;

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

        sal_Bool                mbIsTitlePossible;
        sal_Bool                mbStatusIndicator;
        sal_Bool                mbStatus;


                                ImplEESdrWriter( EscherEx& rEx );

            sal_Bool                ImplInitPageValues();

            void                ImplWritePage(
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    sal_Bool bBackGround = sal_False );

            sal_uInt32              ImplWriteShape( ImplEESdrObject& rObj,
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );  // returns ShapeID

            void                ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
            sal_Bool                ImplGetText( ImplEESdrObject& rObj );
            void                ImplWriteAdditionalText(
                                                ImplEESdrObject& rObj,
                                                const Point& rTextRefPoint );
            sal_uInt32              ImplEnterAdditionalTextGroup(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
                                        const Rectangle* pBoundRect = NULL );


public:
            Point               ImplMapPoint( const Point& rPoint );
            Size                ImplMapSize( const Size& rSize );
            EscherExHostAppData* ImplGetHostData() { return mpHostAppData; }
            void MapRect(ImplEESdrObject& rObj);
};


// ===================================================================

class SdrObject;
class SdrPage;

class ImplEscherExSdr : public ImplEESdrWriter
{
private:
        const SdrPage*          mpSdrPage;
        EscherSolverContainer*  mpSolverContainer;

public:
                                ImplEscherExSdr( EscherEx& rEx );
    virtual                     ~ImplEscherExSdr();

            bool                ImplInitPage( const SdrPage& rPage );
            bool                ImplInitUnoShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );
            void                ImplWriteCurrentPage();

            sal_uInt32              ImplWriteTheShape( ImplEESdrObject& rObj );

            void                ImplExitPage();
            void                ImplFlushSolverContainer();
};



#endif // _ESCHESDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
