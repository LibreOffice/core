/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _FRAMECONTROLSMANAGER_HXX
#define _FRAMECONTROLSMANAGER_HXX

#include <viewsh.hxx>
#include <FrameControl.hxx>

#include <boost/shared_ptr.hpp>
#include <tools/gen.hxx>

#include <map>
#include <vector>

class SwPageFrm;
class SwEditWin;

typedef boost::shared_ptr< SwFrameControl > SwFrameControlPtr;

typedef std::map<const SwFrm*, SwFrameControlPtr> SwFrameControlPtrMap;

/** A container for the Header/Footer, or PageBreak controls.
*/
class SwFrameControlsManager
{
    private:
        SwEditWin* m_pEditWin;
        std::map< FrameControlType, SwFrameControlPtrMap > m_aControls;

    public:
        SwFrameControlsManager( SwEditWin* pEditWin );
        ~SwFrameControlsManager( );

        SwFrameControlsManager( const SwFrameControlsManager& rCopy );
        const SwFrameControlsManager& operator=( const SwFrameControlsManager& rCopy );

        SwFrameControlPtr GetControl( FrameControlType eType, const SwFrm* pFrm );
        void RemoveControls( const SwFrm* pFrm );
        void RemoveControlsByType( FrameControlType eType, const SwFrm* pFrm );
        void HideControls( FrameControlType eType );
        void SetReadonlyControls( bool bReadonly );

        // Helper methods
        void SetHeaderFooterControl( const SwPageFrm* pPageFrm, FrameControlType eType, Point aOffset );
        void SetPageBreakControl( const SwPageFrm* pPageFrm );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
