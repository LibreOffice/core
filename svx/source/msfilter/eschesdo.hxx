/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eschesdo.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:18:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _ESCHESDO_HXX
#define _ESCHESDO_HXX
#ifndef _SVX_ESCHEREX_HXX
#include "escherex.hxx"
#endif
#ifndef _SVX_UNOWPAGE_HXX //autogen wg. SvxDrawPage
#include "unopage.hxx"
#endif

#ifndef _SV_MAPMOD_HXX //autogen wg. MapMode
#include <vcl/mapmod.hxx>
#endif

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
    UINT32              mnShapeId;
    UINT32              mnTextSize;
    INT32               mnAngle;
    BOOL                mbValid : 1;
    BOOL                mbPresObj : 1;
    BOOL                mbEmptyPresObj : 1;

    void Init( ImplEESdrWriter& rEx );
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   mXPropSet;

    ImplEESdrObject( ImplEscherExSdr& rEx, const SdrObject& rObj );
    ImplEESdrObject( ImplEESdrWriter& rEx, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    BOOL ImplGetPropertyValue( const sal_Unicode* pString );

    INT32 ImplGetInt32PropertyValue( const sal_Unicode* pStr, UINT32 nDef = 0 )
    { return ImplGetPropertyValue( pStr ) ? *(INT32*)mAny.getValue() : nDef; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&    GetShapeRef() const     { return mXShape; }
    const ::com::sun::star::uno::Any&       GetUsrAny() const       { return mAny; }
    const String&       GetType() const         { return mType; }
    void                SetType( const String& rS ) { mType = rS; }

    const Rectangle&    GetRect() const         { return maRect; }
    void                SetRect( const Point& rPos, const Size& rSz );
    void                SetRect( const Rectangle& rRect )
                            { maRect = rRect; }

    INT32               GetAngle() const        { return mnAngle; }
    void                SetAngle( INT32 nVal )  { mnAngle = nVal; }

    UINT32              GetTextSize() const     { return mnTextSize; }

    BOOL                IsValid() const         { return mbValid; }
    BOOL                IsPresObj() const       { return mbPresObj; }
    BOOL                IsEmptyPresObj() const  { return mbEmptyPresObj; }
    UINT32              GetShapeId() const      { return mnShapeId; }
    void                SetShapeId( UINT32 nVal ) { mnShapeId = nVal; }

    const SdrObject*    GetSdrObject() const;

    UINT32              ImplGetText();
    BOOL                ImplHasText() const;
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
        EscherEx*           mpEscherEx;
        MapMode             maMapModeSrc;
        MapMode             maMapModeDest;

        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >    mXStatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >        mXDrawPage;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >          mXShapes;

        SvStream*           mpPicStrm;

        // own extensions

        EscherExHostAppData*    mpHostAppData;

        UINT32              mnPagesWritten;

        UINT32              mnShapeMasterTitle;
        UINT32              mnShapeMasterBody;

        // per page values
        UINT32              mnIndices;
        UINT32              mnOutlinerCount;
        UINT32              mnPrevTextStyle;
        UINT32              mnStatMaxValue;

        UINT16              mnEffectCount;

        BOOL                mbIsTitlePossible;
        BOOL                mbStatusIndicator;
        BOOL                mbStatus;


                                ImplEESdrWriter( EscherEx& rEx );

            BOOL                ImplInitPageValues();

            void                ImplWritePage(
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    BOOL bBackGround = FALSE );

            UINT32              ImplWriteShape( ImplEESdrObject& rObj,
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );  // returns ShapeID

            void                ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
            BOOL                ImplGetText( ImplEESdrObject& rObj );
            void                ImplWriteAdditionalText(
                                                ImplEESdrObject& rObj,
                                                const Point& rTextRefPoint );
            UINT32              ImplEnterAdditionalTextGroup(
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

            SvxDrawPage*        ImplInitPage( const SdrPage& rPage );
            void                ImplWriteCurrentPage();

            UINT32              ImplWriteTheShape( ImplEESdrObject& rObj );

            void                ImplExitPage();
            void                ImplFlushSolverContainer();
};



#endif // _ESCHESDO_HXX
