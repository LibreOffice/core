/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_APPSWAPWINDOW_HXX
#define DBAUI_APPSWAPWINDOW_HXX

#include "IClipBoardTest.hxx"
#include <vcl/lstbox.hxx>
#include "AppIconControl.hxx"
#include "AppElementType.hxx"

namespace dbaui
{
    class OAppBorderWindow;
    class OApplicationSwapWindow :  public Window,
                                    public IClipboardTest
    {
        OApplicationIconControl             m_aIconControl;
        ElementType                         m_eLastType;
        OAppBorderWindow&                   m_rBorderWin;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );

        DECL_LINK( OnContainerSelectHdl,    SvtIconChoiceCtrl* );
        DECL_LINK( ChangeToLastSelected, void* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OApplicationSwapWindow( Window* _pParent, OAppBorderWindow& _rBorderWindow );
        virtual ~OApplicationSwapWindow();
        // window overloads
        virtual void Resize();

        inline sal_Bool isCutAllowed()      { return sal_False; }
        inline sal_Bool isCopyAllowed()     { return sal_False; }
        inline sal_Bool isPasteAllowed()    { return sal_False; }
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        inline void copy()  { }
        inline void cut()   { }
        inline void paste() { }

        inline sal_uLong                    GetEntryCount() const { return m_aIconControl.GetEntryCount(); }
        inline SvxIconChoiceCtrlEntry*  GetEntry( sal_uLong nPos ) const { return m_aIconControl.GetEntry(nPos); }
        inline Rectangle                GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const { return m_aIconControl.GetBoundingBox(pEntry); }

        /** automatically creates mnemonics for the icon/texts in our left hand side panel
        */
        void    createIconAutoMnemonics( MnemonicGenerator& _rMnemonics );

        /** called to give the window the chance to intercept key events, while it has not
            the focus

            @return <TRUE/> if and only if the event has been handled, and should not
                not be further processed
        */
        bool    interceptKeyInput( const KeyEvent& _rEvent );

        /// return the element of currently select entry
        ElementType getElementType() const;

        /** clears the selection in the icon choice control and calls the handler
        */
        void clearSelection();

        /** changes the container which should be displayed. The select handler will also be called.
            @param  _eType
                Which container to show.
        */
        void selectContainer(ElementType _eType);

    private:
        bool onContainerSelected( ElementType _eType );
    };
} // namespace dbaui
#endif // DBAUI_APPSWAPWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
