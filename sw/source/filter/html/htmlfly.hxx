/*************************************************************************
 *
 *  $RCSfile: htmlfly.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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

#ifndef _HTMLFLY_HXX
#define _HTMLFLY_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SVARRAY_H
#include <svtools/svarray.hxx>
#endif

class SdrObject;
class SwFrmFmt;
class SwNodeIndex;
class SwPosFlyFrm;

// ACHTUNG: Die Werte dieses Enumgs gehen direkt in die
// Augabe Tabelle!!!
enum SwHTMLFrmType
{
    HTML_FRMTYPE_TABLE,
    HTML_FRMTYPE_TABLE_CAP,
    HTML_FRMTYPE_MULTICOL,
    HTML_FRMTYPE_EMPTY,
    HTML_FRMTYPE_TEXT,
    HTML_FRMTYPE_GRF,
    HTML_FRMTYPE_PLUGIN,
    HTML_FRMTYPE_APPLET,
    HTML_FRMTYPE_IFRAME,
    HTML_FRMTYPE_OLE,
    HTML_FRMTYPE_MARQUEE,
    HTML_FRMTYPE_CONTROL,
    HTML_FRMTYPE_DRAW,
    HTML_FRMTYPE_END
};

#define HTML_OUT_TBLNODE    0x00
#define HTML_OUT_GRFNODE    0x01
#define HTML_OUT_OLENODE    0x02
#define HTML_OUT_DIV        0x03
#define HTML_OUT_MULTICOL   0x04
#define HTML_OUT_SPACER     0x05
#define HTML_OUT_CONTROL    0x06
#define HTML_OUT_AMARQUEE   0x07
#define HTML_OUT_MARQUEE    0x08
#define HTML_OUT_GRFFRM     0x09
#define HTML_OUT_OLEGRF     0x0a
#define HTML_OUT_SPAN       0x0b
#define HTML_OUT_MASK       0x0f

#define HTML_POS_PREFIX     0x00
#define HTML_POS_BEFORE     0x10
#define HTML_POS_INSIDE     0x20
#define HTML_POS_ANY        0x30
#define HTML_POS_MASK       0x30

#define HTML_CNTNR_NONE     0x00
#define HTML_CNTNR_SPAN     0x40
#define HTML_CNTNR_DIV      0x80
#define HTML_CNTNR_MASK     0xc0


const USHORT MAX_FRMTYPES = HTML_FRMTYPE_END;
const USHORT MAX_BROWSERS = 5;

extern BYTE aHTMLOutFrmPageFlyTable[MAX_FRMTYPES][MAX_BROWSERS];
extern BYTE aHTMLOutFrmParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS];
extern BYTE aHTMLOutFrmParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS];
extern BYTE aHTMLOutFrmParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS];
extern BYTE aHTMLOutFrmAsCharTable[MAX_FRMTYPES][MAX_BROWSERS];

class SwHTMLPosFlyFrm
{
    const SwFrmFmt      *pFrmFmt;       // der Rahmen
    const SdrObject     *pSdrObject;    // ggf. Sdr-Objekt
    SwNodeIndex         *pNdIdx;        // Node-Index
    UINT32              nOrdNum;        // Aus SwPosFlyFrm
    xub_StrLen          nCntntIdx;      // seine Position im Content
    BYTE                nOutputMode;    // Ausgabe-Infos

public:

    SwHTMLPosFlyFrm( const SwPosFlyFrm& rPosFly,
                     const SdrObject *pSdrObj, BYTE nOutMode );

    BOOL operator==( const SwHTMLPosFlyFrm& ) const { return FALSE; }
    BOOL operator<( const SwHTMLPosFlyFrm& ) const;

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SdrObject *GetSdrObject() const { return pSdrObject; }

    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }

    xub_StrLen GetCntntIndex() const    { return nCntntIdx; }

    BYTE GetOutMode() const { return nOutputMode; }

    static BYTE GetOutFn( BYTE nMode ) { return nMode & HTML_OUT_MASK; }
    static BYTE GetOutPos( BYTE nMode ) { return nMode & HTML_POS_MASK; }
    static BYTE GetOutCntnr( BYTE nMode ) { return nMode & HTML_CNTNR_MASK; }

    BYTE GetOutFn() const { return nOutputMode & HTML_OUT_MASK; }
    BYTE GetOutPos() const { return nOutputMode & HTML_POS_MASK; }
    BYTE GetOutCntnr() const { return nOutputMode & HTML_CNTNR_MASK; }
};

typedef SwHTMLPosFlyFrm *SwHTMLPosFlyFrmPtr;
SV_DECL_PTRARR_SORT( SwHTMLPosFlyFrms, SwHTMLPosFlyFrmPtr, 10, 10 )


#endif
