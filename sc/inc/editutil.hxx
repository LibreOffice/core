/*************************************************************************
 *
 *  $RCSfile: editutil.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-25 17:30:32 $
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

#ifndef SC_EDITUTIL_HXX
#define SC_EDITUTIL_HXX


#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif

#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif

#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif

#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif


class OutputDevice;
class ScDocument;
class ScPatternAttr;

class ScEditUtil
{
    ScDocument*     pDoc;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
    Point           aScrPos;
    OutputDevice*   pDev;           // MapMode muss eingestellt sein
    double          nPPTX;
    double          nPPTY;
    Fraction        aZoomX;
    Fraction        aZoomY;

    static const char __FAR_DATA pCalcDelimiters[];

public:
    static String ModifyDelimiters( const String& rOld );
    static String GetSpaceDelimitedString( const EditEngine& rEngine );

public:
                ScEditUtil( ScDocument* pDocument, USHORT nX, USHORT nY, USHORT nZ,
                            const Point& rScrPosPixel,
                            OutputDevice* pDevice, double nScaleX, double nScaleY,
                            const Fraction& rX, const Fraction& rY ) :
                    pDoc(pDocument),nCol(nX),nRow(nY),nTab(nZ),
                    aScrPos(rScrPosPixel),pDev(pDevice),
                    nPPTX(nScaleX),nPPTY(nScaleY),aZoomX(rX),aZoomY(rY) {}

    Rectangle   GetEditArea( const ScPatternAttr* pPattern, BOOL bForceToTop );
};


class ScEditAttrTester
{
    EditEngine* pEngine;
    SfxItemSet* pEditAttrs;
    BOOL        bNeedsObject;
    BOOL        bNeedsCellAttr;

public:
                ScEditAttrTester( EditEngine* pEng );
                ~ScEditAttrTester();

    BOOL                NeedsObject() const     { return bNeedsObject; }
    BOOL                NeedsCellAttr() const   { return bNeedsCellAttr; }
    const SfxItemSet&   GetAttribs() const      { return *pEditAttrs; }
};


// construct pool before constructing EditEngine, destroy pool after EditEngine
class ScEnginePoolHelper
{
protected:
    SfxItemPool*    pEnginePool;
    SfxItemSet*     pDefaults;
    BOOL            bDeleteEnginePool;
    BOOL            bDeleteDefaults;

                    ScEnginePoolHelper( SfxItemPool* pEnginePool,
                        BOOL bDeleteEnginePool = FALSE );
                    ScEnginePoolHelper( const ScEnginePoolHelper& rOrg );
    virtual         ~ScEnginePoolHelper();
};


class ScEditEngineDefaulter : public ScEnginePoolHelper, public EditEngine
{
public:
                    /// bDeleteEnginePool: Engine becomes the owner of the pool
                    /// and deletes it on destruction
                    ScEditEngineDefaulter( SfxItemPool* pEnginePool,
                        BOOL bDeleteEnginePool = FALSE );
                    /// If rOrg.bDeleteEnginePool: pool gets cloned and will be
                    /// deleted on destruction. Defaults are not set.
                    ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg );
    virtual         ~ScEditEngineDefaulter();

                    /// Creates a copy of SfxItemSet if bRememberCopy set
    void            SetDefaults( const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );

                    /// Becomes the owner of the SfxItemSet if bTakeOwnership set
    void            SetDefaults( SfxItemSet* pDefaults, BOOL bTakeOwnership = TRUE );

                    /// Set the item in the default ItemSet which is created
                    /// if it doesn't exist yet.
                    /// The default ItemSet is then applied to each paragraph.
    void            SetDefaultItem( const SfxPoolItem& rItem );

                    /// Overwritten method to be able to apply defaults already set
    void            SetText( const EditTextObject& rTextObject );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const EditTextObject& rTextObject,
                        const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );

                    /// Overwritten method to be able to apply defaults already set
    void            SetText( const String& rText );
                    /// Current defaults are not applied, new defaults are applied
    void            SetTextNewDefaults( const String& rText,
                        const SfxItemSet& rDefaults, BOOL bRememberCopy = TRUE );
};


// 1/100 mm
class ScTabEditEngine : public ScEditEngineDefaulter
{
private:
    void    Init(const ScPatternAttr& rPattern);
public:
    ScTabEditEngine( ScDocument* pDoc );            // Default
    // pEnginePool = ScDocument.GetEnginePool()
    // pTextObjectPool = ScDocument.GetEditPool()
    ScTabEditEngine( const ScPatternAttr& rPattern,
                    SfxItemPool* pEnginePool,
                    SfxItemPool* pTextObjectPool = NULL );
};


struct ScHeaderFieldData
{
    String      aTitle;             // Titel oder Dateiname wenn kein Titel
    String      aLongDocName;       // Pfad und Dateiname
    String      aShortDocName;      // nur Dateiname
    String      aTabName;
    Date        aDate;
    Time        aTime;
    long        nPageNo;
    long        nTotalPages;
    SvxNumType  eNumType;

    ScHeaderFieldData();
};


// fuer Feldbefehle in der Tabelle
class ScFieldEditEngine : public ScEditEngineDefaulter
{
private:
    BOOL    bExecuteURL;

public:
    // pEnginePool = ScDocument.GetEnginePool()
    // pTextObjectPool = ScDocument.GetEditPool()
    ScFieldEditEngine( SfxItemPool* pEnginePool,
                        SfxItemPool* pTextObjectPool = NULL,
                        BOOL bDeleteEnginePool = FALSE );

    void    SetExecuteURL(BOOL bSet)    { bExecuteURL = bSet; }

    virtual void    FieldClicked( const SvxFieldItem& rField, USHORT, USHORT );
    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );
};


// for headers/footers with fields
class ScHeaderEditEngine : public ScEditEngineDefaulter
{
private:
    ScHeaderFieldData   aData;

public:
    ScHeaderEditEngine( SfxItemPool* pEnginePool, BOOL bDeleteEnginePool = FALSE );
    virtual String CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );

    void SetTitle(const String& rNew)               { aData.aTitle = rNew; }
    void SetLongDocName(const String& rNew)         { aData.aLongDocName = rNew; }
    void SetShortDocName(const String& rNew)        { aData.aShortDocName = rNew; }
    void SetTabName(const String& rNew)             { aData.aTabName = rNew; }
    void SetDate(const Date& rNew)                  { aData.aDate = rNew; }
    void SetTime(const Time& rNew)                  { aData.aTime = rNew; }
    void SetPageNo(long nNew)                       { aData.nPageNo = nNew; }
    void SetTotalPages(long nNew)                   { aData.nTotalPages = nNew; }
    void SetNumType(SvxNumType eNew)                { aData.eNumType = eNew; }
    void SetData(const ScHeaderFieldData& rNew)     { aData = rNew; }
};

//  SvxFieldData-Ableitungen sind nach Svx verschoben


#endif


