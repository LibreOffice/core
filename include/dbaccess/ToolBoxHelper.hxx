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

#ifndef INCLUDED_DBACCESS_TOOLBOXHELPER_HXX
#define INCLUDED_DBACCESS_TOOLBOXHELPER_HXX

#include <sal/types.h>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vcl/vclptr.hxx>
#include <dbaccess/dbaccessdllapi.h>

class SvtMiscOptions;
class ToolBox;
class VclSimpleEvent;

namespace dbaui
{
    class DBACCESS_DLLPUBLIC OToolBoxHelper
    {
        sal_Int16       m_nSymbolsSize; // shows the toolbox large or small bitmaps
        VclPtr<ToolBox>        m_pToolBox;     // our toolbox (may be NULL)
    public:
        OToolBoxHelper();
        virtual ~OToolBoxHelper();

        /** will be called when the controls need to be resized.
            @param  _rDiff
                Contains the difference of the old and new toolbox size.

                TO-DO: remove this when all implmentations
                are converted to .ui format
        */
        virtual void resizeControls(const Size& _rDiff) = 0;

        /** will be called when the image list needs to change
            @param  _eSymbolsSize
                <svtools/imgdef.hxx>
        */
        virtual void setImageList(sal_Int16 _eSymbolsSize) = 0;

        /** only the member will be set, derived classes can override
            this function and do what need to be done.
            @param  _pTB
                The new ToolBox.
            @attention
                Must be called after a FreeResource() call.
        */
        void    setToolBox(ToolBox* _pTB);

        /** checks if the toolbox needs a new imagelist.
        */
        void checkImageList();

    protected:
        DECL_LINK_TYPED(ConfigOptionsChanged, LinkParamNone*, void);
        DECL_LINK_TYPED(SettingsChanged, VclSimpleEvent&, void );
    };
}
#endif // INCLUDED_DBACCESS_TOOLBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
