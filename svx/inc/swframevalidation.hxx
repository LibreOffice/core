/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#ifndef _SVXSWFRAMEVALIDATION_HXX
#define _SVXSWFRAMEVALIDATION_HXX
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <limits.h>

/* -----------------03.03.2004 16:31-----------------
    struct to determine min/max values for fly frame positioning in Writer
 --------------------------------------------------*/
struct SvxSwFrameValidation
{
    sal_Int16           nAnchorType; //com::sun::star::text::TextContentAnchorType
    sal_Int16           nHoriOrient; //com::sun::star::text::HoriOrientation
    sal_Int16           nVertOrient; //com::sun::star::text::VertOrientation
    sal_Int16           nHRelOrient; //com::sun::star::text::RelOrientation
    sal_Int16           nVRelOrient; //com::sun::star::text::RelOrientation
    bool                bAutoHeight;
    bool                bAutoWidth;
    bool                bMirror;
    bool                bFollowTextFlow;

    sal_Int32 nHPos;
    sal_Int32 nMaxHPos;
    sal_Int32 nMinHPos;

    sal_Int32 nVPos;
    sal_Int32 nMaxVPos;
    sal_Int32 nMinVPos;

    sal_Int32 nWidth;
    sal_Int32 nMinWidth;
    sal_Int32 nMaxWidth;

    sal_Int32 nHeight;
    sal_Int32 nMinHeight;
    sal_Int32 nMaxHeight;

    Size    aPercentSize;   // Size fuer 100%-Wert

    SvxSwFrameValidation() :
        bAutoHeight(false),
        bAutoWidth(false),
        bMirror(false),
        bFollowTextFlow( false ),
        nHPos(0),
        nMaxHPos(LONG_MAX),
        nMinHPos(0),
        nVPos(0),
        nMaxVPos(LONG_MAX),
        nMinVPos(0),
        nWidth( 283 * 4 ), //2.0 cm
        nMinWidth(0),
        nMaxWidth(LONG_MAX),
        nHeight( 283 ), //0.5 cm
        nMaxHeight(LONG_MAX)
    {
    }
};

#endif

