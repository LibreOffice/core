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
#ifndef SCH_CHART_AXIS_HXX
#define SCH_CHART_AXIS_HXX

//STRIP008 class ChartModel;
//STRIP008 class SdrObject;
//STRIP008 class SdrObjList;
//STRIP008 class XPolygon;
//STRIP008 class SdrRectObj;
//STRIP008 class SchAxisObj;

#ifndef _SVX_CHRTITEM_HXX //autogen
#include <bf_svx/chrtitem.hxx>
#endif
#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#include "defines.hxx"
#include "schattr.hxx"
#include "float.h"
#include <math.h>

namespace binfilter {

class SvNumberFormatter;
class SvNumberFormatterIndexTable;
class ChartModel;
class SdrObject;
class SdrObjList;
class XPolygon;
class SdrRectObj;
class SchAxisObj;

#define CHAXIS_POS_A -1   //Top or Right    (normale Position)
#define CHAXIS_POS_B -2	  //Bottom or Left  (Position der 2.Achse)

#define CHAXIS_MARK_BOTH   3   //eigentlich sind es flags....
#define CHAXIS_MARK_OUTER  2
#define CHAXIS_MARK_INNER  1
#define CHAXIS_MARK_NONE   0
#define CHAXIS_MARK_ASMAIN 4  //Flag indiziert, da?Helpticks wie mainticks ausgerichtete werden sollen

#define CHAXIS_AXIS_UNKNOWN 0
#define CHAXIS_AXIS_X	1
#define CHAXIS_AXIS_Y	2
#define CHAXIS_AXIS_Z	3
#define CHAXIS_AXIS_B	4	// secondary Y axis
#define CHAXIS_AXIS_A	5	// secondary X axis

class ChartAxis
{
protected:

    BOOL mbInnerPos;
    long mnInnerPos;
    BOOL mbColText;    // Texte aus Datenspalte statt per Numberformatter generieren
    BOOL mbCenterText; // mbCenterText: FALSE -> Beschriftung wie z.B. Barchart, d.h. unter
                       // der ganzen Spalte, TRUE: wie z.B. Area, zentriert unter Datenpunkt

    long mnMaxTextWidth;

    long mnAlternateId;
    long mbAlternativIdUsed;

    //Statistik bzw. Daten in Prozent: Summe je Spalte der Elemente, die an der Achse skalieren
    BOOL			   mbPercentCol;
    BOOL	           mbTextOverlap;
    double			   *mpTotal;
    long 			   mnTotalSize;
    BOOL			   mbTotalActual;
    BOOL			   mbTotalAlloc;
    void			   ResizeTotal(long nSize);
    void			   CreateTotal();

    double	 		   *mpColStack;
    BOOL			   mbColStackOK;

    long			   mnUId;
    BOOL			   mbShowDescr;
    SfxItemSet*		   mpAllAxisAttr;
    SvNumberFormatter* mpNumFormatter;

    SdrObjList* mpAxisList;
    SchAxisObj* mpAxisObj;
    
    SdrObjList* mpMainGridList;
    SdrObjList* mpHelpGridList;
    SfxItemSet* mpMainGridAttr;
    SfxItemSet* mpHelpGridAttr;

    long mnIdMainGrid;
    long mnIdHelpGrid;

    SfxItemSet* mpGridAttr;
    SfxItemSet* mpTextAttr;

    Rectangle maRectA,maRectB;

    long mnTicks;
    long mnHelpTicks;
    long mnTickLen;
    long mnHelpTickLen;
    long mnPos;
    long mnTextPos;
    BOOL mbSecondary;

    BOOL				mbStepValue;
    BOOL				mbValueDown;

    void RecalcTextPos();

    Size maMaxTextSize;
    long meStackMode;

    long mnId;        //X,Y oder Z, (CHOBJID)
    BOOL mbFlippedXY; //TRUE: X vertikal, Y horizontal; (entspricht in etwa bSwitchColRow...)
    long mnPosition;  //links/rechts bzw. oben/unten

    BOOL mbRadial;    //NetChart, evtl. einmal auch für PolarPlots
    Rectangle maArea; //Dieser Bereich wird von den Achsen umschlossen
    Rectangle maRefArea;

    BOOL mbPercent;

    SfxItemSet* mpAxisAttr;
    ChartModel* mpModel;

    //Elementare Groessen: Min,Max,Markierung,Ursprung
    double  mfMin;
    double  mfMax;
    double  mfStep;
    double  mfStepHelp;
    double  mfOrigin;


    double mfDataBottom; //stacked Charts
    double mfDataTop;


    BOOL mbAutoMin;
    BOOL mbAutoMax;
    BOOL mbAutoOrigin;

    BOOL mbAutoStep;
    BOOL mbAutoStepHelp;

    BOOL mbLogarithm;

    SvxChartTextOrient meTextOrient;

