/*************************************************************************
 *
 *  $RCSfile: impgrf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:57 $
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
#ifndef _SVX_IMPGRF_HXX
#define _SVX_IMPGRF_HXX

// include ---------------------------------------------------------------

#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif

// forward ---------------------------------------------------------------

class  SvxGraphicHdl_Impl;
class  SfxObjectShell;
struct SvxImportGraphicRes_Impl;
class  SvxGraphicPrevWin_Impl;
class  SfxMedium;

// Funktionen ------------------------------------------------------------

// returnt einen static Graphic-Filter, wird einmalig angelegt,
// steht immer zur Verfuegung, DARF NIE geloescht werden!!!!
GraphicFilter* GetGrfFilter();
USHORT  FillFilter( GraphicFilter& rFilter );
int     LoadGraphic( const String& rPath, const String& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = NULL,
                     USHORT* pDeterminedFormat = NULL );

// class SvxImportGraphicDialog ------------------------------------------
#ifndef SV_NODIALOG

#define ENABLE_STANDARD         ((USHORT)0x0001)    // Standard-Button
#define ENABLE_LINK             ((USHORT)0x0002)    // Verkn"upfungs-Box
#define ENABLE_STD_AND_LINK     (ENABLE_STANDARD | ENABLE_LINK)
#define ENABLE_PROPERTY         ((USHORT)0x0004)    // Eigenschaften-Button
#define ENABLE_ALL              ((USHORT)0x0007)    // alle
#define ENABLE_PROP_WITHOUTLINK ((USHORT)0x0008)    // Eigenschaften ohne Link
#define ENABLE_EMPTY_FILENAMES  ((USHORT)0x0010)    // Leere Dateinamen zulassen

class SvxImportGraphicDialog : public SfxFileDialog
{
public:
    SvxImportGraphicDialog( Window* pParent,
                            const String& rTitle,
                            const USHORT nEnable = ENABLE_STANDARD,
                            WinBits nWinBits = WB_OPEN | WB_3DLOOK );
    ~SvxImportGraphicDialog();

    short                   Execute();
    void                    SetPath( const String& rPath, BOOL bDir,
                                        BOOL bLink = FALSE );

    BOOL                    IsURL() const;
    BOOL                    AsLink() const
                                { return pLinkBox && pLinkBox->IsChecked(); }
    GraphicFilter&          GetFilter() { return *GetGrfFilter(); }
    SvxGraphicPrevWin_Impl& GetPreviewWindow() { return *pPrevWin; }

    void                    SetPreviewing( BOOL bPrev );
    BOOL                    IsPreviewing() const { return bPreviewing; }

    Link                    GetPropertyHdl() const { return aPropertyLink; }
    void                    SetPropertyHdl( const Link& rLink )
                                { aPropertyLink = rLink; }

    Graphic*                GetGraphic() const;
    String                  GetPath() const;

private:
friend class SvxGraphicPrevWin_Impl;

    SvxImportGraphicRes_Impl*   pResImpl;
    SvxGraphicPrevWin_Impl*     pPrevWin;

    SfxMedium*              pMedium;

    PushButton*             pStandardButton;
    PushButton*             pInternetButton;
    PushButton*             pPropertiesButton;
    PushButton*             pFilterButton;
    CheckBox*               pLinkBox;
    CheckBox*               pPreviewBox;

    Link                    aPropertyLink;
    String                  aStartPath;
    String                  aCurrPath;
    Timer                   aPrevTimer;
    BOOL                    bPreviewing;
    void                    FileSelect();
    long                    OK();
    void                    SetPath( const String& ); // just to make private

#ifdef _SVX_IMPGRF_CXX
    void                    Construct_Impl( const String &rTitle,
                                            USHORT nEnable );

    DECL_LINK( StandardHdl_Impl, Button * );
    DECL_LINK( PropertiesHdl_Impl, Button * );
    DECL_LINK( FilterHdl_Impl, Button * );
    DECL_LINK( PreviewHdl_Impl, Button * );
    DECL_LINK( TimeOutHdl_Impl, Timer * );
    DECL_LINK( FilterSelectHdl_Impl, void * );
    DECL_LINK( FileSelectHdl_Impl, void * );
#endif
};

#endif  // SV_NODIALOG


#endif

