/*************************************************************************
 *
 *  $RCSfile: cgm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#ifndef CGM_HXX_
#define CGM_HXX_

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

// ---------------------------------------------------------------
#undef CGM_USER_BREAKPOINT

#define CGM_IMPORT_CGM      0x00000001
#define CGM_IMPORT_IM       0x00000002

#define CGM_EXPORT_IMPRESS  0x00000100
#define CGM_EXPORT_META     0x00000200
//#define CGM_EXPORT_COMMENT    0x00000400

#define CGM_NO_PAD_BYTE     0x00010000
#define CGM_BIG_ENDIAN      0x00020000
#define CGM_LITTLE_ENDIAN   0x00040000

// ---------------------------------------------------------------

#include <tools/solar.h>
#include <rtl/ustring>
#include <tools/list.hxx>
#include "cgmtypes.hxx"

// ---------------------------------------------------------------

class   List;
class   Bundle;
class   Graphic;
class   SvStream;
class   CGMChart;
class   CGMBitmap;
class   CGMOutAct;
class   CGMElements;
class   BitmapColor;
class   GDIMetaFile;
class   VirtualDevice;
class   CGMBitmapDescriptor;

class CGM
{
        friend class CGMChart;
        friend class CGMBitmap;
        friend class CGMElements;
        friend class CGMOutAct;
        friend class CGMMetaOutAct;
        friend class CGMImpressOutAct;

        double              mnOutdx;                // Ausgabe Groesse in 1/100TH mm
        double              mnOutdy;                // auf das gemappt wird
        double              mnVDCXadd;
        double              mnVDCYadd;
        double              mnVDCXmul;
        double              mnVDCYmul;
        double              mnVDCdx;
        double              mnVDCdy;
        double              mnXFraction;
        double              mnYFraction;
        sal_Bool                mbAngReverse;           // AngularDirection

        Graphic*            mpGraphic;              // ifdef CGM_EXPORT_META
        SvStream*           mpCommentOut;           // ifdef CGM_EXPORT_COMMENT

        sal_Bool                mbStatus;
        sal_Bool                mbMetaFile;
        sal_Bool                mbIsFinished;
        sal_Bool                mbPicture;
        sal_Bool                mbPictureBody;
        sal_Bool                mbFigure;
        sal_Bool                mbFirstOutPut;
        sal_uInt32              mnAct4PostReset;
        CGMBitmap*          mpBitmapInUse;
        CGMChart*           mpChart;                // if sal_True->"SHWSLIDEREC"
                                                    //  otherwise "BEGINPIC" commands
                                                    // controlls page inserting
        CGMElements*        pElement;
        CGMElements*        pCopyOfE;
        CGMOutAct*          mpOutAct;
        List                maDefRepList;
        List                maDefRepSizeList;

        sal_uInt8*              mpSource;       // source buffer that is not increased
                                            // ( instead use mnParaCount to index )
        sal_uInt32              mnParaSize;     // actual parameter size which has been done so far
        sal_uInt32              mnActCount;     // increased by each action
        sal_uInt8*              mpBuf;          // source stream operation -> then this is allocated for
                                            //                            the temp input buffer

        sal_uInt32              mnMode;         // source description
        sal_uInt32              mnEscape;       //
        sal_uInt32              mnElementClass; //
        sal_uInt32              mnElementID;    //
        sal_uInt32              mnElementSize;  // full parameter size for the latest action

        void                ImplCGMInit();
        sal_uInt32          ImplGetUI16( sal_uInt32 nAlign = 0 );
        sal_uInt8           ImplGetByte( sal_uInt32 nSource, sal_uInt32 nPrecision );
        long                ImplGetI( sal_uInt32 nPrecision );
        sal_uInt32          ImplGetUI( sal_uInt32 nPrecision );
        void                ImplGetSwitch4( sal_uInt8* pSource, sal_uInt8* pDest );
        void                ImplGetSwitch8( sal_uInt8* pSource, sal_uInt8* pDest );
        double              ImplGetFloat( RealPrecision, sal_uInt32 nRealSize );
        sal_uInt32          ImplGetBitmapColor( sal_Bool bDirectColor = sal_False );
        void                ImplSetMapMode();
        void                ImplMapDouble( double& );
        void                ImplMapX( double& );
        void                ImplMapY( double& );
        void                ImplMapPoint( FloatPoint& );
        inline double       ImplGetIY();
        inline double       ImplGetFY();
        inline double       ImplGetIX();
        inline double       ImplGetFX();
        sal_uInt32              ImplGetPointSize();
        void                ImplGetPoint( FloatPoint& rFloatPoint, sal_Bool bMap = sal_False );
        void                ImplGetRectangle( FloatRect&, sal_Bool bMap = sal_False );
        void                ImplGetRectangleNS( FloatRect& );
        void                ImplGetVector( double* );
        double              ImplGetOrientation( FloatPoint& rCenter, FloatPoint& rPoint );
        void                ImplSwitchStartEndAngle( double& rStartAngle, double& rEndAngle );
        sal_Bool                ImplGetEllipse( FloatPoint& rCenter, FloatPoint& rRadius, double& rOrientation );

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
        void                ImplDoClass16();

    public:

                            CGM( sal_uInt32 nMode );
                            ~CGM();

#ifdef CGM_EXPORT_IMPRESS
                            CGM( sal_uInt32 nMode, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel );
#endif
#ifdef CGM_EXPORT_META
        VirtualDevice*      mpVirDev;
        GDIMetaFile*        mpGDIMetaFile;
                            CGM( sal_uInt32 nMode, Graphic& rGraphic );
#endif
        void                ImplComment( sal_uInt32, char* );
        sal_uInt32              GetBackGroundColor();
        sal_Bool                IsValid() { return (const) mbStatus; };
        sal_Bool                IsFinished() { return (const) mbIsFinished; };
        sal_Bool                Write( sal_uInt8* pSource );
        sal_Bool                Write( SvStream& rIStm );

        friend SvStream& operator>>( SvStream& rOStm, CGM& rCGM );

};
#endif

