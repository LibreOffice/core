/*************************************************************************
 *
 *  $RCSfile: eschesdo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: cmc $ $Date: 2001-09-18 09:52:58 $
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
        BOOL                mbStatus;
        UINT32              mnStatMaxValue;
        MapMode             maMapModeSrc;
        MapMode             maMapModeDest;

        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >    mXStatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >        mXDrawPage;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >          mXShapes;
        BOOL                mbStatusIndicator;

        UINT32              mnPagesWritten;

        UINT32              mnShapeMasterTitle;
        UINT32              mnShapeMasterBody;

        SvStream*           mpPicStrm;


        // own extensions

        EscherExHostAppData*    mpHostAppData;

        // per page values
        UINT32              mnIndices;
        UINT32              mnOutlinerCount;
        UINT32              mnPrevTextStyle;
        UINT16              mnEffectCount;
        BOOL                mbIsTitlePossible;


                                ImplEESdrWriter( EscherEx& rEx );

            BOOL                ImplInitPageValues();

            void                ImplWritePage(
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    BOOL bBackGround = FALSE );

            UINT32              ImplWriteShape( ImplEESdrObject& rObj,
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );  // returns ShapeID

            void                ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt,
                                                    const Point& rRefPoint );
            BOOL                ImplGetText( ImplEESdrObject& rObj );
            void                ImplWriteTextBundle( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
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
