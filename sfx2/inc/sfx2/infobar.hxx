/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SFX2_MESSAGEBAR_HXX_
#define _SFX2_MESSAGEBAR_HXX_

#include <vector>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include <sfx2/dllapi.h>
#include <sfx2/childwin.hxx>

/** SfxChildWindow for positioning the InfoBar in the view.
  */
class SFX2_DLLPUBLIC SfxInfoBarContainerChild : public SfxChildWindow
{
    private:
        SfxBindings* m_pBindings;

    public:
        SfxInfoBarContainerChild( Window* pParent,
                                  sal_uInt16 nId,
                                  SfxBindings* pBindings,
                                  SfxChildWinInfo* pInfo );
        virtual ~SfxInfoBarContainerChild();

        SFX_DECL_CHILDWINDOW_WITHID( SfxInfoBarContainerChild );

        void Update( );
};

/** Class representing a single InfoBar to be added in a SfxInfoBarContainerWindow.
  */
class SfxInfoBarWindow : public Window
{
    private:
        FixedText*                  m_pMessage;
        Button*                     m_pCloseBtn;
        std::vector< PushButton* >  m_aActionBtns;

    public:
        SfxInfoBarWindow( Window* parent,
                          const rtl::OUString& sMessage,
                          std::vector< PushButton* > aButtons );
        ~SfxInfoBarWindow( );

        virtual void Paint( const Rectangle& );
        virtual void Resize( );

    private:
        DECL_LINK( CloseHandler, void* );
};

class SfxInfoBarContainerWindow : public Window
{
    private:
        SfxInfoBarContainerChild*        m_pChildWin;
        std::vector< SfxInfoBarWindow* > m_pInfoBars;

    public:
        SfxInfoBarContainerWindow( SfxInfoBarContainerChild* pChildWin );
        ~SfxInfoBarContainerWindow( );

        void appendInfoBar( const rtl::OUString& sMessage, std::vector< PushButton* > aButtons );
        void removeInfoBar( SfxInfoBarWindow* pInfoBar );

        virtual void Resize( );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
