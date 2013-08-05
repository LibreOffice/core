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

package util;

import com.sun.star.awt.Point;
import com.sun.star.beans.PropertyValue;
import com.sun.star.drawing.PolygonFlags;
//import util.BitmapLoader;
import com.sun.star.uno.Enum ;
import java.lang.reflect.Field ;
import java.lang.reflect.Method ;
import java.lang.reflect.Modifier ;
import java.lang.reflect.Array ;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;


public class ValueChanger {


 // Method to change a Value, thought for properties
 public static Object changePValue( Object oldValue ) {

   Object newValue = null;

   if (oldValue instanceof com.sun.star.uno.Any) {
     try {
        oldValue = AnyConverter.toObject(((Any) oldValue).getType(),oldValue);
     } catch (com.sun.star.lang.IllegalArgumentException iae) {
     }
   }

   if (oldValue == null)
     return null;

   if (oldValue instanceof Boolean) {
     boolean oldbool = ((Boolean) oldValue).booleanValue();
     newValue = new Boolean(!oldbool);
   } else

   if (oldValue instanceof Integer) {
     int oldint = ((Integer) oldValue).intValue();
     newValue = new Integer(oldint+5);
   } else

   if (oldValue instanceof Long) {
     long oldlong = ((Long) oldValue).longValue();
     newValue = new Long(oldlong + 15);
   } else

   if (oldValue instanceof Short) {
     short oldshort = ((Short) oldValue).shortValue();
     newValue = new Short((short) (oldshort + 1));
   } else

   if (oldValue instanceof Byte) {
     byte oldbyte = ((Byte) oldValue).byteValue();
     newValue = new Byte((byte) (oldbyte + 1));
   } else

   if (oldValue instanceof Float) {
     float oldfloat = ((Float) oldValue).floatValue();
     newValue = new Float((float) (oldfloat + 16.7));
   } else

   if (oldValue instanceof Double) {
     double olddouble = ((Double) oldValue).doubleValue();
     newValue = new Double(olddouble + 17.8);
   } else

   if (oldValue instanceof String) {
     String oldString = (String) oldValue;
     newValue = oldString + "New";
   } else


   if (oldValue instanceof com.sun.star.chart.ChartAxisArrangeOrderType) {
         Object AO1 = com.sun.star.chart.ChartAxisArrangeOrderType.AUTO;
         Object AO2=  com.sun.star.chart.ChartAxisArrangeOrderType.SIDE_BY_SIDE;
         Object AO3=  com.sun.star.chart.ChartAxisArrangeOrderType.STAGGER_EVEN;
         Object AO4=  com.sun.star.chart.ChartAxisArrangeOrderType.STAGGER_ODD;

         if (oldValue.equals(AO1)) newValue = AO2;
         if (oldValue.equals(AO2)) newValue = AO3;
         if (oldValue.equals(AO3)) newValue = AO4;
         if (oldValue.equals(AO4)) newValue = AO1;
   } else

   if (oldValue instanceof com.sun.star.view.PaperOrientation) {
         Object OR1 = com.sun.star.view.PaperOrientation.LANDSCAPE;
         Object OR2 = com.sun.star.view.PaperOrientation.PORTRAIT;

         if (oldValue.equals(OR1)) newValue = OR2;
                             else newValue = OR1;
   } else

   if (oldValue instanceof com.sun.star.lang.Locale) {
         Object Loc1 = new com.sun.star.lang.Locale("en","US","");
         Object Loc2 = new com.sun.star.lang.Locale("de","DE","");

         if (oldValue.equals(Loc1)) newValue = Loc2;
                             else newValue = Loc1;
   } else

   if (oldValue instanceof com.sun.star.style.ParagraphAdjust) {
         Object PA1 = com.sun.star.style.ParagraphAdjust.LEFT;
         Object PA2 = com.sun.star.style.ParagraphAdjust.CENTER;

         if (oldValue.equals(PA1)) newValue = PA2;
                             else newValue = PA1;
   } else

   if (oldValue instanceof com.sun.star.style.LineSpacing) {
         com.sun.star.style.LineSpacing LS = new com.sun.star.style.LineSpacing();
         com.sun.star.style.LineSpacing LSold = (com.sun.star.style.LineSpacing) oldValue;
         LS.Height = (short) ((LSold.Height)+1);
         LS.Mode = (short) ((LSold.Mode)+1);
         newValue = LS;
   } else

   if (oldValue instanceof com.sun.star.drawing.Direction3D) {
         com.sun.star.drawing.Direction3D D3D = new com.sun.star.drawing.Direction3D();
         com.sun.star.drawing.Direction3D D3Dold = (com.sun.star.drawing.Direction3D) oldValue;
         D3D.DirectionX = D3Dold.DirectionX + .5;
         D3D.DirectionY = D3Dold.DirectionY + .5;
         D3D.DirectionZ = D3Dold.DirectionZ + .5;
         newValue = D3D;
   } else

   if (oldValue instanceof com.sun.star.style.GraphicLocation) {
         Object GL1 = com.sun.star.style.GraphicLocation.AREA;
         Object GL2 = com.sun.star.style.GraphicLocation.LEFT_BOTTOM;

         if (oldValue.equals(GL1)) newValue = GL2;
                             else newValue = GL1;
   } else

   if (oldValue instanceof com.sun.star.style.TabStop) {
         com.sun.star.style.TabStop TS = new com.sun.star.style.TabStop();
         com.sun.star.style.TabStop TSold = (com.sun.star.style.TabStop) oldValue;
         com.sun.star.style.TabAlign TA1 = com.sun.star.style.TabAlign.CENTER;
         com.sun.star.style.TabAlign TA2 = com.sun.star.style.TabAlign.RIGHT;

         if ((TSold.Alignment).equals(TA1)) TS.Alignment = TA2;
                             else TS.Alignment = TA1;

         TS.Position = ((TSold.Position)+1);

         newValue = TS;
   } else

   if (oldValue instanceof com.sun.star.style.DropCapFormat) {
         com.sun.star.style.DropCapFormat DCF = new com.sun.star.style.DropCapFormat();
         com.sun.star.style.DropCapFormat DCFold = (com.sun.star.style.DropCapFormat) oldValue;
         DCF.Count = (byte) ((DCFold.Count)+1);
         DCF.Distance = (short) ((DCFold.Distance)+1);
         DCF.Lines = (byte) ((DCFold.Lines)+1);
         newValue = DCF;
   } else

   if (oldValue instanceof com.sun.star.text.TextContentAnchorType) {
         com.sun.star.text.TextContentAnchorType TCAT1 = com.sun.star.text.TextContentAnchorType.AS_CHARACTER;
         com.sun.star.text.TextContentAnchorType TCAT2 = com.sun.star.text.TextContentAnchorType.AT_CHARACTER;
         com.sun.star.text.TextContentAnchorType TCAT3 = com.sun.star.text.TextContentAnchorType.AT_FRAME;
         com.sun.star.text.TextContentAnchorType TCAT4 = com.sun.star.text.TextContentAnchorType.AT_PAGE;
         com.sun.star.text.TextContentAnchorType TCAT5 = com.sun.star.text.TextContentAnchorType.AT_PARAGRAPH;
         if (oldValue.equals(TCAT1)) newValue = TCAT2;
         if (oldValue.equals(TCAT2)) newValue = TCAT3;
         if (oldValue.equals(TCAT3)) newValue = TCAT4;
         if (oldValue.equals(TCAT4)) newValue = TCAT5;
         if (oldValue.equals(TCAT5)) newValue = TCAT1;
   } else

   if (oldValue instanceof com.sun.star.text.WrapTextMode) {
         com.sun.star.text.WrapTextMode WTM1 = com.sun.star.text.WrapTextMode.DYNAMIC;
         com.sun.star.text.WrapTextMode WTM2 = com.sun.star.text.WrapTextMode.LEFT;
         com.sun.star.text.WrapTextMode WTM3 = com.sun.star.text.WrapTextMode.NONE;
         com.sun.star.text.WrapTextMode WTM4 = com.sun.star.text.WrapTextMode.PARALLEL;
         com.sun.star.text.WrapTextMode WTM5 = com.sun.star.text.WrapTextMode.RIGHT;
         com.sun.star.text.WrapTextMode WTM6 = com.sun.star.text.WrapTextMode.THROUGHT;
         if (oldValue.equals(WTM1)) newValue = WTM2;
         if (oldValue.equals(WTM2)) newValue = WTM3;
         if (oldValue.equals(WTM3)) newValue = WTM4;
         if (oldValue.equals(WTM4)) newValue = WTM5;
         if (oldValue.equals(WTM5)) newValue = WTM6;
         if (oldValue.equals(WTM6)) newValue = WTM1;
   } else

   if (oldValue instanceof com.sun.star.awt.Size) {
         com.sun.star.awt.Size oldSize = (com.sun.star.awt.Size) oldValue;
         com.sun.star.awt.Size newSize = new com.sun.star.awt.Size();
         newSize.Height = oldSize.Height +1;
         newSize.Width = oldSize.Width +1;
         newValue = newSize;
   } else

   if (oldValue instanceof com.sun.star.awt.Rectangle) {
         com.sun.star.awt.Rectangle oldRectangle = (com.sun.star.awt.Rectangle) oldValue;
         com.sun.star.awt.Rectangle newRectangle = new com.sun.star.awt.Rectangle();
         newRectangle.Height =oldRectangle.Height +1;
         newRectangle.Width = oldRectangle.Width +1;
         newRectangle.X =oldRectangle.Y +1;
         newRectangle.Y = oldRectangle.X +1;
         newValue = newRectangle;
   } else

   if (oldValue instanceof com.sun.star.awt.Point) {
         com.sun.star.awt.Point oldPoint = (com.sun.star.awt.Point) oldValue;
         com.sun.star.awt.Point newPoint = new com.sun.star.awt.Point();
         newPoint.X = oldPoint.X +1;
         newPoint.Y = oldPoint.Y +1;
         newValue = newPoint;
   } else

   if (oldValue instanceof com.sun.star.table.ShadowFormat) {
         com.sun.star.table.ShadowFormat SF = new com.sun.star.table.ShadowFormat();
         com.sun.star.table.ShadowFormat SFold = (com.sun.star.table.ShadowFormat) oldValue;
         SF.IsTransparent = (! SFold.IsTransparent);
         SF.ShadowWidth = (short) ((SFold.ShadowWidth)+1);
         newValue = SF;
   } else

   if (oldValue instanceof com.sun.star.awt.FontSlant) {
         com.sun.star.awt.FontSlant FS1 = com.sun.star.awt.FontSlant.DONTKNOW;
         com.sun.star.awt.FontSlant FS2 = com.sun.star.awt.FontSlant.ITALIC;
         com.sun.star.awt.FontSlant FS3 = com.sun.star.awt.FontSlant.NONE;
         com.sun.star.awt.FontSlant FS4 = com.sun.star.awt.FontSlant.OBLIQUE;
         com.sun.star.awt.FontSlant FS5 = com.sun.star.awt.FontSlant.REVERSE_ITALIC;
         com.sun.star.awt.FontSlant FS6 = com.sun.star.awt.FontSlant.REVERSE_OBLIQUE;
         if (oldValue.equals(FS1)) newValue = FS2;
         if (oldValue.equals(FS2)) newValue = FS3;
         if (oldValue.equals(FS3)) newValue = FS4;
         if (oldValue.equals(FS4)) newValue = FS5;
         if (oldValue.equals(FS5)) newValue = FS6;
         if (oldValue.equals(FS6)) newValue = FS1;
   } else

   if (oldValue instanceof com.sun.star.table.CellHoriJustify) {
         com.sun.star.table.CellHoriJustify CHJ1 = com.sun.star.table.CellHoriJustify.BLOCK;
         com.sun.star.table.CellHoriJustify CHJ2 = com.sun.star.table.CellHoriJustify.CENTER;
         com.sun.star.table.CellHoriJustify CHJ3 = com.sun.star.table.CellHoriJustify.LEFT;
         com.sun.star.table.CellHoriJustify CHJ4 = com.sun.star.table.CellHoriJustify.REPEAT;
         com.sun.star.table.CellHoriJustify CHJ5 = com.sun.star.table.CellHoriJustify.RIGHT;
         com.sun.star.table.CellHoriJustify CHJ6 = com.sun.star.table.CellHoriJustify.STANDARD;
         if (oldValue.equals(CHJ1)) newValue = CHJ2;
         if (oldValue.equals(CHJ2)) newValue = CHJ3;
         if (oldValue.equals(CHJ3)) newValue = CHJ4;
         if (oldValue.equals(CHJ4)) newValue = CHJ5;
         if (oldValue.equals(CHJ5)) newValue = CHJ6;
         if (oldValue.equals(CHJ6)) newValue = CHJ1;
   } else

   if (oldValue instanceof com.sun.star.table.CellVertJustify) {
         com.sun.star.table.CellVertJustify CVJ1 = com.sun.star.table.CellVertJustify.BOTTOM;
         com.sun.star.table.CellVertJustify CVJ2 = com.sun.star.table.CellVertJustify.CENTER;
         com.sun.star.table.CellVertJustify CVJ3 = com.sun.star.table.CellVertJustify.STANDARD;
         com.sun.star.table.CellVertJustify CVJ4 = com.sun.star.table.CellVertJustify.TOP;
         if (oldValue.equals(CVJ1)) newValue = CVJ2;
         if (oldValue.equals(CVJ2)) newValue = CVJ3;
         if (oldValue.equals(CVJ3)) newValue = CVJ4;
         if (oldValue.equals(CVJ4)) newValue = CVJ1;
   } else

   if (oldValue instanceof com.sun.star.table.CellOrientation) {
         com.sun.star.table.CellOrientation CO1 = com.sun.star.table.CellOrientation.BOTTOMTOP;
         com.sun.star.table.CellOrientation CO2 = com.sun.star.table.CellOrientation.STACKED;
         com.sun.star.table.CellOrientation CO3 = com.sun.star.table.CellOrientation.STANDARD;
         com.sun.star.table.CellOrientation CO4 = com.sun.star.table.CellOrientation.TOPBOTTOM;
         if (oldValue.equals(CO1)) newValue = CO2;
         if (oldValue.equals(CO2)) newValue = CO3;
         if (oldValue.equals(CO3)) newValue = CO4;
         if (oldValue.equals(CO4)) newValue = CO1;
   } else

   if (oldValue instanceof com.sun.star.util.CellProtection) {
         com.sun.star.util.CellProtection CP = new com.sun.star.util.CellProtection();
         com.sun.star.util.CellProtection CPold = (com.sun.star.util.CellProtection) oldValue;
         CP.IsFormulaHidden = (! CPold.IsFormulaHidden);
         CP.IsHidden = (! CPold.IsHidden);
         CP.IsLocked = (! CPold.IsLocked);
         CP.IsPrintHidden = (! CPold.IsPrintHidden);
         newValue = CP;
   } else

   if (oldValue instanceof com.sun.star.table.TableBorder) {
         com.sun.star.table.TableBorder TBold = (com.sun.star.table.TableBorder) oldValue;
         com.sun.star.table.TableBorder TB = new com.sun.star.table.TableBorder();
         TB.IsBottomLineValid = (! TBold.IsBottomLineValid);
         TB.IsDistanceValid = (! TBold.IsDistanceValid);
         TB.IsRightLineValid = (! TBold.IsRightLineValid);
         TB.IsTopLineValid = (! TBold.IsTopLineValid);
         newValue = TB;
   } else
/*
   if (oldValue instanceof com.sun.star.awt.XBitmap) {
        newValue = new BitmapLoader();
   }
*/
   if (oldValue instanceof com.sun.star.drawing.FillStyle) {
         com.sun.star.drawing.FillStyle FS1 = com.sun.star.drawing.FillStyle.NONE;
         com.sun.star.drawing.FillStyle FS2 = com.sun.star.drawing.FillStyle.SOLID;
         com.sun.star.drawing.FillStyle FS3 = com.sun.star.drawing.FillStyle.GRADIENT;
         com.sun.star.drawing.FillStyle FS4 = com.sun.star.drawing.FillStyle.HATCH;
         com.sun.star.drawing.FillStyle FS5 = com.sun.star.drawing.FillStyle.BITMAP;
         if (oldValue.equals(FS1)) newValue = FS2;
         if (oldValue.equals(FS2)) newValue = FS3;
         if (oldValue.equals(FS3)) newValue = FS4;
         if (oldValue.equals(FS4)) newValue = FS5;
         if (oldValue.equals(FS5)) newValue = FS1;
   } else

   if (oldValue instanceof com.sun.star.awt.Gradient){
        com.sun.star.awt.Gradient _newValue = (com.sun.star.awt.Gradient)oldValue;
        _newValue.Angle += 10;
        _newValue.Border += 1;
        _newValue.EndColor += 1000;
        _newValue.EndIntensity -= 10;
        _newValue.StartColor += 500;
        _newValue.StartIntensity += 10;
        _newValue.StepCount += 50;
        _newValue.Style = com.sun.star.awt.GradientStyle.RADIAL;
        _newValue.XOffset += 10;
        _newValue.YOffset += 10;
        newValue = _newValue;
   } else

   if (oldValue instanceof com.sun.star.text.GraphicCrop){
        com.sun.star.text.GraphicCrop _newValue = (com.sun.star.text.GraphicCrop)oldValue;
        _newValue.Bottom += 10;
        _newValue.Left += 10;
        _newValue.Right += 10;
        _newValue.Top += 10;
        newValue = _newValue;
   } else

   if (oldValue instanceof com.sun.star.drawing.BitmapMode){
        com.sun.star.drawing.BitmapMode bm1 = com.sun.star.drawing.BitmapMode.NO_REPEAT;
        com.sun.star.drawing.BitmapMode bm2 = com.sun.star.drawing.BitmapMode.REPEAT;
        com.sun.star.drawing.BitmapMode bm3 = com.sun.star.drawing.BitmapMode.STRETCH;
         if (oldValue.equals(bm1)) newValue = bm2;
         if (oldValue.equals(bm2)) newValue = bm3;
         if (oldValue.equals(bm3)) newValue = bm3;
   } else

   if (oldValue instanceof com.sun.star.drawing.TextAdjust){
        com.sun.star.drawing.TextAdjust TA1 = com.sun.star.drawing.TextAdjust.BLOCK;
        com.sun.star.drawing.TextAdjust TA2 = com.sun.star.drawing.TextAdjust.CENTER;
        com.sun.star.drawing.TextAdjust TA3 = com.sun.star.drawing.TextAdjust.LEFT;
        com.sun.star.drawing.TextAdjust TA4 = com.sun.star.drawing.TextAdjust.RIGHT;
        com.sun.star.drawing.TextAdjust TA5 = com.sun.star.drawing.TextAdjust.STRETCH;
        if (oldValue.equals(TA1)) newValue = TA2;
        if (oldValue.equals(TA2)) newValue = TA3;
        if (oldValue.equals(TA3)) newValue = TA4;
        if (oldValue.equals(TA4)) newValue = TA5;
        if (oldValue.equals(TA5)) newValue = TA1;
   } else
    if (oldValue instanceof com.sun.star.drawing.TextFitToSizeType){
        com.sun.star.drawing.TextFitToSizeType TF1 = com.sun.star.drawing.TextFitToSizeType.ALLLINES;
        com.sun.star.drawing.TextFitToSizeType TF2 = com.sun.star.drawing.TextFitToSizeType.NONE;
        com.sun.star.drawing.TextFitToSizeType TF3 = com.sun.star.drawing.TextFitToSizeType.PROPORTIONAL;
        com.sun.star.drawing.TextFitToSizeType TF4 = com.sun.star.drawing.TextFitToSizeType.AUTOFIT;
        if (oldValue.equals(TF1)) newValue = TF2;
        if (oldValue.equals(TF2)) newValue = TF3;
        if (oldValue.equals(TF3)) newValue = TF4;
        if (oldValue.equals(TF4)) newValue = TF1;
    } else
    if (oldValue instanceof com.sun.star.drawing.TextAnimationKind){
        com.sun.star.drawing.TextAnimationKind AK1 = com.sun.star.drawing.TextAnimationKind.NONE;
        com.sun.star.drawing.TextAnimationKind AK2 = com.sun.star.drawing.TextAnimationKind.SLIDE;
        com.sun.star.drawing.TextAnimationKind AK3 = com.sun.star.drawing.TextAnimationKind.SCROLL;
        com.sun.star.drawing.TextAnimationKind AK4 = com.sun.star.drawing.TextAnimationKind.BLINK;
        com.sun.star.drawing.TextAnimationKind AK5 = com.sun.star.drawing.TextAnimationKind.ALTERNATE;

        if (oldValue.equals(AK1)) newValue = AK2;
        if (oldValue.equals(AK2)) newValue = AK3;
        if (oldValue.equals(AK3)) newValue = AK4;
        if (oldValue.equals(AK4)) newValue = AK5;
        if (oldValue.equals(AK5)) newValue = AK1;
    } else
    if (oldValue instanceof com.sun.star.drawing.TextAnimationDirection){
        com.sun.star.drawing.TextAnimationDirection AD1 = com.sun.star.drawing.TextAnimationDirection.LEFT;
        com.sun.star.drawing.TextAnimationDirection AD2 = com.sun.star.drawing.TextAnimationDirection.RIGHT;
        com.sun.star.drawing.TextAnimationDirection AD3 = com.sun.star.drawing.TextAnimationDirection.DOWN;
        com.sun.star.drawing.TextAnimationDirection AD4 = com.sun.star.drawing.TextAnimationDirection.UP;
        if (oldValue.equals(AD1)) newValue = AD2;
        if (oldValue.equals(AD2)) newValue = AD3;
        if (oldValue.equals(AD3)) newValue = AD4;
        if (oldValue.equals(AD4)) newValue = AD1;
    } else
    if (oldValue instanceof com.sun.star.drawing.LineStyle){
        com.sun.star.drawing.LineStyle LS1 = com.sun.star.drawing.LineStyle.NONE;
        com.sun.star.drawing.LineStyle LS2 = com.sun.star.drawing.LineStyle.DASH;
        com.sun.star.drawing.LineStyle LS3 = com.sun.star.drawing.LineStyle.SOLID;
        if (oldValue.equals(LS1)) newValue = LS2;
        if (oldValue.equals(LS2)) newValue = LS3;
        if (oldValue.equals(LS3)) newValue = LS1;
    } else
    if (oldValue instanceof com.sun.star.drawing.LineJoint){
        com.sun.star.drawing.LineJoint LJ1 = com.sun.star.drawing.LineJoint.BEVEL;
        com.sun.star.drawing.LineJoint LJ2 = com.sun.star.drawing.LineJoint.MIDDLE;
        com.sun.star.drawing.LineJoint LJ3 = com.sun.star.drawing.LineJoint.MITER;
        com.sun.star.drawing.LineJoint LJ4 = com.sun.star.drawing.LineJoint.NONE;
        com.sun.star.drawing.LineJoint LJ5 = com.sun.star.drawing.LineJoint.ROUND;
        if (oldValue.equals(LJ1)) newValue = LJ2;
        if (oldValue.equals(LJ2)) newValue = LJ3;
        if (oldValue.equals(LJ3)) newValue = LJ4;
        if (oldValue.equals(LJ4)) newValue = LJ5;
        if (oldValue.equals(LJ5)) newValue = LJ1;
    } else
    if (oldValue instanceof com.sun.star.drawing.LineDash){
        com.sun.star.drawing.LineDash _newValue = (com.sun.star.drawing.LineDash)oldValue;
        _newValue.Dashes += 1;
        _newValue.DashLen += 10;
        _newValue.Distance += 20;
        _newValue.DotLen += 10;
        _newValue.Dots += 10;
        _newValue.Style = com.sun.star.drawing.DashStyle.RECT;
        newValue = _newValue;
    } else
    if (oldValue instanceof com.sun.star.drawing.Hatch){
        com.sun.star.drawing.Hatch _newValue = (com.sun.star.drawing.Hatch) oldValue;
        _newValue.Angle += 10;
        _newValue.Color += 1000;
        _newValue.Distance += 10;
        _newValue.Style = com.sun.star.drawing.HatchStyle.DOUBLE;
    } else
    if (oldValue instanceof com.sun.star.drawing.RectanglePoint){
        com.sun.star.drawing.RectanglePoint RP1 = com.sun.star.drawing.RectanglePoint.LEFT_BOTTOM;
        com.sun.star.drawing.RectanglePoint RP2 = com.sun.star.drawing.RectanglePoint.LEFT_MIDDLE;
        com.sun.star.drawing.RectanglePoint RP3 = com.sun.star.drawing.RectanglePoint.LEFT_TOP;
        com.sun.star.drawing.RectanglePoint RP4 = com.sun.star.drawing.RectanglePoint.MIDDLE_BOTTOM;
        com.sun.star.drawing.RectanglePoint RP5 = com.sun.star.drawing.RectanglePoint.MIDDLE_MIDDLE;
        com.sun.star.drawing.RectanglePoint RP6 = com.sun.star.drawing.RectanglePoint.MIDDLE_TOP;
        com.sun.star.drawing.RectanglePoint RP7 = com.sun.star.drawing.RectanglePoint.RIGHT_BOTTOM;
        com.sun.star.drawing.RectanglePoint RP8 = com.sun.star.drawing.RectanglePoint.RIGHT_MIDDLE;
        com.sun.star.drawing.RectanglePoint RP9 = com.sun.star.drawing.RectanglePoint.RIGHT_TOP;

        if (oldValue.equals(RP1)) newValue = RP2;
        if (oldValue.equals(RP2)) newValue = RP3;
        if (oldValue.equals(RP3)) newValue = RP4;
        if (oldValue.equals(RP4)) newValue = RP5;
        if (oldValue.equals(RP5)) newValue = RP6;
        if (oldValue.equals(RP6)) newValue = RP7;
        if (oldValue.equals(RP7)) newValue = RP8;
        if (oldValue.equals(RP8)) newValue = RP9;
        if (oldValue.equals(RP9)) newValue = RP1;

    } else
    if (oldValue instanceof com.sun.star.chart.ChartErrorCategory){
        com.sun.star.chart.ChartErrorCategory CC1 = com.sun.star.chart.ChartErrorCategory.CONSTANT_VALUE;
        com.sun.star.chart.ChartErrorCategory CC2 = com.sun.star.chart.ChartErrorCategory.ERROR_MARGIN;
        com.sun.star.chart.ChartErrorCategory CC3 = com.sun.star.chart.ChartErrorCategory.NONE;
        com.sun.star.chart.ChartErrorCategory CC4 = com.sun.star.chart.ChartErrorCategory.PERCENT;
        com.sun.star.chart.ChartErrorCategory CC5 = com.sun.star.chart.ChartErrorCategory.STANDARD_DEVIATION;
        com.sun.star.chart.ChartErrorCategory CC6 = com.sun.star.chart.ChartErrorCategory.VARIANCE;
        if (oldValue.equals(CC1)) newValue = CC2;
        if (oldValue.equals(CC2)) newValue = CC3;
        if (oldValue.equals(CC3)) newValue = CC4;
        if (oldValue.equals(CC4)) newValue = CC5;
        if (oldValue.equals(CC5)) newValue = CC6;
        if (oldValue.equals(CC6)) newValue = CC1;
    } else
    if (oldValue instanceof com.sun.star.chart.ChartErrorIndicatorType){
        com.sun.star.chart.ChartErrorIndicatorType IT1 = com.sun.star.chart.ChartErrorIndicatorType.LOWER;
        com.sun.star.chart.ChartErrorIndicatorType IT2 = com.sun.star.chart.ChartErrorIndicatorType.NONE;
        com.sun.star.chart.ChartErrorIndicatorType IT3 = com.sun.star.chart.ChartErrorIndicatorType.TOP_AND_BOTTOM;
        com.sun.star.chart.ChartErrorIndicatorType IT4 = com.sun.star.chart.ChartErrorIndicatorType.UPPER;
        if (oldValue.equals(IT1)) newValue = IT2;
        if (oldValue.equals(IT2)) newValue = IT3;
        if (oldValue.equals(IT3)) newValue = IT4;
        if (oldValue.equals(IT4)) newValue = IT1;
    } else
    if (oldValue instanceof com.sun.star.chart.ChartRegressionCurveType){
        com.sun.star.chart.ChartRegressionCurveType CT1 = com.sun.star.chart.ChartRegressionCurveType.EXPONENTIAL;
        com.sun.star.chart.ChartRegressionCurveType CT2 = com.sun.star.chart.ChartRegressionCurveType.LINEAR;
        com.sun.star.chart.ChartRegressionCurveType CT3 = com.sun.star.chart.ChartRegressionCurveType.LOGARITHM;
        com.sun.star.chart.ChartRegressionCurveType CT4 = com.sun.star.chart.ChartRegressionCurveType.NONE;
        com.sun.star.chart.ChartRegressionCurveType CT5 = com.sun.star.chart.ChartRegressionCurveType.POLYNOMIAL;
        com.sun.star.chart.ChartRegressionCurveType CT6 = com.sun.star.chart.ChartRegressionCurveType.POWER;
        if (oldValue.equals(CT1)) newValue = CT2;
        if (oldValue.equals(CT2)) newValue = CT3;
        if (oldValue.equals(CT3)) newValue = CT4;
        if (oldValue.equals(CT4)) newValue = CT5;
        if (oldValue.equals(CT5)) newValue = CT6;
        if (oldValue.equals(CT6)) newValue = CT1;

    } else
    if (oldValue instanceof com.sun.star.chart.ChartDataRowSource){
        com.sun.star.chart.ChartDataRowSource RS1 = com.sun.star.chart.ChartDataRowSource.COLUMNS;
        com.sun.star.chart.ChartDataRowSource RS2 = com.sun.star.chart.ChartDataRowSource.ROWS;
        if (oldValue.equals(RS1)) newValue = RS2;
        if (oldValue.equals(RS2)) newValue = RS1;
    } else
    if (oldValue instanceof com.sun.star.awt.FontDescriptor){
        com.sun.star.awt.FontDescriptor _newValue = (com.sun.star.awt.FontDescriptor)oldValue;
        _newValue.CharacterWidth += 5;
        _newValue.CharSet = com.sun.star.awt.CharSet.ANSI;
        _newValue.Family = com.sun.star.awt.FontFamily.DECORATIVE;
        _newValue.Height += 2;
        _newValue.Kerning = !_newValue.Kerning;
        _newValue.Name = "Courier";
        _newValue.Orientation += 45;
        _newValue.Pitch = com.sun.star.awt.FontPitch.VARIABLE;
        _newValue.Slant = com.sun.star.awt.FontSlant.REVERSE_ITALIC;
        _newValue.Strikeout = com.sun.star.awt.FontStrikeout.X;
        _newValue.StyleName = "Bold";
        _newValue.Type = com.sun.star.awt.FontType.SCALABLE;
        _newValue.Underline = com.sun.star.awt.FontUnderline.BOLDDASHDOTDOT;
        _newValue.Weight += 5;
        _newValue.Width += 6;
        _newValue.WordLineMode = !_newValue.WordLineMode;

        newValue = _newValue;
    } else
    if (oldValue instanceof com.sun.star.form.NavigationBarMode){
        com.sun.star.form.NavigationBarMode BM1 = com.sun.star.form.NavigationBarMode.CURRENT;
        com.sun.star.form.NavigationBarMode BM2 = com.sun.star.form.NavigationBarMode.NONE;
        com.sun.star.form.NavigationBarMode BM3 = com.sun.star.form.NavigationBarMode.PARENT;
        if (oldValue.equals(BM1)) newValue = BM2;
        if (oldValue.equals(BM2)) newValue = BM3;
        if (oldValue.equals(BM3)) newValue = BM1;
    } else
    if (oldValue instanceof com.sun.star.form.FormSubmitEncoding){
        com.sun.star.form.FormSubmitEncoding SE1 = com.sun.star.form.FormSubmitEncoding.MULTIPART;
        com.sun.star.form.FormSubmitEncoding SE2 = com.sun.star.form.FormSubmitEncoding.TEXT;
        com.sun.star.form.FormSubmitEncoding SE3 = com.sun.star.form.FormSubmitEncoding.URL;
        if (oldValue.equals(SE1)) newValue = SE2;
        if (oldValue.equals(SE2)) newValue = SE3;
        if (oldValue.equals(SE3)) newValue = SE1;
    } else
    if (oldValue instanceof com.sun.star.form.FormSubmitMethod){
        com.sun.star.form.FormSubmitMethod FM1 = com.sun.star.form.FormSubmitMethod.GET;
        com.sun.star.form.FormSubmitMethod FM2 = com.sun.star.form.FormSubmitMethod.POST;
        if (oldValue.equals(FM1)) newValue = FM2;
        if (oldValue.equals(FM2)) newValue = FM1;
    } else

   if (oldValue instanceof com.sun.star.text.SectionFileLink){
        com.sun.star.text.SectionFileLink _newValue =
                                    new com.sun.star.text.SectionFileLink();

        _newValue.FileURL = util.utils.getFullTestURL("SwXTextSection.sdw");
        newValue=_newValue;
    } else

   if (oldValue instanceof com.sun.star.table.BorderLine){
        com.sun.star.table.BorderLine _newValue = (com.sun.star.table.BorderLine)oldValue;
    _newValue.Color += 2;
        _newValue.InnerLineWidth += 2;
        _newValue.LineDistance += 2;
        _newValue.OuterLineWidth += 3;
        newValue=_newValue;
    } else

   if (oldValue instanceof com.sun.star.text.XTextColumns){
        com.sun.star.text.XTextColumns _newValue = (com.sun.star.text.XTextColumns)oldValue;
    _newValue.setColumnCount((short)1);
        newValue=_newValue;
    } else

   if (oldValue instanceof com.sun.star.form.FormButtonType){
        com.sun.star.form.FormButtonType BT1 = com.sun.star.form.FormButtonType.PUSH;
        com.sun.star.form.FormButtonType BT2 = com.sun.star.form.FormButtonType.RESET;
        com.sun.star.form.FormButtonType BT3 = com.sun.star.form.FormButtonType.SUBMIT;
        com.sun.star.form.FormButtonType BT4 = com.sun.star.form.FormButtonType.URL;

        if (oldValue.equals(BT1)) newValue = BT2;
        if (oldValue.equals(BT2)) newValue = BT3;
        if (oldValue.equals(BT3)) newValue = BT4;
        if (oldValue.equals(BT4)) newValue = BT1;

   } else
   if (oldValue instanceof com.sun.star.form.ListSourceType){
        com.sun.star.form.ListSourceType ST1 = com.sun.star.form.ListSourceType.QUERY;
        com.sun.star.form.ListSourceType ST2 = com.sun.star.form.ListSourceType.SQL;
        com.sun.star.form.ListSourceType ST3 = com.sun.star.form.ListSourceType.SQLPASSTHROUGH;
        com.sun.star.form.ListSourceType ST4 = com.sun.star.form.ListSourceType.TABLE;
        com.sun.star.form.ListSourceType ST5 = com.sun.star.form.ListSourceType.TABLEFIELDS;
        com.sun.star.form.ListSourceType ST6 = com.sun.star.form.ListSourceType.VALUELIST;
        if (oldValue.equals(ST1)) newValue = ST2;
        if (oldValue.equals(ST2)) newValue = ST3;
        if (oldValue.equals(ST3)) newValue = ST4;
        if (oldValue.equals(ST4)) newValue = ST5;
        if (oldValue.equals(ST5)) newValue = ST6;
        if (oldValue.equals(ST6)) newValue = ST1;
   } else
   if (oldValue instanceof com.sun.star.table.BorderLine){
        com.sun.star.table.BorderLine _newValue = (com.sun.star.table.BorderLine)oldValue;
        _newValue.Color += 1000;
        _newValue.InnerLineWidth += 2;
        _newValue.LineDistance +=3;
        _newValue.OuterLineWidth += 3;
        newValue = _newValue;
   } else
   if (oldValue instanceof com.sun.star.sheet.DataPilotFieldOrientation) {
        com.sun.star.sheet.DataPilotFieldOrientation FO1 = com.sun.star.sheet.DataPilotFieldOrientation.PAGE;
        com.sun.star.sheet.DataPilotFieldOrientation FO2 = com.sun.star.sheet.DataPilotFieldOrientation.COLUMN;
        com.sun.star.sheet.DataPilotFieldOrientation FO3 = com.sun.star.sheet.DataPilotFieldOrientation.DATA;
        com.sun.star.sheet.DataPilotFieldOrientation FO4 = com.sun.star.sheet.DataPilotFieldOrientation.HIDDEN;
        com.sun.star.sheet.DataPilotFieldOrientation FO5 = com.sun.star.sheet.DataPilotFieldOrientation.ROW;
        if (oldValue.equals(FO1)) newValue = FO2;
        if (oldValue.equals(FO2)) newValue = FO3;
        if (oldValue.equals(FO3)) newValue = FO4;
        if (oldValue.equals(FO4)) newValue = FO5;
        if (oldValue.equals(FO5)) newValue = FO1;
   } else
   if (oldValue instanceof com.sun.star.sheet.GeneralFunction) {
        com.sun.star.sheet.GeneralFunction GF1 = com.sun.star.sheet.GeneralFunction.NONE;
        com.sun.star.sheet.GeneralFunction GF2 = com.sun.star.sheet.GeneralFunction.AVERAGE;
        com.sun.star.sheet.GeneralFunction GF3 = com.sun.star.sheet.GeneralFunction.COUNT;
        com.sun.star.sheet.GeneralFunction GF4 = com.sun.star.sheet.GeneralFunction.COUNTNUMS;
        com.sun.star.sheet.GeneralFunction GF5 = com.sun.star.sheet.GeneralFunction.MAX;
        com.sun.star.sheet.GeneralFunction GF6 = com.sun.star.sheet.GeneralFunction.MIN;
        com.sun.star.sheet.GeneralFunction GF7 = com.sun.star.sheet.GeneralFunction.AUTO;
        com.sun.star.sheet.GeneralFunction GF8 = com.sun.star.sheet.GeneralFunction.PRODUCT;
        com.sun.star.sheet.GeneralFunction GF9 = com.sun.star.sheet.GeneralFunction.STDEV;
        com.sun.star.sheet.GeneralFunction GF10 = com.sun.star.sheet.GeneralFunction.STDEVP;
        com.sun.star.sheet.GeneralFunction GF11 = com.sun.star.sheet.GeneralFunction.SUM;
        com.sun.star.sheet.GeneralFunction GF12 = com.sun.star.sheet.GeneralFunction.VAR;
        com.sun.star.sheet.GeneralFunction GF13 = com.sun.star.sheet.GeneralFunction.VARP;

        if (oldValue.equals(GF1)) newValue = GF2;
        if (oldValue.equals(GF2)) newValue = GF3;
        if (oldValue.equals(GF3)) newValue = GF4;
        if (oldValue.equals(GF4)) newValue = GF5;
        if (oldValue.equals(GF5)) newValue = GF6;
        if (oldValue.equals(GF6)) newValue = GF7;
        if (oldValue.equals(GF7)) newValue = GF8;
        if (oldValue.equals(GF8)) newValue = GF9;
        if (oldValue.equals(GF9)) newValue = GF10;
        if (oldValue.equals(GF10)) newValue = GF11;
        if (oldValue.equals(GF11)) newValue = GF12;
        if (oldValue.equals(GF12)) newValue = GF13;
        if (oldValue.equals(GF13)) newValue = GF1;
   } else

   if (oldValue instanceof com.sun.star.table.CellAddress){
        com.sun.star.table.CellAddress _newValue = (com.sun.star.table.CellAddress)oldValue;
        _newValue.Column += 1;
        _newValue.Row += 1;
        newValue = _newValue;
   } else

   if (oldValue instanceof com.sun.star.table.TableOrientation){
        com.sun.star.table.TableOrientation TO1 = com.sun.star.table.TableOrientation.COLUMNS;
        com.sun.star.table.TableOrientation TO2 = com.sun.star.table.TableOrientation.ROWS;
        if (oldValue.equals(TO1)) newValue = TO2; else newValue = TO1;
   } else

   if (oldValue instanceof com.sun.star.sheet.DataImportMode){
        com.sun.star.sheet.DataImportMode DIM1 = com.sun.star.sheet.DataImportMode.NONE;
        com.sun.star.sheet.DataImportMode DIM2 = com.sun.star.sheet.DataImportMode.QUERY;
        com.sun.star.sheet.DataImportMode DIM3 = com.sun.star.sheet.DataImportMode.SQL;
        com.sun.star.sheet.DataImportMode DIM4 = com.sun.star.sheet.DataImportMode.TABLE;

        if (oldValue.equals(DIM1)) newValue = DIM2;
        if (oldValue.equals(DIM2)) newValue = DIM3;
        if (oldValue.equals(DIM3)) newValue = DIM4;
        if (oldValue.equals(DIM4)) newValue = DIM1;

   } else

//   if (oldValue instanceof com.sun.star.text.TableColumnSeparator[]){
//        com.sun.star.text.TableColumnSeparator[] _newValue = (com.sun.star.text.TableColumnSeparator[]) oldValue;
//        com.sun.star.text.TableColumnSeparator sep = new com.sun.star.text.TableColumnSeparator();
//        sep.IsVisible = ! _newValue[0].IsVisible;
//        _newValue[0] = sep;
//        newValue = _newValue;
//   } else

   if (oldValue instanceof com.sun.star.style.BreakType){
        com.sun.star.style.BreakType BT1 = com.sun.star.style.BreakType.COLUMN_AFTER;
        com.sun.star.style.BreakType BT2 = com.sun.star.style.BreakType.COLUMN_BEFORE;
        com.sun.star.style.BreakType BT3 = com.sun.star.style.BreakType.COLUMN_BOTH;
        com.sun.star.style.BreakType BT4 = com.sun.star.style.BreakType.PAGE_AFTER;
        com.sun.star.style.BreakType BT5 = com.sun.star.style.BreakType.PAGE_BEFORE;
        com.sun.star.style.BreakType BT6 = com.sun.star.style.BreakType.PAGE_BOTH;
        com.sun.star.style.BreakType BT7 = com.sun.star.style.BreakType.NONE;
        if (oldValue.equals(BT1)) newValue = BT2;
        if (oldValue.equals(BT2)) newValue = BT3;
        if (oldValue.equals(BT3)) newValue = BT4;
        if (oldValue.equals(BT4)) newValue = BT5;
        if (oldValue.equals(BT6)) newValue = BT6;
        if (oldValue.equals(BT6)) newValue = BT7;
        if (oldValue.equals(BT7)) newValue = BT1;
   } else
    if (oldValue instanceof PropertyValue){
        PropertyValue newVal = new PropertyValue();
        newVal.Name = ((PropertyValue)oldValue).Name;
        newVal.Value = changePValue(((PropertyValue)oldValue).Value);
        newValue = newVal;
    } else
   if (oldValue instanceof com.sun.star.sheet.ValidationAlertStyle){
        com.sun.star.sheet.ValidationAlertStyle VAS1 = com.sun.star.sheet.ValidationAlertStyle.INFO;
        com.sun.star.sheet.ValidationAlertStyle VAS2 = com.sun.star.sheet.ValidationAlertStyle.MACRO;
        com.sun.star.sheet.ValidationAlertStyle VAS3 = com.sun.star.sheet.ValidationAlertStyle.STOP;
        com.sun.star.sheet.ValidationAlertStyle VAS4 = com.sun.star.sheet.ValidationAlertStyle.WARNING;

        if (oldValue.equals(VAS1)) newValue = VAS2;
        if (oldValue.equals(VAS2)) newValue = VAS3;
        if (oldValue.equals(VAS3)) newValue = VAS4;
        if (oldValue.equals(VAS4)) newValue = VAS1;

   } else
    if (oldValue instanceof com.sun.star.sheet.ValidationType){
        com.sun.star.sheet.ValidationType VT1 = com.sun.star.sheet.ValidationType.ANY;
        com.sun.star.sheet.ValidationType VT2 = com.sun.star.sheet.ValidationType.CUSTOM;
        com.sun.star.sheet.ValidationType VT3 = com.sun.star.sheet.ValidationType.DATE;
        com.sun.star.sheet.ValidationType VT4 = com.sun.star.sheet.ValidationType.DECIMAL;
        com.sun.star.sheet.ValidationType VT5 = com.sun.star.sheet.ValidationType.LIST;
        com.sun.star.sheet.ValidationType VT6 = com.sun.star.sheet.ValidationType.TEXT_LEN;
        com.sun.star.sheet.ValidationType VT7 = com.sun.star.sheet.ValidationType.TIME;
        com.sun.star.sheet.ValidationType VT8 = com.sun.star.sheet.ValidationType.WHOLE;

        if (oldValue.equals(VT1)) newValue = VT2;
        if (oldValue.equals(VT2)) newValue = VT3;
        if (oldValue.equals(VT3)) newValue = VT4;
        if (oldValue.equals(VT4)) newValue = VT5;
        if (oldValue.equals(VT5)) newValue = VT6;
        if (oldValue.equals(VT6)) newValue = VT7;
        if (oldValue.equals(VT7)) newValue = VT8;
        if (oldValue.equals(VT8)) newValue = VT1;

    } else
    if (oldValue instanceof com.sun.star.text.WritingMode){
        if (oldValue.equals(com.sun.star.text.WritingMode.LR_TB)) {
            newValue = com.sun.star.text.WritingMode.TB_RL;
        } else {
            newValue = com.sun.star.text.WritingMode.LR_TB;
        }
    } else
    if (oldValue instanceof com.sun.star.uno.Enum) {
        // universal changer for Enumerations
        try {
            Class<?> enumClass = oldValue.getClass() ;
            Field[] flds = enumClass.getFields() ;

            newValue = null ;

            for (int i = 0; i < flds.length; i++) {
                if (Enum.class.equals(flds[i].getType().getSuperclass())) {

                    Enum value = (Enum) flds[i].get(null) ;
                    if (newValue == null && !value.equals(oldValue)) {
                        newValue = value ;
                        break ;
                    }
                }
            }
        } catch (Exception e) {
            System.err.println("Exception occurred while changing Enumeration value:") ;
            e.printStackTrace(System.err) ;
        }
        if (newValue == null) newValue = oldValue ;

    } else
    if (oldValue instanceof com.sun.star.style.TabStop[]){
        com.sun.star.style.TabStop[] _newValue = (com.sun.star.style.TabStop[]) oldValue;
        if (_newValue.length == 0) {
            _newValue = new com.sun.star.style.TabStop[1];
        }
        com.sun.star.style.TabStop sep = new com.sun.star.style.TabStop();
        sep.Position += 1;
        _newValue[0] = sep;
        newValue = _newValue;
    } else
    if (oldValue instanceof short[]){
        short[] oldArr = (short[])oldValue;
        int len = oldArr.length;
        short[] newArr = new short[len + 1];
        for (int i = 0; i < len; i++) {
            newArr[i] = (short)(oldArr[i] + 1);
        }
        newArr[len] = 5;
        newValue = newArr;
    } else
    if (oldValue instanceof String[]){
        String[] oldArr = (String[])oldValue;
        int len = oldArr.length;
        String[] newArr = new String[len + 1];
        for (int i = 0; i < len; i++) {
            newArr[i] = "_" + oldArr[i];
        }
        newArr[len] = "_dummy";
        newValue = newArr;
    } else
    if (oldValue instanceof PropertyValue){
        PropertyValue newVal = new PropertyValue();
        newVal.Name = ((PropertyValue)oldValue).Name;
        newVal.Value = changePValue(((PropertyValue)oldValue).Value);
        newValue = newVal;
    } else
    if (oldValue instanceof com.sun.star.util.Date) {
        com.sun.star.util.Date oldD = (com.sun.star.util.Date) oldValue;
        com.sun.star.util.Date newD = new com.sun.star.util.Date();
        newD.Day = (short) (oldD.Day+(short) 1);
        newValue = newD;
    } else
    if (oldValue instanceof com.sun.star.util.DateTime) {
        com.sun.star.util.DateTime oldDT = (com.sun.star.util.DateTime) oldValue;
        com.sun.star.util.DateTime newDT = new com.sun.star.util.DateTime();
        newDT.Day = (short) (oldDT.Day+(short) 1);
        newDT.Hours = (short) (oldDT.Hours+(short) 1);
        newValue = newDT;
    } else
    if (oldValue instanceof com.sun.star.util.Time) {
        com.sun.star.util.Time oldT = (com.sun.star.util.Time) oldValue;
        com.sun.star.util.Time newT = new com.sun.star.util.Time();
        newT.Hours = (short) (oldT.Hours+(short) 1);
        newValue = newT;
    } else
    if (oldValue instanceof com.sun.star.text.TableColumnSeparator) {
        com.sun.star.text.TableColumnSeparator oldTCS = (com.sun.star.text.TableColumnSeparator) oldValue;
        com.sun.star.text.TableColumnSeparator newTCS = new com.sun.star.text.TableColumnSeparator();
        newTCS.IsVisible = !(oldTCS.IsVisible);
        newTCS.Position = (short) (oldTCS.Position+(short) 1);
        newValue = newTCS;
    } else
    if (oldValue instanceof com.sun.star.drawing.HomogenMatrix3) {
        com.sun.star.drawing.HomogenMatrix3 oldHM = (com.sun.star.drawing.HomogenMatrix3) oldValue;
        com.sun.star.drawing.HomogenMatrix3 newHM = new com.sun.star.drawing.HomogenMatrix3();
        newHM.Line1.Column1 = oldHM.Line1.Column1+1;
        newHM.Line2.Column2 = oldHM.Line2.Column2+1;
        newHM.Line3.Column3 = oldHM.Line3.Column3+1;
        newValue = newHM;
    } else

    if (oldValue instanceof com.sun.star.drawing.PolyPolygonBezierCoords) {
        com.sun.star.drawing.PolyPolygonBezierCoords oldPPC = (com.sun.star.drawing.PolyPolygonBezierCoords) oldValue;
        com.sun.star.drawing.PolyPolygonBezierCoords newPPC = new com.sun.star.drawing.PolyPolygonBezierCoords();
        newPPC.Coordinates = oldPPC.Coordinates;
        newPPC.Flags = oldPPC.Flags;
        PolygonFlags[][] fArray = new PolygonFlags[1][1];
        PolygonFlags[] flags = new PolygonFlags[1];
        flags[0] = PolygonFlags.NORMAL;
        fArray[0] = flags;
        Point[][] pArray = new Point[1][1];
        Point[] points = new Point[1];
        Point aPoint = new Point();
        aPoint.X = 1;
        aPoint.Y = 2;
        points[0] = aPoint;
        pArray[0] = points;
        if ( oldPPC.Coordinates.length == 0 ) {
            newPPC.Coordinates = pArray;
            newPPC.Flags = fArray;
        } else {
            if ( oldPPC.Coordinates[0].length == 0 ) {
                newPPC.Coordinates = pArray;
                newPPC.Flags = fArray;
            } else {
                newPPC.Coordinates[0][0].X = oldPPC.Coordinates[0][0].X +1;
                newPPC.Coordinates[0][0].Y = oldPPC.Coordinates[0][0].Y +1;
            }
        }
        newValue = newPPC;
    } else
    if (oldValue.getClass().isArray()) {
        // changer for arrays : changes all elements
        Class<?> arrType = oldValue.getClass().getComponentType() ;
        newValue = Array.newInstance(arrType, Array.getLength(oldValue)) ;
        for (int i = 0; i < Array.getLength(newValue); i++) {
            if (!arrType.isPrimitive()) {
                Object elem = changePValue(Array.get(oldValue, i)) ;
                Array.set(newValue, i, elem);
            } else {
                if (Boolean.TYPE.equals(arrType)) {
                    Array.setBoolean(newValue, i, !Array.getBoolean(oldValue, i));
                } else
                if (Byte.TYPE.equals(arrType)) {
                    Array.setByte(newValue, i,
                        (byte) (Array.getByte(oldValue, i) + 1));
                } else
                if (Character.TYPE.equals(arrType)) {
                    Array.setChar(newValue, i,
                        (char) (Array.getChar(oldValue, i) + 1));
                } else
                if (Double.TYPE.equals(arrType)) {
                    Array.setDouble(newValue, i, Array.getDouble(oldValue, i) + 1);
                } else
                if (Float.TYPE.equals(arrType)) {
                    Array.setFloat(newValue, i, Array.getFloat(oldValue, i) + 1);
                } else
                if (Integer.TYPE.equals(arrType)) {
                    Array.setInt(newValue, i, Array.getInt(oldValue, i) + 1);
                } else
                if (Long.TYPE.equals(arrType)) {
                    Array.setLong(newValue, i, Array.getLong(oldValue, i) + 1);
                } else
                if (Short.TYPE.equals(arrType)) {
                    Array.setShort(newValue, i,
                        (short) (Array.getShort(oldValue, i) + 1));
                }
            }
        }
    } else
    if (isStructure(oldValue)) {
        // universal changer for structures
        Class<?> clazz = oldValue.getClass() ;
        try {
            newValue = clazz.newInstance() ;
            Field[] fields = clazz.getFields();
            for (int i = 0; i < fields.length; i++) {
                if ((fields[i].getModifiers() & Modifier.PUBLIC) != 0) {
                    Class<?> fType = fields[i].getType() ;
                    Field field = fields[i] ;
                    if (!fType.isPrimitive()) {
                        field.set(newValue, changePValue(field.get(oldValue)));
                    } else {
                        if (Boolean.TYPE.equals(fType)) {
                            field.setBoolean(newValue, !field.getBoolean(oldValue));
                        } else
                        if (Byte.TYPE.equals(fType)) {
                            field.setByte(newValue, (byte) (field.getByte(oldValue) + 1));
                        } else
                        if (Character.TYPE.equals(fType)) {
                            field.setChar(newValue, (char) (field.getChar(oldValue) + 1));
                        } else
                        if (Double.TYPE.equals(fType)) {
                            field.setDouble(newValue, field.getDouble(oldValue) + 1);
                        } else
                        if (Float.TYPE.equals(fType)) {
                            field.setFloat(newValue, field.getFloat(oldValue) + 1);
                        } else
                        if (Integer.TYPE.equals(fType)) {
                            field.setInt(newValue, field.getInt(oldValue) + 1);
                        } else
                        if (Long.TYPE.equals(fType)) {
                            field.setLong(newValue, field.getLong(oldValue) + 1);
                        } else
                        if (Short.TYPE.equals(fType)) {
                            field.setShort(newValue, (short) (field.getShort(oldValue) + 1));
                        }
                    }
                }
            }
        } catch (IllegalAccessException e) {
            e.printStackTrace() ;
        } catch (InstantiationException e) {
            e.printStackTrace() ;
        }

    } else
    {
        System.out.println("ValueChanger don't know type " + oldValue.getClass());
    }

   return newValue;

 } // end of Change PValue

 /**
  * Checks if the passed value is the API structure.
  * The value assumed to be a structure if there are no public
  * methods, and all public fields are not static or final.
  *
  * @param val the value to be checked.
  * @return <code>true</code> if the value is acssumed to be a
  * structure.
  */
 private static boolean isStructure(Object val) {
    boolean result = true ;

    Class<?> clazz = val.getClass() ;

    Method[] meth = clazz.getDeclaredMethods() ;
    for (int i = 0; i < meth.length; i++) {
        result &= (meth[i].getModifiers() & Modifier.PUBLIC) == 0 ;
    }

    Field[] fields = clazz.getDeclaredFields() ;
    for (int i = 0; i < fields.length; i++) {
        int mod = fields[i].getModifiers() ;
        // If the field is PUBLIC it must not be STATIC or FINAL
        result &= ((mod & Modifier.PUBLIC) == 0) ||
            (((mod & Modifier.STATIC) == 0) && ((mod & Modifier.FINAL) == 0)) ;
    }

    return result ;
 }
}
