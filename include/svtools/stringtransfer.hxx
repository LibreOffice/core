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

#pragma once

#include <svtools/svtdllapi.h>
#include <vcl/transfer.hxx>


namespace svt
{
    //= OStringTransferable
    class SVT_DLLPUBLIC OStringTransferable final : public TransferDataContainer
    {
    public:
        OStringTransferable(const OUString& rContent);
        void SetData(const OUString& rContent);

    private:
        // TransferableHelper overridables
        virtual void AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& _rFlavor, const OUString& rDestDoc ) override;

        OUString     m_sContent;
    };


    //= OStringTransfer
    class OStringTransfer
    {
    public:
        /** copies the string given into the system clipboard
            <p>Beware: In opposite to the old DragServer::CopyString, this method does not <em>add</em> the string
            to the clipboard, instead the old clipboard content is <em>replaced</em>!</p>
        */
        SVT_DLLPUBLIC static void           CopyString( const OUString& _rContent, vcl::Window* _pWindow );

        /** extracts a string from the system clipboard given
            @return <TRUE/> if the extraction was successful, i.e. if the clipboard contained a string content
        */
        SVT_DLLPUBLIC static bool           PasteString( OUString& _rContent, vcl::Window* _pWindow );

        /** starts dragging a simple string
        */
        SVT_DLLPUBLIC static void           StartStringDrag( const OUString& _rContent, vcl::Window* _pWindow, sal_Int8 _nDragSourceActions );
    };

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
