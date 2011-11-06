/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

        void                SetComponentHelpIds( const rtl::OString& _rHelpId, const rtl::OString& _sPrimaryButtonId, const rtl::OString& _sSecondaryButtonId );

        void                SetTitle(const String& rString );
        void                FullFillTitleString();
        String              GetTitle() const;
        void                SetTitleWidth(sal_uInt16);

        void                SetPosSizePixel(Point aPos,Size aSize);
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

