/*************************************************************************
 *
 *  $RCSfile: textattr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:55:30 $
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
#ifndef _SVX_TEXTATTR_HXX
#define _SVX_TEXTATTR_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_DLG_CTRL_HXX
#include "dlgctrl.hxx"
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

class SdrView;

/*************************************************************************
|*
|* Dialog zum "Andern von TextAttributen
|*
\************************************************************************/

class SvxTextAttrPage : public SvxTabPage
{
private:
    FixedLine           aFlText;
    TriStateBox         aTsbAutoGrowWidth;
    TriStateBox         aTsbAutoGrowHeight;
    TriStateBox         aTsbFitToSize;
    TriStateBox         aTsbContour;

    FixedLine           aFlDistance;
    FixedText           aFtLeft;
    MetricField         aMtrFldLeft;
    FixedText           aFtRight;
    MetricField         aMtrFldRight;
    FixedText           aFtTop;
    MetricField         aMtrFldTop;
    FixedText           aFtBottom;
    MetricField         aMtrFldBottom;

    FixedLine           aFlSeparator;

    FixedLine           aFlPosition;
    SvxRectCtl          aCtlPosition;
    TriStateBox         aTsbFullWidth;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    BOOL                bAutoGrowSizeEnabled;
    BOOL                bContourEnabled;

#ifdef _SVX_TEXTATTR_CXX
    DECL_LINK( ClickFullWidthHdl_Impl, void * );
    DECL_LINK( ClickHdl_Impl, void * );
#endif

    /** Return whether the text direction is from left to right (</TRUE>) or
        top to bottom (</FALSE>).
    */
    bool IsTextDirectionLeftToRight (void) const;

public:

    SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxTextAttrPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  USHORT*     GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void            PageCreated(SfxAllItemSet aSet); // add CHINA001
};

/*************************************************************************
|*
|* Von SfxSingleTabDialog abgeleitet, um vom Control "uber virtuelle Methode
|* benachrichtigt werden zu k"onnen.
|*
\************************************************************************/

//CHINA001 class SvxTextAttrDialog : public SfxSingleTabDialog
//CHINA001 {
//CHINA001 public:
//CHINA001 SvxTextAttrDialog( Window* pParent, const SfxItemSet& rAttr,
//CHINA001 const SdrView* pView );
//CHINA001 ~SvxTextAttrDialog();
//CHINA001 };


#endif // _SVX_TEXTATTR_HXX