    void CalcMinMaxValue();
    void SetDefaults();

    long GetLength() const;

    inline double GetRowError(long nRow);
    inline BOOL IsDataOnAxis(long nRow);

    void CreateTextAttr();
    void   InitColStacking(long nColCnt);

public:


    long GetDescrWidth();
    void SetColTextMode(BOOL bColText,BOOL bCenterText)
    {
        mbColText    = bColText;
        mbCenterText = bCenterText;
    }
    //Hilfsfunktion für X-Achse, nur in Create2DBackplane
    void InsertHelpTick(long nPos) {InsertMark(nPos,mnHelpTickLen,mnHelpTicks);}
    BOOL HasHelpTicks() {return mnHelpTicks!=0;}

    BOOL AttachIfNoOwnData(const ChartAxis* pAxis);

    void SetPercentMode(BOOL bPercentCol=TRUE)
    {
        if(mbPercentCol!=bPercentCol)
            mbTotalActual=FALSE;
        mbPercentCol=bPercentCol;
    }

    void IfNoDataLookAt(long nAlternateAxisId)		{ mnAlternateId = nAlternateAxisId; }

    const Rectangle& GetArea()						{ return maRefArea; }
    double StackColData(double fData,long nCol,long nColCnt);

    inline double	   GetData(long nCol,long nRow);
    double			   Data2Percent(double fData,long nCol,long nRow);
    double			   GetTotal(long n);

    void Update()									{ ReadAutoAttr(); ReadAttr(); }
    void FillItemSet()								{ GetMembersAsAttr(*mpAxisAttr); }

    BOOL IsVertical();
    BOOL IsNegative()								{ return (mfMax <= 0.0 && mfMin < 0.0 ); }

    // Stack stapelt den Wert an der entsprechenden Achse
    // und gibt die Position (Start und Ende) relativ zum Koordinatensystem zurück
    // Pair::A() für Top/Left, B() für Bottom/Right
    Pair Stack(double fData,BOOL bConstrained=TRUE);//stacked Charts
    void InitStacking()								{ mfDataBottom=mfDataTop=0.0; }
    //void Create();
    //void PreDestruct();

    // Top/Right Position
    long GetUpper(double fData,BOOL bConstrained=TRUE);
    // Bottom/Left Position
    long GetLower(double fData,BOOL bConstrained=TRUE);

    long GetPos(double fData);
    long GetPosOrigin()								{ return GetPos(mfOrigin); }
    BOOL SetArea(const Rectangle& rRect);
    void LoadMemberCompat(SvStream& rIn);
    void StoreMemberCompat(SvStream& rOut) const;

    ChartAxis(ChartModel* pModel,long nId,long nUid);
    virtual ~ChartAxis();

    BOOL IsOriginInRange() const;


    double NumStepsMain();

    void SetAttributes(const SfxItemSet &rAttr);
    void SetAttributes(SfxItemSet *pSet);
    SfxItemSet* GetItemSet() const	{ return mpAxisAttr; }
    double GetMax()		 const		{ return mfMax; }
    double GetMin()		 const		{ return mfMin; }
    double GetStep()	 const		{ return mfStep; }
    double GetOrigin()   const		{ return mfOrigin; }
    double GetHelpStep() const		{ return mfStepHelp; }
    void SetMax(double f)			{ mfMax = f; }
    void SetMin(double f)			{ mfMin = f; }
    BOOL IsLogarithm() const		{ return mbLogarithm; }
    /* Freischalten, wenn benötigt:

    BOOL IsAutoMin()   const {return mbAutoMin;}
    BOOL IsAutoMax()   const {return mbAutoMax;}
    BOOL IsAutoOrigin()const {return mbAutoOrigin;}

    void SetOrigin(double f);	{mfOrigin=f;}
    void SetStep(double f);		{mfStep=f;}
    void SetHelpStep(double f);	{mfStepHelp=f;}
    void SetLogarithmic(BOOL b) {mbLogarithm=b;}
    void SetAutoMin(BOOL b)		{mbAutoMin=b;}
    void SetAutoMax(BOOL b)		{mbAutoMax=b;}
    void SetAutoOrigin(BOOL b)  {mbAutoOrigin=b;}
    */
    void Initialise(const Rectangle &rRect,BOOL bFlippedXY=FALSE,long eStackmode=CHSTACK_NONE,BOOL bPercent=FALSE,BOOL bRadial=FALSE,BOOL bPercentCol=TRUE);

    void VerifySteps();
    void CalcValueSteps();
    BOOL GetMinMaxFromData();

    inline double CalcFact(const double fValue) const
    {
        if (fValue == DBL_MIN)
        {
            return DBL_MIN;
        }
        else if (mfMin == mfMax)
        {
            return 0.0;
        }
        else if (mbLogarithm)
        {
            double fVal = (fValue > 0.0) ? log10(fValue) : log10(mfMin);
            return (fVal - log10(mfMin)) / (log10(mfMax) - log10(mfMin));
        }
        else
        {
            return (fValue - mfMin) / (mfMax - mfMin);
        }
    }

