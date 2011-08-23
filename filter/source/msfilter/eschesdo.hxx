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
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >			mXShape;
//	XTextRef			mXText;	// TextRef des globalen Text
    ::com::sun::star::uno::Any				mAny;
    Rectangle			maRect;
    String				mType;
    UINT32				mnShapeId;
    UINT32				mnTextSize;
    INT32				mnAngle;
    BOOL 				mbValid : 1;
    BOOL				mbPresObj : 1;
    BOOL				mbEmptyPresObj : 1;

    void Init( ImplEESdrWriter& rEx );
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > 	mXPropSet;

    ImplEESdrObject( ImplEscherExSdr& rEx, const SdrObject& rObj );
    ImplEESdrObject( ImplEESdrWriter& rEx, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    BOOL ImplGetPropertyValue( const sal_Unicode* pString );

    INT32 ImplGetInt32PropertyValue( const sal_Unicode* pStr, UINT32 nDef = 0 )
    { return ImplGetPropertyValue( pStr ) ? *(INT32*)mAny.getValue() : nDef; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&	GetShapeRef() const 	{ return mXShape; }
    const ::com::sun::star::uno::Any&		GetUsrAny() const		{ return mAny; }
    const String&		GetType() const 		{ return mType; }
    void				SetType( const String& rS ) { mType = rS; }

    const Rectangle&	GetRect() const 		{ return maRect; }
    void				SetRect( const Point& rPos, const Size& rSz );
    void				SetRect( const Rectangle& rRect )
                            { maRect = rRect; }

    INT32				GetAngle() const 		{ return mnAngle; }
    void				SetAngle( INT32 nVal ) 	{ mnAngle = nVal; }

    UINT32				GetTextSize() const 	{ return mnTextSize; }

    BOOL 				IsValid() const 		{ return mbValid; }
    BOOL				IsPresObj() const 		{ return mbPresObj; }
    BOOL				IsEmptyPresObj() const 	{ return mbEmptyPresObj; }
    UINT32				GetShapeId() const 		{ return mnShapeId; }
    void 				SetShapeId( UINT32 nVal ) { mnShapeId = nVal; }

    const SdrObject*	GetSdrObject() const;

    UINT32 				ImplGetText();
    BOOL 				ImplHasText() const;
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
class Polygon;

class ImplEESdrWriter
{
protected:
        EscherEx*			mpEscherEx;
        MapMode				maMapModeSrc;
        MapMode				maMapModeDest;

        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >	mXStatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >		mXDrawPage;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >			mXShapes;

        SvStream*			mpPicStrm;

        // own extensions

        EscherExHostAppData*	mpHostAppData;

        UINT32				mnPagesWritten;

        UINT32				mnShapeMasterTitle;
        UINT32				mnShapeMasterBody;

        // per page values
        UINT32				mnIndices;
        UINT32				mnOutlinerCount;
        UINT32				mnPrevTextStyle;
        UINT32				mnStatMaxValue;

        UINT16				mnEffectCount;

        BOOL				mbIsTitlePossible;
        BOOL				mbStatusIndicator;
        BOOL				mbStatus;


                                ImplEESdrWriter( EscherEx& rEx );

            BOOL				ImplInitPageValues();

            void				ImplWritePage(
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    BOOL bBackGround = FALSE );

            UINT32				ImplWriteShape( ImplEESdrObject& rObj,
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );	// returns ShapeID

            void				ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
            BOOL				ImplGetText( ImplEESdrObject& rObj );
            void				ImplWriteAdditionalText(
                                                ImplEESdrObject& rObj,
                                                const Point& rTextRefPoint );
            UINT32				ImplEnterAdditionalTextGroup(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
                                        const Rectangle* pBoundRect = NULL );


public:
            Point				ImplMapPoint( const Point& rPoint );
            Size				ImplMapSize( const Size& rSize );
            EscherExHostAppData* ImplGetHostData() { return mpHostAppData; }
            void MapRect(ImplEESdrObject& rObj);
};


// ===================================================================

class SdrObject;
class SdrPage;

class ImplEscherExSdr : public ImplEESdrWriter
{
private:
        const SdrPage*			mpSdrPage;
        EscherSolverContainer*	mpSolverContainer;

public:
                                ImplEscherExSdr( EscherEx& rEx );
    virtual						~ImplEscherExSdr();

            bool                ImplInitPage( const SdrPage& rPage );
            bool                ImplInitUnoShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );
            void				ImplWriteCurrentPage();

            UINT32				ImplWriteTheShape( ImplEESdrObject& rObj );

            void				ImplExitPage();
            void				ImplFlushSolverContainer();
};



#endif // _ESCHESDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
