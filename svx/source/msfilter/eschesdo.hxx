/*************************************************************************
 *
 *  $RCSfile: eschesdo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
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

struct ImplEESdrSOParagraph
{
    BOOL                bDepth;
    BOOL                bExtendedParameters;
    UINT32              nParaFlags;
    INT16               nBulletFlags;
    String              sPrefix;
    String              sSuffix;
    String              sGraphicUrl;    // String auf eine Graphic
    UINT32              nNumberingType; // in wirlichkeit ist dies ein SvxEnum
    UINT32              nHorzAdjust;
    UINT32              nBulletColor;
    INT32               nFirstOffset;
    INT16               nStartWith;     // Start der nummerierung
    INT16               nLeftMargin;
    INT16               nBulletRealSize;// GroessenVerhaeltnis in Proz
    INT16               nDepth;         // aktuelle tiefe
    sal_Unicode             cBulletId;      // wenn Numbering Type == CharSpecial
    ::com::sun::star::awt::FontDescriptor       aFontDesc;

    ImplEESdrSOParagraph()
    {
        nDepth = 0;
        bDepth = TRUE;
        bExtendedParameters = FALSE;
        nParaFlags = 0;
        nBulletFlags = 0;
    };
};


// -------------------------------------------------------------------

struct ImplEESdrConnectorRule
{
    UINT32  nRuleId;
    UINT32  nShapeA;        // SPID of shape A
    UINT32  nShapeB;        // SPID of shape B
    UINT32  nShapeC;        // SPID of connector shape
    UINT32  ncptiA;         // Connection site Index of shape A
    UINT32  ncptiB;         // Connection site Index of shape B
};


// -------------------------------------------------------------------

class ImplEESdrShapeListEntry
{

    friend class ImplEESdrSolverContainer;

protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       aXShape;
    UINT32          nEscherId;

public:
                    ImplEESdrShapeListEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
                                                UINT32 nId )
                        :aXShape    ( rShape ),
                        nEscherId   ( nId )
                    {}
};


// -------------------------------------------------------------------

class ImplEESdrConnectorListEntry
{

    friend class ImplEESdrSolverContainer;

protected:

    Point           maPointA;
    Point           maPointB;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       mXConnector;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       mXConnectToA;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       mXConnectToB;

public:

    UINT32          GetConnectorRule( BOOL bFirst );
                    ImplEESdrConnectorListEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rC,
                                    const Point& rPA, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rSA,
                                    const Point& rPB, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rSB )
                        : mXConnector( rC ),
                        maPointA    ( rPA ),
                        maPointB    ( rPB ),
                        mXConnectToA( rSA ),
                        mXConnectToB( rSB )
                    {}
};


// -------------------------------------------------------------------

class EscherEx;
class ImplEESdrSolverContainer
{
    List                maShapeList;
    List                maConnectorList;

    UINT32              ImplGetId( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape );

public:
    void                AddShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&, UINT32 nId );
    void                AddConnector( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rConnector,
                                    const Point& rA, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rConA,
                                    const Point& rB, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rConB );

    void                WriteSolver( EscherEx& );

                        ImplEESdrSolverContainer(){};
                        ~ImplEESdrSolverContainer();
};


// -------------------------------------------------------------------

class ImplEESdrWriter;
class ImplEscherExSdr;

class ImplEESdrObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >           mXShape;
//  XTextRef            mXText; // TextRef des globalen Text
    ::com::sun::star::uno::Any              mAny;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   mXPropSet;
    Size                maSize;
    Point               maPosition;
    Rectangle           maRect;
    String              mType;
    UINT32              mnShapeId;
    UINT32              mnTextSize;
    INT32               mnAngle;
    INT32               mnShadow;   // eq 0 if fillstyle && linestyle == None
    BOOL                mbValid : 1;
    BOOL                mbPresObj : 1;
    BOOL                mbEmptyPresObj : 1;

    void Init( ImplEESdrWriter& rEx );
public:

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

    const Size&         GetSize() const         { return maSize; }
    const Point&        GetPos() const          { return maPosition; }
    const Rectangle&    GetRect() const         { return maRect; }
    void                SetRect( const Point& rPos, const Size& rSz );
    void                SetRect( const Rectangle& rRect )
                            { maRect = rRect; }

    INT32               GetAngle() const        { return mnAngle; }
    void                SetAngle( INT32 nVal )  { mnAngle = nVal; }

    INT32               GetShadow() const       { return mnShadow; }
    void                SetShadow( INT32 nVal ) { mnShadow = nVal; }

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
        UINT32              mnTextStyle;

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
                                    ImplEESdrSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    BOOL bBackGround = FALSE );
//          BOOL                ImplGetShapeByIndex( UINT32 nIndex, BOOL bGroup = FALSE );

            UINT32              ImplWriteShape( ImplEESdrObject& rObj,
                                    ImplEESdrSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );  // returns ShapeID

            void                ImplFlipBoundingBox( ImplEESdrObject& rObj,
                                                    const Point& rRefPoint );
            BOOL                ImplGetText( ImplEESdrObject& rObj );
            void                ImplWriteLineBundle( ImplEESdrObject& rObj,
                                                     BOOL bEdge );
            void                ImplWriteFillBundle( ImplEESdrObject& rObj,
                                                     BOOL bEdge );
            void                ImplWriteTextBundle( ImplEESdrObject& rObj );
            void                ImplWriteAny( ImplEESdrObject& rObj,
                                                UINT32 nFlags, BOOL bBezier,
                                                Polygon* pPolygon = NULL );
            BOOL                ImplIsMetaFile( ImplEESdrObject& rObj );
            BOOL                ImplGetGraphic( ImplEESdrObject& rObj,
                                            const sal_Unicode*, BOOL bFillBitmap );
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
};


// ===================================================================

class SdrObject;
class SdrPage;

class ImplEscherExSdr : public ImplEESdrWriter
{
private:
        const SdrPage*      mpSdrPage;
        ImplEESdrSolverContainer*   mpSolverContainer;

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
