/*************************************************************************
 *
 *  $RCSfile: browserlistbox.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:01:00 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROLLISTENER_HXX_
#include "brwcontrollistener.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#include "browserline.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif

#include <set>

//............................................................................
namespace pcr
{
//............................................................................

    class IPropertyLineListener;
    struct OLineDescriptor;
    //========================================================================
    //= OBrowserListBox
    //========================================================================
    class OBrowserListBox
                :public Control
                ,public IBrowserControlListener
                ,public IButtonClickListener
                ,public OModuleResourceClient
    {
    protected:
        Window                      m_aPlayGround;
        ScrollBar                   m_aVScroll;
        OBrowserLinesArray          m_aLines;
        ::rtl::OUString             m_aStandard;
        IPropertyLineListener*      m_pLineListener;
        long                        m_nYOffset;
        sal_uInt16                  m_nSelectedLine;
        sal_uInt16                  m_nTheNameSize;
        sal_uInt16                  m_nRowHeight;
        ::std::set< sal_uInt16 >    m_aOutOfDateLines;
        sal_Bool                    m_bIsActive : 1;
        sal_Bool                    m_bUpdate : 1;

    protected:
                void                        PositionLine( sal_uInt16 _nIndex );
                void                        UpdatePosNSize();
                void                        UpdatePlayGround();
                void                        UpdateVScroll();
                void                        ShowEntry(sal_uInt16 nPos);
                void                        MoveThumbTo(long nNewThumbPos);
                void                        Resize();


    public:
                                            OBrowserListBox( Window* pParent, WinBits nWinStyle = WB_DIALOGCONTROL );

                                            ~OBrowserListBox();

                void                        UpdateAll();

                void                        Activate(sal_Bool _bActive = sal_True);

                sal_uInt16                  CalcVisibleLines();
                void                        EnableUpdate();
                void                        DisableUpdate();
                long                        Notify( NotifyEvent& _rNEvt );

                void                        setListener(IPropertyLineListener* _pPLL);

                void                        Clear();

                sal_uInt16                  InsertEntry( const OLineDescriptor&, sal_uInt16 nPos = EDITOR_LIST_APPEND );
                sal_Bool                    RemoveEntry( const ::rtl::OUString& _rName );
                void                        ChangeEntry( const OLineDescriptor&, sal_uInt16 nPos );

                void                        SetPropertyValue( const ::rtl::OUString & rEntryName, const ::rtl::OUString & rValue );
                ::rtl::OUString             GetPropertyValue( const ::rtl::OUString & rEntryName ) const;
                sal_uInt16                  GetPropertyPos( const ::rtl::OUString& rEntryName ) const;
                IBrowserControl*            GetPropertyControl( const ::rtl::OUString& rEntryName );
                IBrowserControl*            GetCurrentPropertyControl();
                void                        EnablePropertyControls( const ::rtl::OUString& _rEntryName, bool _bEnableInput, bool _bEnablePrimaryButton, bool _bEnableSecondaryButton = false );
                void                        EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable );
                sal_Bool                    IsPropertyInputEnabled( const ::rtl::OUString& _rEntryName ) const;

                void                        SetFirstVisibleEntry(sal_uInt16 nPos);
                sal_uInt16                  GetFirstVisibleEntry();

                void                        SetSelectedEntry(sal_uInt16 nPos);
                sal_uInt16                  GetSelectedEntry();

                // #95343# --------------------------
                sal_Int32                   GetMinimumWidth();


                sal_Bool    IsModified( ) const;
                void        CommitModified( );

    protected:
        DECL_LINK( ScrollHdl, ScrollBar* );

        // IBrowserControlListener
                void                    Modified    (IBrowserControl* _pControl);
                void                    GetFocus    (IBrowserControl* _pControl);
                void                    Commit      (IBrowserControl* _pControl);
                void                    KeyInput    (IBrowserControl* _pControl, const KeyCode& _rKey);
                void                    TravelLine  (IBrowserControl* _pControl);

        // IButtonClickListener
                void    buttonClicked( OBrowserLine* _pLine, bool _bPrimary );
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_


