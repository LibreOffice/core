/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: browserline.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XPropertyControl.hpp>
/** === end UNO includes === **/
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

namespace com { namespace sun { namespace star { namespace inspection { namespace PropertyLineElement
{
    const sal_Int16 CompleteLine = 0x4000;
} } } } }

//............................................................................
namespace pcr
{
//............................................................................

    class OBrowserLine;

    //========================================================================
    class IButtonClickListener
    {
    public:
        virtual void    buttonClicked( OBrowserLine* _pLine, sal_Bool _bPrimary ) = 0;
    };

    //========================================================================
    class OBrowserLine
    {
    private:
        ::rtl::OUString         m_sEntryName;
        FixedText               m_aFtTitle;
        Size                    m_aOutputSize;
        Point                   m_aLinePos;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                m_xControl;
        Window*                 m_pControlWindow;
        PushButton*             m_pBrowseButton;
        PushButton*             m_pAdditionalBrowseButton;
        IButtonClickListener*   m_pClickListener;
        Window*                 m_pTheParent;
        sal_uInt16              m_nNameWidth;
        sal_uInt16              m_nEnableFlags;
        bool                    m_bIndentTitle;
        bool                    m_bReadOnly;

    public:
                            OBrowserLine( const ::rtl::OUString& _rEntryName, Window* pParent);
                            ~OBrowserLine();

        void setControl( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
        const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& getControl()
        {
            return m_xControl;
        }
        inline Window* getControlWindow() const
        {
            return m_pControlWindow;
        }

        const ::rtl::OUString&
                            GetEntryName() const { return m_sEntryName; }

        void                SetComponentHelpIds( const SmartId& _rHelpId, sal_uInt32 _bPrimaryButtonId, sal_uInt32 _nSecondaryButtonId );

        void                SetTitle(const String& rString );
        void                FullFillTitleString();
        String              GetTitle() const;
        void                SetTitleWidth(sal_uInt16);

        void                SetPosPixel(Point aPos);
        void                SetPosSizePixel(Point aPos,Size aSize);
        Size                GetSizePixel();
        void                Show(sal_Bool bFlag=sal_True);
        void                Hide();
        sal_Bool            IsVisible();

        Window*             GetRefWindow();
        void                SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags );

        sal_Bool            GrabFocus();
        void                ShowBrowseButton( const ::rtl::OUString& _rImageURL, sal_Bool _bPrimary );
        void                ShowBrowseButton( const Image& _rImage, sal_Bool _bPrimary );
        void                ShowBrowseButton( sal_Bool _bPrimary );
        void                HideBrowseButton( sal_Bool _bPrimary );

        void                EnablePropertyControls( sal_Int16 _nControls, bool _bEnable );
        void                EnablePropertyLine( bool _bEnable );
        sal_Bool            IsPropertyInputEnabled( ) const;

        void                SetReadOnly( bool _bReadOnly );

        void                SetClickListener( IButtonClickListener* _pListener );

        void                IndentTitle( bool _bIndent );

    private:
        DECL_LINK( OnButtonClicked, PushButton* );
        DECL_LINK( OnButtonFocus, PushButton* );

        void    implHideBrowseButton( sal_Bool _bPrimary, bool _bReLayout );
        void    implUpdateEnabledDisabled();

        void    impl_layoutComponents();

        PushButton& impl_ensureButton( bool _bPrimary );
        void        impl_getImagesFromURL_nothrow( const ::rtl::OUString& _rImageURL, Image& _out_rImage, Image& _out_rHCImage );
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

