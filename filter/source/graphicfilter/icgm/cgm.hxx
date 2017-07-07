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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_CGM_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_CGM_HXX

#include <com/sun/star/frame/XModel.hpp>

#include <rtl/ustring.hxx>
#include <vector>
#include <memory>
#include <vcl/vclptr.hxx>
#include "cgmtypes.hxx"

class   Graphic;
class   SvStream;
class   CGMChart;
class   CGMBitmap;
class   CGMImpressOutAct;
class   CGMElements;
class   GDIMetaFile;
class   VirtualDevice;

class CGM
{
        friend class CGMChart;
        friend class CGMBitmap;
        friend class CGMElements;
        friend class CGMImpressOutAct;

        double              mnOutdx;                // Output size in 1/100TH mm
        double              mnOutdy;                // on which is mapped
        double              mnVDCXadd;
        double              mnVDCYadd;
        double              mnVDCXmul;
        double              mnVDCYmul;
        double              mnVDCdx;
        double              mnVDCdy;
        double              mnXFraction;
        double              mnYFraction;
        bool                mbAngReverse;           // AngularDirection

        Graphic*            mpGraphic;

        bool                mbStatus;
        bool                mbMetaFile;
        bool                mbIsFinished;
        bool                mbPicture;
        bool                mbPictureBody;
        bool                mbFigure;
        bool                mbFirstOutPut;
        bool                mbInDefaultReplacement;
        sal_uInt32          mnAct4PostReset;
        CGMBitmap*          mpBitmapInUse;
        CGMChart*           mpChart;                // if sal_True->"SHWSLIDEREC"
                                                    //  otherwise "BEGINPIC" commands
                                                    // controls page inserting
        CGMElements*        pElement;
        CGMElements*        pCopyOfE;
        CGMImpressOutAct*   mpOutAct;
        ::std::vector< sal_uInt8 * > maDefRepList;
        ::std::vector< sal_uInt32  > maDefRepSizeList;

        sal_uInt8*              mpSource;         // start of source buffer that is not increased
                                                  // ( instead use mnParaCount to index )
        sal_uInt8*              mpEndValidSource; // end position in source buffer of last valid data
        sal_uInt32              mnParaSize;     // actual parameter size which has been done so far
        sal_uInt32              mnActCount;     // increased by each action
        std::unique_ptr<sal_uInt8[]>
                                mpBuf;          // source stream operation -> then this is allocated for
                                                //                            the temp input buffer

        sal_uInt32              mnEscape;
        sal_uInt32              mnElementClass;
        sal_uInt32              mnElementID;
        sal_uInt32              mnElementSize;  // full parameter size for the latest action

        sal_uInt32          ImplGetUI16();
        static sal_uInt8    ImplGetByte( sal_uInt32 nSource, sal_uInt32 nPrecision );
        sal_Int32           ImplGetI( sal_uInt32 nPrecision );
        sal_uInt32          ImplGetUI( sal_uInt32 nPrecision );
        static void         ImplGetSwitch4( const sal_uInt8* pSource, sal_uInt8* pDest );
        static void         ImplGetSwitch8( const sal_uInt8* pSource, sal_uInt8* pDest );
        double              ImplGetFloat( RealPrecision, sal_uInt32 nRealSize );
        sal_uInt32          ImplGetBitmapColor( bool bDirectColor = false );
        void                ImplSetMapMode();
        void                ImplSetUnderlineMode();
        void                ImplMapDouble( double& );
        void                ImplMapX( double& );
        void                ImplMapY( double& );
        void                ImplMapPoint( FloatPoint& );
        inline double       ImplGetIY();
        inline double       ImplGetFY();
        inline double       ImplGetIX();
        inline double       ImplGetFX();
        sal_uInt32              ImplGetPointSize();
        void                ImplGetPoint( FloatPoint& rFloatPoint, bool bMap = false );
        void                ImplGetRectangle( FloatRect&, bool bMap = false );
        void                ImplGetRectangleNS( FloatRect& );
        void                ImplGetVector( double* );
        static double       ImplGetOrientation( FloatPoint& rCenter, FloatPoint& rPoint );
        static void         ImplSwitchStartEndAngle( double& rStartAngle, double& rEndAngle );
        bool                ImplGetEllipse( FloatPoint& rCenter, FloatPoint& rRadius, double& rOrientation );

        void                ImplDefaultReplacement();
        void                ImplDoClass();
        void                ImplDoClass0();
        void                ImplDoClass1();
        void                ImplDoClass2();
        void                ImplDoClass3();
        void                ImplDoClass4();
        void                ImplDoClass5();
        void                ImplDoClass6();
        void                ImplDoClass7();
        void                ImplDoClass8();
        void                ImplDoClass9();
        void                ImplDoClass15();

    public:

                            ~CGM();

                            CGM(css::uno::Reference< css::frame::XModel > const & rModel);
        GDIMetaFile*        mpGDIMetaFile;
        sal_uInt32          GetBackGroundColor();
        bool                IsValid() const { return mbStatus; };
        bool                IsFinished() const { return mbIsFinished; };
        bool                Write( SvStream& rIStm );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
