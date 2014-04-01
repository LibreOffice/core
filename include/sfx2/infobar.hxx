/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SFX2_INFOBAR_HXX
#define INCLUDED_SFX2_INFOBAR_HXX

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
        OUString               m_sId;
        FixedText*                  m_pMessage;
        Button*                     m_pCloseBtn;
        std::vector< PushButton* >  m_aActionBtns;

    public:
        SfxInfoBarWindow( Window* parent, const OUString& sId,
                          const OUString& sMessage,
                          std::vector< PushButton* > aButtons );
        virtual ~SfxInfoBarWindow( );

        virtual const OUString& getId() const { return m_sId; }
        virtual void Paint( const Rectangle& ) SAL_OVERRIDE;
        virtual void Resize( ) SAL_OVERRIDE;

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
        virtual ~SfxInfoBarContainerWindow( );

        void appendInfoBar( const OUString& sId, const OUString& sMessage, std::vector< PushButton* > aButtons );
        SfxInfoBarWindow* getInfoBar( const OUString& sId );
        void removeInfoBar( SfxInfoBarWindow* pInfoBar );

        virtual void Resize( ) SAL_OVERRIDE;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