    double CalcFactOrigin()		{ return CalcFact(mfOrigin); }

    //ToDo: Zusammenfassen:
    void ReadAutoAttr();
    void ReadAttr();

    inline BOOL IsVisible() const
    {
        return static_cast< const SfxBoolItem& >( mpAxisAttr->Get( SCHATTR_AXIS_SHOWAXIS ) ).GetValue();
    }
    inline BOOL HasDescription() const
    {
        // meaning should description be displayed, so necessary is IsVisible()
        return static_cast< const SfxBoolItem& >( mpAxisAttr->Get( SCHATTR_AXIS_SHOWDESCR ) ).GetValue() &&
               IsVisible();
    }

    void GetMembersAsAttr(SfxItemSet& rSet) const;

    void InsertMark(long nPos,long nLen,long nWhichTicks);
    UINT32 GetNumFormat()							{ return GetNumFormat( mbPercent ); }
    UINT32 GetNumFormat(BOOL bPercent);
    
    void GridLine(XPolygon& aLine,long nPos);

    void ShowDescr(BOOL b);
    void ShowAxis(BOOL b);

    void DrawGrids();
    void SetMainGrid(SdrObjList* pList,SfxItemSet*pAttr);
    void SetHelpGrid(SdrObjList* pList,SfxItemSet*pAttr);

    long CreateAxis();
    void CreateAxis(const long nPos,BOOL bBorderAxis,BOOL bInnerAxis);
    void CreateAxis(SdrObjList& rList,long nChObjId);
    void SetAllAxisAttr(SfxItemSet *pSet)						{ mpAllAxisAttr = pSet; }
    void SetNumberFormatter( SvNumberFormatter* pFormatter )	{ mpNumFormatter = pFormatter; }
    void SubtractDescrSize(Rectangle& rRect);
    void SubtractDescrSize_X(Rectangle& rRect,const Rectangle& rOldRect); //X-Richtung ist leider etwas anders
    void InitDescr_X();
    void InitDescr(SvxChartTextOrient &rValueOrient,long nMaxTextWidth=-1);
    void InitDescr();

    Size CalcMaxTextSize(SvxChartTextOrient eOrient);
    Size CalcMaxTextSize();

    void CreateMarkDescr(const String& rString, long nPosition, Color *); // #60999# BM
    void CreateMarkDescr(double fData,long nPosition);
    void CreateMarkDescr(SdrRectObj *pTextObj,long nPosition);
    void SetAxisList( SdrObjList *pList );
    void SetGridAttrList(SfxItemSet* pList)	{ mpGridAttr = pList; }
    void SetTextAttrList(SfxItemSet* pList)	{ mpTextAttr = pList; }

    //nach Initialise und SetRefArea (ChartAxis) setzen!!!!!
    //nPos ist entweder eine echte Position (bei Y-Achse der X Wert u.U.)
    //oder ein define für Standardposition (CHAXIS_POS_A) bzw. sekundäre
    //(CHAXIS_POS_B) Achse (= oben bzw. Rechts)
    void SetPosition(long nPos);
    //nDir ist Flag, kann sein CHAXIS_MARK_INNER und CHAXIS_MARK_OUTER (oder beides)
    SdrObject *CreateMarks(long nPosition,long nLen,long nWhichTicks);

    BOOL TranslateMergedNumFormat( SvNumberFormatterIndexTable* pTransTable );
    long GetUniqueId() const    { return mnUId; }

    /** Converts a CHOBJID into an axis UId.

        @return CHAXIS_AXIS_X, CHAXIS_AXIS_Y, CHAXIS_AXIS_Z, CHAXIS_AXIS_A or
                CHAXIS_AXIS_B or CHAXIS_AXIS_UNKNOWN if nObjectId does not refer
                to any valid axis object.
     */
    static long GetUniqueIdByObjectId( long nObjectId );

private:
    /**
        @descr	Calculate the size of the bounding box of a text string.  The 
                size is measured with respect to the coordinate axes.
    
        @param	rAttr	Attributes of the font to use.
        @param	aString	The "typical" description.
    
        @return	Returns the size of the bounding box of the text.
    */
    Size	CalcDescriptionSize	(const SfxItemSet * rAttr,
                                const String & aString);

    /**
        @descr	Calculate the expected size of a "typical" description.  The 
                size is measured along the axis.  The axis orientation is taken
                into account.
        @precond CalcMinMaxValue() has to have been called already.
    
        @param	rAttr	Attributes of the font to use.
    
        @return	Returns the size of the text projected onto the axis.
    */
    long int	CalcTypicalDescriptionSize	(const SfxItemSet * rAttr);
};

} //namespace binfilter
#endif
