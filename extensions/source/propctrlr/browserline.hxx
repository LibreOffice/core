/*************************************************************************
 *
 *  $RCSfile: browserline.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:03:43 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROL_HXX_
#include "brwcontrol.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    class IBrowserControl;
    //========================================================================
    class OBrowserLine
    {
    private:
                FixedText           m_aFtTitle;
                Size                m_aOutputSize;
                Point               m_aLinePos;
                IBrowserControl*    m_pBrowserControl;
                PushButton*         m_pXButton;
                Window*             m_pTheParent;
                void*               m_pData;
                BrowserControlType  m_eControlType;
                sal_uInt16          m_nNameWidth;
                sal_Bool            m_bNewNameWidth:1;
                sal_Bool            m_bNeedsRepaint:1;
                sal_Bool            m_bIsLocked:1;
                sal_Bool            m_bHasBrowseButton:1;
                sal_Bool            m_bIsHyperlink:1;

                DECL_LINK(GetFocusHdl,PushButton*);


    protected:
                virtual void        Resize();
                void                SetControlPosSize(Point aPos,Size aSize);

    public:
                OBrowserLine( Window* pParent);
                ~OBrowserLine();

                sal_Bool                    NeedsRepaint();
                void                        SetNeedsRepaint(sal_Bool bFlag);
                void                        setControl(IBrowserControl*);
                IBrowserControl*            getControl();

                void                        SetKindOfControl(BrowserControlType);
                BrowserControlType          GetKindOfControl();

                void                        SetHelpId(sal_uInt32 nCtrHelpId,sal_uInt32 nBtUniqueId);

                void                        SetTitle(const String& rString );
                // #99102# ---------------
                void                        FullFillTitleString();
                String                      GetTitle() const;
                void                        SetTitleWidth(sal_uInt16);

                void                        SetSizePixel(Size aSize);
                void                        SetPosPixel(Point aPos);
                void                        SetPosSizePixel(Point aPos,Size aSize);
                Size                        GetSizePixel();
                void                        Show(sal_Bool bFlag=sal_True);
                void                        Enable( sal_Bool _bDoEnable = sal_True );
                void                        Hide();
                sal_Bool                    IsVisible();
                void                        Invalidate();
                void                        Update();

                Window*                     GetRefWindow();
                void                        SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags );

                sal_Bool                    GrabFocus();
                void                        ShowXButton();
                void                        HideXButton();
                sal_Bool                    IsVisibleXButton();
                void                        ShowAsHyperLink(sal_Bool nFlag=sal_True);
                sal_Bool                    IsShownAsHyperlink();

                void                        Locked(sal_Bool nFlag=sal_True);
                sal_Bool                    IsLocked();

                void                        SetClickHdl(const Link&);
                void                        SetData(void* pDat);
                void*                       GetData();
    };

    DECLARE_STL_VECTOR( OBrowserLine*, OBrowserLinesArray );

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

