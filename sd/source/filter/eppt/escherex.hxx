/*************************************************************************
 *
 *  $RCSfile: escherex.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sj $ $Date: 2000-12-07 16:52:23 $
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

#ifndef __EscherEX_HXX
#define __EscherEX_HXX
#ifndef _SVX_ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

// ---------------------------------------------------------------------------------------------
// Werte fuer den ULONG im PPT_PST_TextHeaderAtom
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       // Text in a Shape
    PPTTH_CENTERBODY,  // Subtitle in Title-Slide
    PPTTH_CENTERTITLE, // Title in Title-Slide
    PPTTH_HALFBODY,    // Body in two-column slide
    PPTTH_QUARTERBODY  // Body in four-body slide
};

struct _EscherPropSortStruct
{
    BYTE*       pBuf;
    UINT32      nPropSize;
    UINT32      nPropValue;
    UINT16      nPropId;
};

struct _Escher_GDIStruct
{
    Rectangle   GDIBoundRect;
    Size        GDISize;
    UINT32      GDIUncompressedSize;
};

// ---------------------------------------------------------------------------------------------

class Color;
class Graphic;
class SvMemoryStream;
class SvStorageStream;
class _EscherEx : public EscherPersistTable
{
        SvStorageStream*        mpOutStrm;
        EscherGraphicProvider*  mpGraphicProvider;

        UINT32                  mnStrmStartOfs;
        int                     mnLevel;
        UINT32*                 mpOffsets;
        UINT32*                 mpSizes;
        UINT16*                 mpRecTypes;                 // nimmt die Container RecTypes auf

        UINT32                  mnDrawings;
        SvMemoryStream          maFIDCLs;
        UINT32                  mnFIDCLs;                   // anzahl der cluster ID's

        UINT32                  mnCurrentDg;
        UINT32                  mnCurrentShapeID;           // die naechste freie ID
        UINT32                  mnCurrentShapeMaximumID;    // die hoechste und auch benutzte ID
        UINT32                  mnTotalShapesDg;            // anzahl der shapes im Dg
        UINT32                  mnTotalShapeIdUsedDg;       // anzahl der benutzten shape Id's im Dg
        UINT32                  mnTotalShapesDgg;           // anzahl der shapes im Dgg
        UINT32                  mnCountOfs;
        UINT32                  mnSortCount;
        UINT32                  mnSortBufSize;
        _EscherPropSortStruct*  mpSortStruct;
        UINT32                  mnCountCount;
        UINT32                  mnCountSize;
        BOOL                    mb_EscherSpgr;
        BOOL                    mb_EscherDg;

        UINT32                  mnGroupLevel;

        BOOL    ImplSeek( UINT32 nKey );

        // ist die graphic noch nicht vorhanden, so wird sie eingefuegt

        UINT32  ImplDggContainerSize();
        void    ImplWriteDggContainer( SvStream& rSt );

        UINT32  ImplDggAtomSize();
        void    ImplWriteDggAtom( SvStream& rSt );

        UINT32  ImplBlibStoreContainerSize();
        void    ImplWriteBlibStoreContainer( SvStream& rSt );

        UINT32  ImplOptAtomSize();
        void    ImplWriteOptAtom( SvStream& rSt );

        UINT32  ImplSplitMenuColorsAtomSize();
        void    ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                _EscherEx( SvStorageStream& rOut, UINT32 nDrawings );
                ~_EscherEx();

        void    InsertAtCurrentPos( UINT32 nBytes, BOOL bCont = FALSE );// es werden nBytes an der aktuellen Stream Position eingefuegt,
                                                                    // die PersistantTable und interne Zeiger angepasst

        void    InsertPersistOffset( UINT32 nKey, UINT32 nOffset ); // Es wird nicht geprueft, ob sich jener schluessel schon in der PersistantTable befindet
        BOOL    SeekToPersistOffset( UINT32 nKey );
        BOOL    InsertAtPersistOffset( UINT32 nKey, UINT32 nValue );// nValue wird im Stream an entrsprechender Stelle eingefuegt(overwrite modus), ohne dass sich die
                                                                    // aktuelle StreamPosition aendert

        BOOL    SeekBehindRecHeader( UINT16 nRecType );             // der stream muss vor einem gueltigen Record Header oder Atom stehen

                // features beim erzeugen folgender Container:
                //
                //      _Escher_DggContainer:   ein _EscherDgg Atom wird automatisch erzeugt und verwaltet
                //      _Escher_DgContainer:        ein _EscherDg Atom wird automatisch erzeugt und verwaltet
                //      _Escher_SpgrContainer:
                //      _Escher_SpContainer:

        void    OpenContainer( UINT16 n_EscherContainer, int nRecInstance = 0 );
        void    CloseContainer();

        void    BeginAtom();
        void    EndAtom( UINT16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
        void    AddAtom( UINT32 nAtomSitze, UINT16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
        void    AddClientAnchor( const Rectangle& rRectangle );

        void    EnterGroup( Rectangle* pBoundRect = NULL, SvMemoryStream* pClientData = NULL );
        UINT32  GetGroupLevel() const { return mnGroupLevel; };
        BOOL    SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect );
        BOOL    SetGroupLogicRect( UINT32 nGroupLevel, const Rectangle& rRect );
        void    LeaveGroup();

                // ein _Escher_Sp wird geschrieben ( Ein _Escher_DgContainer muss dazu geoeffnet sein !!)
        void    AddShape( UINT32 nShpInstance, UINT32 nFlagIds, UINT32 nShapeID = 0 );
                // reserviert eine ShapeId
        UINT32  GetShapeID();

        void    BeginCount();
        void    AddOpt( UINT16 nPropertyID, UINT32 nPropValue, BOOL bBlib = FALSE );
                // der Buffer pProp wird spaeter bei einem EndCount automatisch freigegeben!!
        BOOL    GetOpt( UINT16 nPropertyID, UINT32& rPropValue );
        void    AddOpt( UINT16 nPropertyID, BOOL bBlib, UINT32 nPropValue, BYTE* pProp, UINT32 nPropSize );
        void    AddColor( UINT32 nColor );
        void    EndCount( UINT16 nRecType, UINT16 nRecVersion = 0 );

        UINT32  GetColor( const UINT32 nColor, BOOL bSwap = TRUE );
        UINT32  GetColor( const Color& rColor, BOOL bSwap = TRUE );
        UINT32  GetGradientColor( const ::com::sun::star::awt::Gradient* pGradient, UINT32 bStartColor );
        void    WriteGradient( const ::com::sun::star::awt::Gradient* pGradient );

        UINT32  AddGraphic( SvStorageStream& rPicStrm, const ByteString& rUniqueId,
                                const Rectangle& rBoundRect, const GraphicAttr* pGraphicAttr = NULL );

        UINT32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );
};


#endif
