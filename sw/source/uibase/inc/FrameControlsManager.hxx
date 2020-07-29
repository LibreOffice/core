/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRAMECONTROLSMANAGER_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRAMECONTROLSMANAGER_HXX

#include "FrameControl.hxx"

#include <tools/gen.hxx>

#include <map>
#include <memory>

class SwPageFrame;
class SwEditWin;
class SwContentFrame;
class SwTextNode;

typedef std::shared_ptr< SwFrameControl > SwFrameControlPtr;

typedef std::map<const SwFrame*, SwFrameControlPtr> SwFrameControlPtrMap;

/** A container for the Header/Footer, or PageBreak controls.
*/
class SwFrameControlsManager
{
    private:
        VclPtr<SwEditWin> m_pEditWin;
        std::map< FrameControlType, SwFrameControlPtrMap > m_aControls;
        std::map<const SwTextNode*, const SwContentFrame*> m_aTextNodeContentFrameMap;

    public:
        SwFrameControlsManager( SwEditWin* pEditWin );
        ~SwFrameControlsManager();
        void dispose();

        SwFrameControlPtr GetControl( FrameControlType eType, const SwFrame* pFrame );
        void RemoveControls( const SwFrame* pFrame );
        void RemoveControlsByType( FrameControlType eType, const SwFrame* pFrame );
        void HideControls( FrameControlType eType );
        void SetReadonlyControls( bool bReadonly );

        // Helper methods
        void SetHeaderFooterControl( const SwPageFrame* pPageFrame, FrameControlType eType, Point aOffset );
        void SetPageBreakControl( const SwPageFrame* pPageFrame );
        void SetUnfloatTableButton( const SwFlyFrame* pFlyFrame, bool bShow, Point aTopRightPixel = Point() );
        void SetOutlineContentVisibilityButton(const SwTextNode* pTextNd);
        void SetOutlineContentVisibilityButtons();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
