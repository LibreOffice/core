/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _ESCHESDO_HXX
#define _ESCHESDO_HXX
#include <filter/msfilter/escherex.hxx>
#include <svx/unopage.hxx>
#include <vcl/mapmod.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

// ===================================================================
// fractions of Draw PPTWriter etc.

enum ImplEESdrPageType { NORMAL = 0, MASTER = 1, NOTICE = 2, UNDEFINED = 3 };

class ImplEESdrWriter;
class ImplEscherExSdr;

class ImplEESdrObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >           mXShape;
//  XTextRef            mXText; // TextRef des globalen Text
    ::com::sun::star::uno::Any              mAny;

    // the object range, split in pos and scale to keep the evtl. negative size (mirroring)
    basegfx::B2DRange   maObjectRange;
    basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose   maObjTrans;

    String              mType;
    sal_uInt32          mnShapeId;
    sal_uInt32          mnTextSize;
    sal_Int32           mnAngle;

    /// bitfield
    bool                mbValid : 1;
    bool                mbPresObj : 1;
    bool                mbEmptyPresObj : 1;

    void Init( ImplEESdrWriter& rEx );
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   mXPropSet;

    ImplEESdrObject( ImplEscherExSdr& rEx, const SdrObject& rObj );
    ImplEESdrObject( ImplEESdrWriter& rEx, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape );
    ~ImplEESdrObject();

    bool ImplGetPropertyValue( const sal_Unicode* pString );

    sal_Int32 ImplGetInt32PropertyValue( const sal_Unicode* pStr, sal_uInt32 nDef = 0 )
    { return ImplGetPropertyValue( pStr ) ? *(sal_Int32*)mAny.getValue() : nDef; }

    sal_Bool ImplGetPropertyValue( const rtl::OUString& rStr ) { return ImplGetPropertyValue( rStr.getStr() ); }
    sal_Bool ImplGetInt32PropertyValue( const rtl::OUString& rStr ) { return ImplGetInt32PropertyValue( rStr.getStr() ); }

    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&    GetShapeRef() const     { return mXShape; }
    const ::com::sun::star::uno::Any&       GetUsrAny() const       { return mAny; }
    const String&       GetType() const         { return mType; }
    void                SetType( const String& rS ) { mType = rS; }

    void setObjectRange(const basegfx::B2DRange& rObjectRange);
    const basegfx::B2DRange& getObjectRange() const;

    sal_Int32 GetAngle() const;
    void SetAngle(sal_Int32 nVal);

    sal_uInt32              GetTextSize() const     { return mnTextSize; }

    bool                IsValid() const         { return mbValid; }
    bool                IsPresObj() const       { return mbPresObj; }
    bool                IsEmptyPresObj() const  { return mbEmptyPresObj; }
    sal_uInt32              GetShapeId() const      { return mnShapeId; }
    void                SetShapeId( sal_uInt32 nVal ) { mnShapeId = nVal; }

    const SdrObject*    GetSdrObject() const;

    sal_uInt32              ImplGetText();
    bool                ImplHasText() const;

    basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& getTransform();
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
        basegfx::B2DHomMatrix   maLogicToLogic;

        void implPrepareLogicToLogic();

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

        /// bitfield
        bool                mbIsTitlePossible : 1;
        bool                mbStatusIndicator : 1;
        bool                mbStatus : 1;


                                ImplEESdrWriter( EscherEx& rEx );

            bool                ImplInitPageValues();

            void                ImplWritePage(
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType,
                                    bool bBackGround = false );

            sal_uInt32              ImplWriteShape( ImplEESdrObject& rObj,
                                    EscherSolverContainer& rSolver,
                                    ImplEESdrPageType ePageType );  // returns ShapeID

            void                ImplHandleRotation( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt );
            bool                ImplGetText( ImplEESdrObject& rObj );
            void                ImplWriteAdditionalText(
                                                ImplEESdrObject& rObj,
                                                const Point& rTextRefPoint );
            sal_uInt32              ImplEnterAdditionalTextGroup(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
                                        const basegfx::B2DRange* pObjectRange = 0);


public:
            basegfx::B2DPoint ImplMapB2DPoint( const basegfx::B2DPoint& rPoint );
            basegfx::B2DVector ImplMapB2DVector( const basegfx::B2DVector& rScale );
            basegfx::B2DRange ImplMapB2DRange(const basegfx::B2DRange& rRange);

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
