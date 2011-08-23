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

#ifndef CGM_HXX_
#define CGM_HXX_

#include <com/sun/star/frame/XModel.hpp>

// ---------------------------------------------------------------
#undef CGM_USER_BREAKPOINT

#define CGM_IMPORT_CGM		0x00000001

#define CGM_EXPORT_IMPRESS	0x00000100
#define CGM_EXPORT_META		0x00000200
//#define CGM_EXPORT_COMMENT	0x00000400

// ---------------------------------------------------------------

#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <tools/list.hxx>
#include "cgmtypes.hxx"

// ---------------------------------------------------------------

class	List;
class	Bundle;
class	Graphic;
class	SvStream;
class	CGMChart;
class	CGMBitmap;
class	CGMOutAct;
class	CGMElements;
class	BitmapColor;
class	GDIMetaFile;
class	VirtualDevice;
class	CGMBitmapDescriptor;

class CGM
{
        friend class CGMChart;
        friend class CGMBitmap;
        friend class CGMElements;
        friend class CGMOutAct;
        friend class CGMImpressOutAct;

        double				mnOutdx;				// Ausgabe Groesse in 1/100TH mm
        double				mnOutdy;				// auf das gemappt wird
        double				mnVDCXadd;
        double				mnVDCYadd;
        double				mnVDCXmul;
        double				mnVDCYmul;
        double				mnVDCdx;
        double				mnVDCdy;
        double				mnXFraction;
        double				mnYFraction;
        sal_Bool				mbAngReverse;			// AngularDirection

        Graphic*			mpGraphic;				// ifdef CGM_EXPORT_META
        SvStream*			mpCommentOut;			// ifdef CGM_EXPORT_COMMENT

        sal_Bool				mbStatus;
        sal_Bool				mbMetaFile;
        sal_Bool				mbIsFinished;
        sal_Bool				mbPicture;
        sal_Bool				mbPictureBody;
        sal_Bool				mbFigure;
        sal_Bool				mbFirstOutPut;
        sal_uInt32				mnAct4PostReset;
        CGMBitmap*			mpBitmapInUse;
        CGMChart*			mpChart;				// if sal_True->"SHWSLIDEREC"
                                                    //	otherwise "BEGINPIC" commands
                                                    // controlls page inserting
        CGMElements*		pElement;
        CGMElements*		pCopyOfE;
        CGMOutAct*			mpOutAct;
        List				maDefRepList;
        List				maDefRepSizeList;

        sal_uInt8*				mpSource;		// source buffer that is not increased
                                            // ( instead use mnParaCount to index )
        sal_uInt32				mnParaSize;		// actual parameter size which has been done so far
        sal_uInt32				mnActCount;		// increased by each action
        sal_uInt8*				mpBuf;			// source stream operation -> then this is allocated for
                                            //							  the temp input buffer

        sal_uInt32				mnMode;			// source description
        sal_uInt32				mnEscape;		//
        sal_uInt32				mnElementClass;	//
        sal_uInt32				mnElementID;	//
        sal_uInt32				mnElementSize;	// full parameter size for the latest action

        void				ImplCGMInit();
        sal_uInt32			ImplGetUI16( sal_uInt32 nAlign = 0 );
        sal_uInt8			ImplGetByte( sal_uInt32 nSource, sal_uInt32 nPrecision );
        long				ImplGetI( sal_uInt32 nPrecision );
        sal_uInt32			ImplGetUI( sal_uInt32 nPrecision );
        void				ImplGetSwitch4( sal_uInt8* pSource, sal_uInt8* pDest );
        void				ImplGetSwitch8( sal_uInt8* pSource, sal_uInt8* pDest );
        double				ImplGetFloat( RealPrecision, sal_uInt32 nRealSize );
        sal_uInt32			ImplGetBitmapColor( sal_Bool bDirectColor = sal_False );
        void				ImplSetMapMode();
        void				ImplMapDouble( double& );
        void				ImplMapX( double& );
        void				ImplMapY( double& );
        void				ImplMapPoint( FloatPoint& );
        inline double		ImplGetIY();
        inline double		ImplGetFY();
        inline double		ImplGetIX();
        inline double		ImplGetFX();
        sal_uInt32				ImplGetPointSize();
        void				ImplGetPoint( FloatPoint& rFloatPoint, sal_Bool bMap = sal_False );
        void				ImplGetRectangle( FloatRect&, sal_Bool bMap = sal_False );
        void				ImplGetRectangleNS( FloatRect& );
        void				ImplGetVector( double* );
        double				ImplGetOrientation( FloatPoint& rCenter, FloatPoint& rPoint );
        void				ImplSwitchStartEndAngle( double& rStartAngle, double& rEndAngle );
        sal_Bool				ImplGetEllipse( FloatPoint& rCenter, FloatPoint& rRadius, double& rOrientation );

        void				ImplDefaultReplacement();
        void 				ImplDoClass();
        void				ImplDoClass0();
        void				ImplDoClass1();
        void				ImplDoClass2();
        void				ImplDoClass3();
        void				ImplDoClass4();
        void				ImplDoClass5();
        void				ImplDoClass6();
        void				ImplDoClass7();
        void				ImplDoClass8();
        void				ImplDoClass9();
        void				ImplDoClass15();

    public:

                            ~CGM();

                            CGM( sal_uInt32 nMode, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel );
#ifdef CGM_EXPORT_META
        VirtualDevice*		mpVirDev;
        GDIMetaFile* 		mpGDIMetaFile;
#endif
        void				ImplComment( sal_uInt32, const char* );
        sal_uInt32				GetBackGroundColor();
        sal_Bool				IsValid() { return mbStatus; };
        sal_Bool				IsFinished() { return mbIsFinished; };
        sal_Bool				Write( SvStream& rIStm );

        friend SvStream& operator>>( SvStream& rOStm, CGM& rCGM );

};
#endif

